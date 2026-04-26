// 这一份是我从 jyy 的网站上下载的
// 2026/04/04 - use wrapped syscall func instead of raw syscall()
// 2026/04/04 - use execvp in libc to locate executables
// 2026/04/11 - support running builtin commands (cd) as a part of lists
// 2026/04/11 - error will be reported when wrong syntax is detected
// 2026/04/11 - migrate from `print` to `dprintf` in libc
// 2026/04/11 - simple `readline` lib - feat: history
// 2026/04/11 - use array (not strcmp chains) to systematize built-in cmds
// 2026/04/11 - new builtins: history, putenv, unsetenv
// 2026/04/16 - new builtin - help, rename builtin-(helpers) -> bi-(helpers)
// 2026/04/16 - new builtin - exec
// 2026/04/16 - receive cmd-line args: -h / -c
// 2026/04/17 - add exit-status indicator
// 2026/04/18 - simple job control
// 2026/04/19 - readline reset non-block flag.
// 2026/04/19 - fixes some bugs & add exit flag in builtin_exit
// 2026/04/25 - allow SIGINT to interrupt readline
// 2026/04/26 - fixes 'putenv' -> 'setenv' which allocates memory itself

#include <assert.h>
#include <fcntl.h>
#include <malloc.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

#define CONFIG_READLINE 1
#define HISTSIZE        16
#define MAXARGS         10
#define MAXJOBS         4
#define MAXBUFSIZ       100

//
// readline
// - rdl_l_ lib
// - rdl_v_ visual
// - rdl_h_ history
//

enum rdl_seq_opt
{
    rso_hist_prev,
    rso_hist_next,
    // TODO
    rso_left,
    rso_right,
    rso_clear,
    rso_none,
};

struct rdl_rso_map
{
    const int opt;
    const char *seq;
} rso_map[] = {
    {rso_hist_prev, "[A"},
    {rso_hist_next, "[B"},
};

struct rdlctx
{
    int times;
    int interrupt;
    char buf[MAXBUFSIZ];
    // [hist_l, hist_r) is used
    char hist[HISTSIZE][MAXBUFSIZ];
    int hist_l, hist_r;
    struct sigaction oact[_NSIG];
    struct termios oldt;
    struct termios newt;
};

void rdl_start(struct rdlctx *ctx)
{
    // SIGINT
    ctx->interrupt = 0;
    // set terminal mode for readline
    assert(tcgetattr(STDIN_FILENO, &ctx->oldt) >= 0);
    ctx->newt = ctx->oldt;
    ctx->newt.c_lflag &= ~(ICANON | ECHO);
    ctx->newt.c_cc[VMIN] = 1;  // at least one char
    ctx->newt.c_cc[VTIME] = 0; // no timeout
    assert(tcsetattr(STDIN_FILENO, TCSANOW, &ctx->newt) >= 0);

    // reset non-block flag
    // if disabled, bug found as running `vi &`
#if 1
    int fl;
    assert((fl = fcntl(0, F_GETFL)) >= 0);
    fl &= ~O_NONBLOCK;
    assert(fcntl(0, F_SETFL, fl) >= 0);
#endif

    struct sigaction act;
    assert(sigaction(SIGINT, 0, &act) >= 0);
    ctx->oact[SIGINT] = act;
    act.sa_flags &= ~SA_RESTART;
    assert(sigaction(SIGINT, &act, &act) >= 0);
}

void rdl_interrupt(struct rdlctx *ctx)
{
    ctx->interrupt = 1;
}

void rdl_stop(struct rdlctx *ctx)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &ctx->oldt);

    assert(sigaction(SIGINT, 0, &ctx->oact[SIGINT]) >= 0);
}

void rdl_init(struct rdlctx *ctx)
{
    ctx->hist_l = 0;
    ctx->hist_r = 0;
}

void rdl_getseq(char *seq, int m)
{
    char ch;
    int n = 0;
    while (n < m - 1) {
        if (read(0, &ch, 1) <= 0) break;
        if (n == 0 && ch != '[') break;
        seq[n++] = ch;
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '~')
            break;
    }
    seq[n] = '\0';
}

