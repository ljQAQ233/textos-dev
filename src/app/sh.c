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

#define CONFIG_READLINE 1
#define HISTSIZE        16
#define MAXARGS         10
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
    char buf[MAXBUFSIZ];
    // [hist_l, hist_r) is used
    char hist[HISTSIZE][MAXBUFSIZ];
    int hist_l, hist_r;
    struct termios oldt;
    struct termios newt;
};

void rdl_start(struct rdlctx *ctx)
{
    tcgetattr(STDIN_FILENO, &ctx->oldt);
    ctx->newt = ctx->oldt;
    ctx->newt.c_lflag &= ~(ICANON | ECHO);
    ctx->newt.c_cc[VMIN] = 1;  // at least one char
    ctx->newt.c_cc[VTIME] = 0; // no timeout
    tcsetattr(STDIN_FILENO, TCSANOW, &ctx->newt);
}

void rdl_stop(struct rdlctx *ctx)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &ctx->oldt);
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

int readline(struct rdlctx *ctx)
{
    if (ctx->times++ == 0) {
        rdl_init(ctx);
    }
    char ch = 0;
    char *p = ctx->buf;
    int n = sizeof(ctx->buf);
    int histcur = ctx->hist_r;
    rdl_start(ctx);
    while (n > 1) {
        if (read(0, &ch, 1) <= 0) goto rollback;
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

    *p = '\0';
    rdl_stop(ctx);
    return n;
rollback:

    *p = '\0';
    rdl_stop(ctx);
    return -1;
}

//
// shell
//
int status;
struct rdlctx gctx = {0};
jmp_buf jmpbuf; // Error handling
#define expect(x)                             \
    if (!(x)) {                               \
        dprintf(2, "syntax error: " #x "\n"); \
        longjmp(jmpbuf, 1);                   \
    }

struct builtin
{
    const char *name;
    int (*main)(int argc, char *argv[], struct builtin *self);
    const char *usage;
};

struct builtin *bi_lookup(char *name);
void bi_help(struct builtin *bi);

#define DEFINE_BUILTIN(name) int name(int argc, char *argv[], struct builtin *self)
#define DECLARE_BUILTIN(name) DEFINE_BUILTIN(name)

DECLARE_BUILTIN(builtin_cd);
DECLARE_BUILTIN(builtin_exit);
DECLARE_BUILTIN(builtin_exec);
DECLARE_BUILTIN(builtin_help);
DECLARE_BUILTIN(builtin_history);
DECLARE_BUILTIN(builtin_putenv);
DECLARE_BUILTIN(builtin_unsetenv);

// 
struct builtin builtins[] = {
    // clang-format off
    // name         function            synopsis
    { "cd",         builtin_cd,         "<dir>" },
    { "exit",       builtin_exit,       "[status]" },
    { "exec",       builtin_exec,       "[cmd [arg]...]" },
    { "help",       builtin_help,       "[name]..." },
    { "history",    builtin_history,    "[number]" },
    { "putenv",     builtin_putenv,     "[string]" },
    { "unsetenv",   builtin_unsetenv,   "[varname]" },
    // clang-format on
};

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
    int s = argc <= 1 ? 0 : atoi(argv[1]);
    _exit(s);
}

DEFINE_BUILTIN(builtin_exec)
{
    if (argc <= 1) _exit(0);
    char *c = argv[1];
    execvp(c, argv + 1);
    assert(0);
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

    for (int i = 1 ; i < argc ; i++) {
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

DEFINE_BUILTIN(builtin_putenv)
{
    if (argc != 2) return 1;
    if (putenv(argv[1]) < 0) {
        perror("putenv");
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

// cmd is the "abstract syntax tree" (AST) of the command;
// if replace == true, then the current process will do exec if needed
// instead of creating a subshell first, when the func never returns.
// if exec fails, the process exits with code 1.
void nakerun(struct cmd *cmd, int replace)
{
    int p[2];
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
        if (strcmp(ecmd->argv[0], "cd") == 0) {
        }
        struct builtin *bi = bi_lookup(ecmd->argv[0]);
        if (bi != 0) {
            int argc = 0;
            while (ecmd->argv[argc])
                argc++;
            status = bi->main(argc, ecmd->argv, bi);
            if (replace) _exit(0);
            break;
        }

        if (replace || fork() == 0) {
            char *c = ecmd->argv[0];
            execvp(c, ecmd->argv);
            dprintf(2, "fail to exec %s\n", c);
            _exit(1);
        } else {
            int ws;
            wait4(-1, &ws, 0, 0);
            status = WEXITSTATUS(ws);
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
        if (fork() == 0) {
            close(1);
            dup(p[1]);
            close(p[0]);
            close(p[1]);
            nakerun(pcmd->left, 1);
            exit(0);
        }
        if (fork() == 0) {
            close(0);
            dup(p[0]);
            close(p[0]);
            close(p[1]);
            nakerun(pcmd->right, 1);
            exit(0);
        }
        close(p[0]);
        close(p[1]);
        wait4(-1, 0, 0, 0);
        wait4(-1, 0, 0, 0);
        break;

    case BACK:
        bcmd = (struct backcmd *)cmd;
        if (fork() == 0) runcmd(bcmd->cmd);
        break;

    default:
        assert(0);
    }
}

int getcmd(char *buf)
{
    if (status) dprintf(2, "\033[31m$ \033[0m");
    else dprintf(2, "\033[32m$ \033[0m");

#if CONFIG_READLINE
    readline(&gctx);
    strcpy(buf, gctx.buf);
    return 0;
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
            break;
        case 'h':
        case '?':
            dprintf(1, "Usage: %s [-h | -c cmd [arg...] ]\n", argv[0]);
            return 1;
        }
    }

    while (getcmd(buf) >= 0) {
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

struct cmd *backcmd(struct cmd *subcmd)
{
    struct backcmd *cmd;

    cmd = malloc(sizeof(*cmd));
    cmd->type = BACK;
    cmd->cmd = subcmd;
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

    cmd = parsepipe(ps, es);
    while (peek(ps, es, "&")) {
        gettoken(ps, es, 0, 0);
        cmd = backcmd(cmd);
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