void rdl_mapseq(char *seq, int *opt)
{
    *opt = rso_none;
    int nr = sizeof(rso_map) / sizeof(rso_map[0]);
    for (int i = 0; i < nr; i++) {
        if (strcmp(rso_map[i].seq, seq) == 0) {
            *opt = rso_map[i].opt;
        }
    }
}

void rdl_v_erase(int n)
{
    while (n--)
        write(1, "\b \b", 3);
}

void rdl_v_refresh(char *buf)
{
    write(1, buf, strlen(buf));
}

int rdl_l_iswhite(char *str)
{
    char whitespace[] = " \t\r\n\v";
    return str[strspn(str, whitespace)] == '\0';
}

void rdl_h_shift(struct rdlctx *ctx, int *current, int prev)
{
    int cur = *current;
    if (prev) {
        if (--cur < ctx->hist_l) {
            cur = ctx->hist_l;
            goto nocopy;
        }
    } else {
        if (++cur > ctx->hist_r) {
            cur = ctx->hist_r;
            goto nocopy;
        }
    }
    strcpy(ctx->buf, ctx->hist[cur % HISTSIZE]);
nocopy:
    *current = cur;
}

int rdl_histcnt(struct rdlctx *ctx)
{
    return ctx->hist_r - ctx->hist_l;
}

char *rdl_histget(struct rdlctx *ctx, int idx)
{
    if (rdl_histcnt(ctx) <= idx || idx < 0) return 0;
    return ctx->hist[(ctx->hist_l + idx) % HISTSIZE];
}

void rdl_h_write(struct rdlctx *ctx, char *buf)
{
    char *slot = ctx->hist[ctx->hist_r % HISTSIZE];
    strcpy(slot, buf);
    ctx->hist_r += 1;
    if (ctx->hist_l <= ctx->hist_r - HISTSIZE)
        ctx->hist_l = ctx->hist_r - HISTSIZE;
}

int readline(struct rdlctx *ctx, int *nread)
{
    if (ctx->times++ == 0) {
        rdl_init(ctx);
    }
    int err = 0;
    char ch = 0;
    char *p = ctx->buf;
    int n = sizeof(ctx->buf);
    int histcur = ctx->hist_r;
    rdl_start(ctx);
    while (n > 1) {
        if (read(0, &ch, 1) <= 0) {
            err = 1;
            if (ctx->interrupt) {
                p = ctx->buf;
            }
            goto rollback;
        }
        if (ch == '\e') {
            int opt;
            char seq[16];
            rdl_getseq(seq, sizeof(seq));
            rdl_mapseq(seq, &opt);
            switch (opt) {
            case rso_hist_prev:
                rdl_v_erase(p - ctx->buf);
                rdl_h_shift(ctx, &histcur, 1);
                rdl_v_refresh(ctx->buf);
                p = ctx->buf + strlen(ctx->buf);
                break;
            case rso_hist_next:
                rdl_v_erase(p - ctx->buf);
                rdl_h_shift(ctx, &histcur, 0);
                rdl_v_refresh(ctx->buf);
                p = ctx->buf + strlen(ctx->buf);
                break;
            case rso_none:
                break;
            default:
                assert(0);
            }
            continue;
        }
        if (ch == '\b' || ch == 0x7f) {
            *p = '\0';
            if (p > ctx->buf) {
                p -= 1;
                rdl_v_erase(1);
            }
            continue;
        }
        write(1, &ch, 1);
        if (ch == '\n') {
            *p = '\0';
            if (!rdl_l_iswhite(ctx->buf)) {
                rdl_h_write(ctx, ctx->buf);
            }
            break;
        }
        *p++ = ch;
        n--;
    }

rollback:
    *p++ = '\0';
    *nread = p - ctx->buf;
    rdl_stop(ctx);
    return !err ? 0 : -1;
}

//
// shell
//

struct builtin
{
    const char *name;
    int (*main)(int argc, char *argv[], struct builtin *self);
    const char *usage;
};

enum
{
    JNONE = 0,
    JSTOPPED,
    JFOREGND,
    JBACKGND,
    JSTATMAX,
};

struct job
{
    int jstat;
    int serial;
    pid_t pid;
    char *pick;
    struct job *prev;
    struct job *next;
};

#define expect(x)                             \
    if (!(x)) {                               \
        dprintf(2, "syntax error: " #x "\n"); \
        longjmp(jmpbuf, 1);                   \
    }

#define DEFINE_BUILTIN(name) \
    int name(int argc, char *argv[], struct builtin *self)
#define DECLARE_BUILTIN(name) DEFINE_BUILTIN(name)

struct builtin *bi_lookup(char *name);
void bi_help(struct builtin *bi);

struct job *jfrom_prev();
struct job *jfrom_curr();
struct job *jfrom_corp();
struct job *jfrom_jid(int jid);
struct job *jfrom_pid(pid_t pid);
struct job *jfrom_str(char *s);
struct job *jalloc();
void jfree(struct job *j);
int jid(struct job *j);
char *jcode(struct job *j);
void jlog(struct job *j, char code[], int showpick);
int jmake(pid_t pid, char pick[]);
void jdealwait(struct job *j, int ws, int *status);
void tswitch(pid_t pid);

DECLARE_BUILTIN(builtin_bg);
DECLARE_BUILTIN(builtin_cd);
DECLARE_BUILTIN(builtin_exit);
DECLARE_BUILTIN(builtin_exec);
DECLARE_BUILTIN(builtin_fg);
DECLARE_BUILTIN(builtin_getenv);
DECLARE_BUILTIN(builtin_help);
DECLARE_BUILTIN(builtin_history);
DECLARE_BUILTIN(builtin_jobs);
DECLARE_BUILTIN(builtin_setenv);
DECLARE_BUILTIN(builtin_unsetenv);

//
struct builtin builtins[] = {
    // clang-format off
    // name         function            synopsis
    { "bg",         builtin_bg,         "[%jid]" },
    { "cd",         builtin_cd,         "<dir>" },
    { "exit",       builtin_exit,       "[status]" },
    { "exec",       builtin_exec,       "[cmd [arg]...]" },
    { "fg",         builtin_fg,         "[%jid]" },
    { "getenv",     builtin_getenv,     "[varname]"},
    { "help",       builtin_help,       "[name]..." },
    { "history",    builtin_history,    "[number]" },
    { "jobs",       builtin_jobs,       "[%jid]..." },
    { "setenv",     builtin_setenv,     "[string]" },
    { "unsetenv",   builtin_unsetenv,   "[varname]" },
    // clang-format on
};

int status;
int serial;
jmp_buf jmpbuf; // Error handling
struct rdlctx gctx = {0};
struct job jobs[MAXJOBS];
struct job head = {
    .prev = &head,
    .next = &head,
};

DEFINE_BUILTIN(builtin_bg)
{
    if (argc != 1 && argc != 2) {
        bi_help(self);
        return 1;
    }

    struct job *j;
    if (argc == 1)
        j = jfrom_prev();
    else
        j = jfrom_str(argv[1]);
    if (!j || j->jstat == JNONE) {
        dprintf(2, "%s: %s: no such job\n", argv[0],
                argv[1] ? argv[1] : "+ or -");
        return 127;
    }
    if (j->jstat != JSTOPPED) {
        dprintf(2, "%s: %d: job already in background\n", argv[0], jid(j));
        return 1;
    }
    j->jstat = JBACKGND;
    // TODO: killpg
    kill(j->pid, SIGCONT);
    return 0;
}

DEFINE_BUILTIN(builtin_cd)
{
    if (chdir(argv[1]) < 0) {
        dprintf(2, "cannot cd %s\n", argv[1]);
        return 1;
    }
    return 0;
}

DEFINE_BUILTIN(builtin_exit)
{
    static int last = -2;
    int s = argc <= 1 ? 0 : atoi(argv[1]);
    if (last != serial - 1) {
        if (head.next != &head) {
            last = serial;
            dprintf(2, "\nThere are jobs leaked:\n\n");
            builtin_jobs(1, 0, 0);
            return 1;
        }
    }
    _exit(s);
}

void nakexec(char *argv[]);

DEFINE_BUILTIN(builtin_exec)
{
    if (argc <= 1) _exit(0);
    nakexec(argv + 1);
    assert(0);
}

DEFINE_BUILTIN(builtin_fg)
{
    if (argc != 1 && argc != 2) {
        bi_help(self);
        return 1;
    }

    struct job *j;
    if (argc == 1)
        j = jfrom_corp();
    else
        j = jfrom_str(argv[1]);
    if (!j || j->jstat == JNONE) {
        dprintf(2, "%s: %s: no such job\n", argv[0],
                argv[1] ? argv[1] : "+ or -");
        return 127;
    }
    j->jstat = JFOREGND;

    int ws;
    int status;
    pid_t chd = j->pid;
    kill(chd, SIGCONT);
    tswitch(chd);
    wait4(chd, &ws, WUNTRACED, 0);
    tswitch(0);
    jdealwait(j, ws, &status);
    return status;
}

DEFINE_BUILTIN(builtin_getenv)
{
    if (argc != 2) return 1;
    char *s = getenv(argv[1]);
    if (!s) {
        perror("getenv");
        return 1;
    }
    puts(s);
    return 0;
}

DEFINE_BUILTIN(builtin_help)
{
    static const char *fmt = " %- 8s %s\n";
    if (argc <= 1) {
        int nr = sizeof(builtins) / sizeof(builtins[0]);
        for (int i = 0; i < nr; i++)
            dprintf(2, fmt, builtins[i].name, builtins[i].usage);
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        struct builtin *bi;
        if ((bi = bi_lookup(argv[i]))) {
            dprintf(2, fmt, bi->name, bi->usage);
        } else {
            dprintf(2, fmt, argv[i], "not found");
        }
    }
    return 0;
}

DEFINE_BUILTIN(builtin_history)
{
    char *res;
    int cur;
    if (argc <= 1) {
        cur = 0;
    } else if (argc == 2) {
        int n = atoi(argv[1]);
        cur = rdl_histcnt(&gctx) - n;
        if (cur < 0) cur = 0;
    } else {
        return 1;
    }
    while ((res = rdl_histget(&gctx, cur++)))
        dprintf(1, "% 4d %s\n", cur, res);
    return 0;
}

DEFINE_BUILTIN(builtin_jobs)
{
    if (argc <= 1) {
        for (int i = 0; i < MAXJOBS; i++) {
            struct job *j = &jobs[i];
            if (j->jstat != JNONE) {
                jlog(j, jcode(j), 1);
            }
        }
        return 0;
    }
    for (int i = 1; i < argc; i++) {
        char *s = argv[i];
        struct job *j = jfrom_str(s);
        if (j == 0) {
            dprintf(2, "%s: %s: no such job\n", argv[0], s);
            return 127;
        }
        jlog(j, jcode(j), 1);
    }
    return 0;
}

DEFINE_BUILTIN(builtin_setenv)
{
    if (argc != 3) return 1;
    if (setenv(argv[1], argv[2], 1) < 0) {
        perror("setenv");
        return 1;
    }
    return 0;
}

DEFINE_BUILTIN(builtin_unsetenv)
{
    if (argc != 2) return 1;
    if (unsetenv(argv[1]) < 0) {
        perror("unsetenv");
        return 1;
    }
    return 0;
}

//
// internal utils
//
void bi_help(struct builtin *bi)
{
    dprintf(2, "Usage: %s %s\n", bi->name, bi->usage);
}

struct builtin *bi_lookup(char *name)
{
    int nr = sizeof(builtins) / sizeof(builtins[0]);
    for (int i = 0; i < nr; i++) {
        if (strcmp(builtins[i].name, name) == 0) {
            return builtins + i;
        }
    }
    return 0;
}

//
// job control
//
struct job *jfrom_prev()
{
    if (head.next == &head) return 0;
    if (head.next->next == &head) return 0;
    return head.next->next;
}

struct job *jfrom_curr()
{
    if (head.next == &head) return 0;
    return head.next;
}

// curr or prev
struct job *jfrom_corp()
{
    return jfrom_curr() ? jfrom_curr() : jfrom_prev();
}

struct job *jfrom_jid(int jid)
{
    return jid <= 0 || jid > MAXJOBS ? 0 : &jobs[jid - 1];
}

struct job *jfrom_pid(pid_t pid)
{
    for (int i = 0; i < MAXJOBS; i++) {
        if (jobs[i].jstat == JNONE) continue;
        if (jobs[i].pid == pid) return &jobs[i];
    }
    return 0;
}

struct job *jfrom_str(char *s)
{
    if (s[0] != '%') {
        return 0;
    }
    struct job *j = 0;
    if (s[1] == '+')
        j = jfrom_curr();
    else if (s[1] == '-')
        j = jfrom_prev();
    else
        j = jfrom_jid(atoi(s + 1));
    return j;
}

struct job *jalloc()
{
    struct job *j = 0;
    for (int i = 0; i < MAXJOBS; i++) {
        if (jobs[i].jstat == JNONE) {
            j = &jobs[i];
            break;
        }
    }
    if (j == 0) return 0;
    j->prev = &head;
    j->next = head.next;
    head.next->prev = j;
    head.next = j;
    return j;
}

void jfree(struct job *j)
{
    assert(j->jstat != JNONE);
    j->jstat = JNONE;
    j->prev->next = j->next;
    j->next->prev = j->prev;
}

int jid(struct job *j)
{
    return (int)(j - jobs) + 1;
}

char *jcode(struct job *j)
{
    static char *code[] = {
        [JNONE] = "unknown",
        [JSTOPPED] = "stopped",
        [JBACKGND] = "running",
    };
    return j->jstat <= JNONE || j->jstat >= JSTATMAX //
               ? code[JNONE]
               : code[j->jstat];
}

void jlog(struct job *j, char code[], int showpick)
{
    char mark = ' ';
    if (j == jfrom_curr())
        mark = '+';
    else if (j == jfrom_prev())
        mark = '-';
    dprintf(2, "[%d]%c %d %- 10s %s\n", jid(j), mark, j->pid, code,
            showpick ? j->pick : "");
}

int jmake(pid_t pid, char pick[])
{
    struct job *j;
    if ((j = jalloc()) == 0) {
        dprintf(2, "[0] %d : too many jobs: max = %d\n", pid, MAXJOBS);
        return -1;
    }
    j->jstat = JBACKGND;
    j->pid = pid;
    j->pick = strdup(pick);
    return jid(j);
}

void jdealwait(struct job *j, int ws, int *status)
{
    if (WIFEXITED(ws)) {
        char code[8];
        snprintf(code, sizeof(code), "exit %d", WEXITSTATUS(ws));
        jlog(j, code, 1);
        jfree(j);
    } else if (WIFSTOPPED(ws)) {
        jlog(j, "stopped", 1);
        j->jstat = JSTOPPED;
    } else if (WIFSIGNALED(ws)) {
        char code[8];
        snprintf(code, sizeof(code), "sig %d", WTERMSIG(ws));
        jlog(j, code, 1);
        jfree(j);
    } else if (WIFCONTINUED(ws)) {
        jlog(j, "continue", 1);
    }
}

void tswitch(pid_t pid)
{
    assert(tcsetpgrp(0, pid != 0 ? pid : getpgid(0)) >= 0);
}

void ignore_stp()
{
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
}

void default_stp()
{
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
}

// EXEC:   ls
// REDIR:  ls > a.txt
// PIPE:   ls | wc -l
// LIST:   ls ; ls
enum
{
    EXEC = 1,
    REDIR,
    PIPE,
    LIST,
    BACK
};

#define MAXARGS 10

struct cmd
{
    int type;
};

struct execcmd
{
    int type;
    char *argv[MAXARGS], *eargv[MAXARGS];
};

struct redircmd
{
    int type, fd, mode;
    char *file, *efile;
    struct cmd *cmd;
};

struct pipecmd
{
    int type;
    struct cmd *left, *right;
};

struct listcmd
{
    int type;
    struct cmd *left, *right;
};

struct backcmd
{
    int type;
    struct cmd *cmd;
    char pick[MAXBUFSIZ];
};

struct cmd *parsecmd(char *);
void nakerun(struct cmd *cmd, int replace);

void runcmd(struct cmd *cmd)
{
    nakerun(cmd, 0);
}

// save or restore fd
void snapfd(int fd, int save)
{
    if (save) {
        dup2(fd, 16 + fd);
        close(fd);
    } else {
        dup2(16 + fd, fd);
        close(16 + fd);
    }
}

void nakexec(char *argv[])
{
    char *c = argv[0];
    execvp(c, argv);
    dprintf(2, "fail to exec %s\n", c);
    _exit(1);
}

// cmd is the "abstract syntax tree" (AST) of the command;
// if replace == true, then the current process will do exec if needed
// instead of creating a subshell first, when the func never returns.
// if exec fails, the process exits with code 1.
void nakerun(struct cmd *cmd, int replace)
{
    int p[2];
    int lchd, rchd;
    struct backcmd *bcmd;
    struct execcmd *ecmd;
    struct listcmd *lcmd;
    struct pipecmd *pcmd;
    struct redircmd *rcmd;

    if (cmd == 0) return;

    switch (cmd->type) {
    case EXEC:
        ecmd = (struct execcmd *)cmd;
        if (ecmd->argv[0] == 0) break;
        struct builtin *bi = bi_lookup(ecmd->argv[0]);
        if (bi != 0) {
            int argc = 0;
            while (ecmd->argv[argc])
                argc++;
            status = bi->main(argc, ecmd->argv, bi);
            if (replace) _exit(status);
            break;
        }

        if (replace) {
            nakexec(ecmd->argv);
        } else {
            int chd = fork();
            if (chd == 0) {
                default_stp();
                setpgid(0, 0);
                nakexec(ecmd->argv);
            } else {
                int ws;
                setpgid(chd, chd);
                tswitch(chd);
                wait4(chd, &ws, WUNTRACED, 0);
                tswitch(0);
                if (WIFSTOPPED(ws)) {
                    struct job *j;
                    int jid;
                    char pick[MAXBUFSIZ];
                    sprintf(pick, "%s [...]", ecmd->argv[0]);
                    jid = jmake(chd, pick);
                    j = jfrom_jid(jid);
                    if (j) {
                        jlog(j, "stopped", 1);
                        j->jstat = JSTOPPED;
                    }
                    status = 128 + WSTOPSIG(ws);
                } else if (WIFEXITED(ws)) {
                    status = WEXITSTATUS(ws);
                } else if (WIFSIGNALED(ws)) {
                    status = 128 + WTERMSIG(ws);
                } else {
                    assert(!"how did you arrive here?");
                }
            }
        }
        break;

    case REDIR:
        rcmd = (struct redircmd *)cmd;
        snapfd(rcmd->fd, 1);
        if (open(rcmd->file, rcmd->mode, 0644) < 0) {
            dprintf(2, "fail to open %s\n", rcmd->file);
            break;
        }
        runcmd(rcmd->cmd);
        snapfd(rcmd->fd, 0);
        break;

    case LIST:
        lcmd = (struct listcmd *)cmd;
        runcmd(lcmd->left);
        runcmd(lcmd->right);
        break;

    case PIPE:
        pcmd = (struct pipecmd *)cmd;
        assert(pipe(p) >= 0);
        lchd = fork();
        if (lchd == 0) {
            default_stp();
            setpgid(0, 0);
            close(1);
            dup(p[1]);
            close(p[0]);
            close(p[1]);
            nakerun(pcmd->left, 1);
            _exit(status);
        }
        rchd = fork();
        if (rchd == 0) {
            default_stp();
            setpgid(0, 0);
            close(0);
            dup(p[0]);
            close(p[0]);
            close(p[1]);
            nakerun(pcmd->right, 1);
            _exit(status);
        }
        setpgid(lchd, lchd);
        setpgid(rchd, rchd);
        close(p[0]);
        close(p[1]);
        wait4(lchd, 0, 0, 0);
        wait4(rchd, 0, 0, 0);
        break;

    case BACK:
        bcmd = (struct backcmd *)cmd;
        pid_t chd = fork();
        if (chd == 0) {
            default_stp();
            setpgid(0, 0);
            nakerun(bcmd->cmd, 1);
            _exit(0);
        } else {
            setpgid(chd, chd);
            jmake(chd, bcmd->pick);
            jlog(jfrom_pid(chd), "", 1);
        }
        break;

    default:
        assert(0);
    }
}

int getcmd(char *buf)
{
    if (status)
        dprintf(2, "\033[31m$ \033[0m");
    else
        dprintf(2, "\033[32m$ \033[0m");

#if CONFIG_READLINE
    int nread;
    if (readline(&gctx, &nread) < 0) {
        buf[0] = 0;
        status = 1;
        dprintf(2, "\n");
        return 0;
    }
    strncpy(buf, gctx.buf, nread);
    return nread;
#endif

    int nbuf = MAXBUFSIZ;
    for (int i = 0; i < nbuf; i++)
        buf[i] = '\0';

    char ch = 0;
    char *p = buf;
    while (nbuf > 1 && ch != '\n') {
        int nread = read(0, &ch, 1);
        if (nread < 0) return -1;
        if (ch == '\t') ch = ' ';
        if (ch == '\e') {
            *p = '\0';
            continue;
        } else if (ch == '\b') {
            if (buf < p)
                *(p--) = '\0';
            else
                ch = 0;
        } else {
            *(p++) = ch;
            nbuf--;
        }
    }

    return 0;
}

void do_sigint(int sig)
{
    rdl_interrupt(&gctx);
}

void do_sigchld(int sig)
{
    int ws;
    int pid = wait4(-1, &ws, WNOHANG | WUNTRACED | WCONTINUED | WEXITED, 0);
    if (pid <= 0) {
        return;
    }

    struct job *j = jfrom_pid(pid);
    if (!j) return;
    /* handle it only if this shell is in charge of it */
    int status;
    (void)status;
    jdealwait(j, ws, &status);
}

int main(int argc, char *argv[])
{
    static char buf[MAXBUFSIZ];

    int c;
    while ((c = getopt(argc, argv, "hc:")) != -1) {
        switch (c) {
        case 'c':
            if (setjmp(jmpbuf) == 0) {
                runcmd(parsecmd(optarg));
                wait4(-1, 0, 0, 0);
            }
            return status;
        case 'h':
        case '?':
            dprintf(1, "Usage: %s [-h | -c cmd [arg...] ]\n", argv[0]);
            return 1;
        }
    }

    /* protect our shell */
    ignore_stp();
    signal(SIGINT, do_sigint);
    signal(SIGCHLD, do_sigchld);

    setpgid(0, 0);
    tswitch(0);

    while (getcmd(buf) >= 0) {
        serial += 1;
        if (setjmp(jmpbuf) == 0) {
            runcmd(parsecmd(buf));
        }
    }
    return 0;
}

// Constructors

struct cmd *execcmd(void)
{
    struct execcmd *cmd;

    cmd = malloc(sizeof(*cmd));
    cmd->type = EXEC;
    return (struct cmd *)cmd;
}

struct cmd *redircmd(struct cmd *subcmd, char *file, char *efile, int mode,
                     int fd)
{
    struct redircmd *cmd;

    cmd = malloc(sizeof(*cmd));
    cmd->type = REDIR;
    cmd->cmd = subcmd;
    cmd->file = file;
    cmd->efile = efile;
    cmd->mode = mode;
    cmd->fd = fd;
    return (struct cmd *)cmd;
}

struct cmd *pipecmd(struct cmd *left, struct cmd *right)
{
    struct pipecmd *cmd;

    cmd = malloc(sizeof(*cmd));
    cmd->type = PIPE;
    cmd->left = left;
    cmd->right = right;
    return (struct cmd *)cmd;
}

struct cmd *listcmd(struct cmd *left, struct cmd *right)
{
    struct listcmd *cmd;

    cmd = malloc(sizeof(*cmd));
    cmd->type = LIST;
    cmd->left = left;
    cmd->right = right;
    return (struct cmd *)cmd;
}

struct cmd *backcmd(struct cmd *subcmd, char *cherrypick, int l)
{
    struct backcmd *cmd;

    cmd = malloc(sizeof(*cmd));
    cmd->type = BACK;
    cmd->cmd = subcmd;
    strncpy(cmd->pick, cherrypick, l);
    return (struct cmd *)cmd;
}

// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

int gettoken(char **ps, char *es, char **q, char **eq)
{
    char *s;
    int ret;

    s = *ps;
    while (s < es && strchr(whitespace, *s))
        s++;
    if (q) *q = s;
    ret = *s;
    switch (*s) {
    case 0:
        break;
    case '|':
    case '(':
    case ')':
    case ';':
    case '&':
    case '<':
        s++;
        break;
    case '>':
        s++;
        if (*s == '>') {
            ret = '+';
            s++;
        }
        break;
    default:
        ret = 'a';
        while (s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
            s++;
        break;
    }
    if (eq) *eq = s;

    while (s < es && strchr(whitespace, *s))
        s++;
    *ps = s;
    return ret;
}

int peek(char **ps, char *es, char *toks)
{
    char *s;

    s = *ps;
    while (s < es && strchr(whitespace, *s))
        s++;
    *ps = s;
    return *s && strchr(toks, *s);
}

struct cmd *parseline(char **, char *);
struct cmd *parsepipe(char **, char *);
struct cmd *parseexec(char **, char *);
struct cmd *nulterminate(struct cmd *);

struct cmd *parsecmd(char *s)
{
    char *es;
    struct cmd *cmd;

    es = s + strlen(s);
    cmd = parseline(&s, es);
    peek(&s, es, "");
    expect(s == es);
    nulterminate(cmd);
    return cmd;
}

struct cmd *parseline(char **ps, char *es)
{
    struct cmd *cmd;
    char *backl = *ps;
    char *backr = es;

    cmd = parsepipe(ps, es);
    while (peek(ps, es, "&")) {
        gettoken(ps, es, 0, 0);
        cmd = backcmd(cmd, backl, backr - backl);
    }
    if (peek(ps, es, ";")) {
        gettoken(ps, es, 0, 0);
        cmd = listcmd(cmd, parseline(ps, es));
    }
    return cmd;
}

struct cmd *parsepipe(char **ps, char *es)
{
    struct cmd *cmd;

    cmd = parseexec(ps, es);
    if (peek(ps, es, "|")) {
        gettoken(ps, es, 0, 0);
        cmd = pipecmd(cmd, parsepipe(ps, es));
    }
    return cmd;
}

struct cmd *parseredirs(struct cmd *cmd, char **ps, char *es)
{
    int tok;
    char *q, *eq;

    while (peek(ps, es, "<>")) {
        tok = gettoken(ps, es, 0, 0);
        expect(gettoken(ps, es, &q, &eq) == 'a');
        switch (tok) {
        case '<':
            cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
            break;
        case '>':
            cmd = redircmd(cmd, q, eq, O_WRONLY | O_CREAT | O_TRUNC, 1);
            break;
        case '+': // >>
            cmd = redircmd(cmd, q, eq, O_WRONLY | O_CREAT, 1);
            break;
        }
    }
    return cmd;
}

struct cmd *parseblock(char **ps, char *es)
{
    struct cmd *cmd;

    expect(peek(ps, es, "("));
    gettoken(ps, es, 0, 0);
    cmd = parseline(ps, es);
    expect(peek(ps, es, ")"));
    gettoken(ps, es, 0, 0);
    cmd = parseredirs(cmd, ps, es);
    return cmd;
}

struct cmd *parseexec(char **ps, char *es)
{
    char *q, *eq;
    int tok, argc;
    struct execcmd *cmd;
    struct cmd *ret;

    if (peek(ps, es, "(")) return parseblock(ps, es);

    ret = execcmd();
    cmd = (struct execcmd *)ret;

    argc = 0;
    ret = parseredirs(ret, ps, es);
    while (!peek(ps, es, "|)&;")) {
        if ((tok = gettoken(ps, es, &q, &eq)) == 0) break;
        expect(tok == 'a');
        cmd->argv[argc] = q;
        cmd->eargv[argc] = eq;
        expect(++argc < MAXARGS);
        ret = parseredirs(ret, ps, es);
    }
    cmd->argv[argc] = 0;
    cmd->eargv[argc] = 0;
    return ret;
}

// NUL-terminate all the counted strings.
struct cmd *nulterminate(struct cmd *cmd)
{
    int i;
    struct backcmd *bcmd;
    struct execcmd *ecmd;
    struct listcmd *lcmd;
    struct pipecmd *pcmd;
    struct redircmd *rcmd;

    if (cmd == 0) return 0;

    switch (cmd->type) {
    case EXEC:
        ecmd = (struct execcmd *)cmd;
        for (i = 0; ecmd->argv[i]; i++)
            *ecmd->eargv[i] = 0;
        break;

    case REDIR:
        rcmd = (struct redircmd *)cmd;
        nulterminate(rcmd->cmd);
        *rcmd->efile = 0;
        break;

    case PIPE:
        pcmd = (struct pipecmd *)cmd;
        nulterminate(pcmd->left);
        nulterminate(pcmd->right);
        break;

    case LIST:
        lcmd = (struct listcmd *)cmd;
        nulterminate(lcmd->left);
        nulterminate(lcmd->right);
        break;

    case BACK:
        bcmd = (struct backcmd *)cmd;
        nulterminate(bcmd->cmd);
        break;
    }
    return cmd;
}
