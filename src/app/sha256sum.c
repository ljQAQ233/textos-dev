/*
 * written by deepseek
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

static const unsigned K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

typedef struct
{
    unsigned state[8];
    unsigned char buf[64];
    unsigned buflen;
    unsigned long long bitlen;
} sha256_ctx;

#define rotr(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define Ch(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define Maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define Sigma0(x) (rotr((x), 2) ^ rotr((x), 13) ^ rotr((x), 22))
#define Sigma1(x) (rotr((x), 6) ^ rotr((x), 11) ^ rotr((x), 25))
#define sigma0(x) (rotr((x), 7) ^ rotr((x), 18) ^ ((x) >> 3))
#define sigma1(x) (rotr((x), 17) ^ rotr((x), 19) ^ ((x) >> 10))

static void sha256_init(sha256_ctx *ctx)
{
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
    ctx->buflen = 0;
    ctx->bitlen = 0;
}

static void sha256_transform(sha256_ctx *ctx, const unsigned char data[])
{
    unsigned a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    for (; i < 64; ++i)
        m[i] = sigma1(m[i - 2]) + m[i - 7] + sigma0(m[i - 15]) + m[i - 16];

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i)
    {
        t1 = h + Sigma1(e) + Ch(e, f, g) + K[i] + m[i];
        t2 = Sigma0(a) + Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

// 输入数据
static void sha256_update(sha256_ctx *ctx, const unsigned char data[], unsigned len)
{
    for (unsigned i = 0; i < len; i++)
    {
        ctx->buf[ctx->buflen] = data[i];
        ctx->buflen++;
        if (ctx->buflen == 64)
        {
            sha256_transform(ctx, ctx->buf);
            ctx->bitlen += 512;
            ctx->buflen = 0;
        }
    }
}

// 最终哈希计算
static void sha256_final(sha256_ctx *ctx, unsigned char hash[])
{
    unsigned i = ctx->buflen;

    // 填充消息
    if (ctx->buflen < 56)
    {
        ctx->buf[i++] = 0x80;
        while (i < 56)
            ctx->buf[i++] = 0x00;
    }
    else
    {
        ctx->buf[i++] = 0x80;
        while (i < 64)
            ctx->buf[i++] = 0x00;
        sha256_transform(ctx, ctx->buf);
        for (i = 0; i < 56; i++)
            ctx->buf[i] = 0x00;
    }

    // 添加位长度
    ctx->bitlen += ctx->buflen * 8;
    ctx->buf[63] = ctx->bitlen;
    ctx->buf[62] = ctx->bitlen >> 8;
    ctx->buf[61] = ctx->bitlen >> 16;
    ctx->buf[60] = ctx->bitlen >> 24;
    ctx->buf[59] = ctx->bitlen >> 32;
    ctx->buf[58] = ctx->bitlen >> 40;
    ctx->buf[57] = ctx->bitlen >> 48;
    ctx->buf[56] = ctx->bitlen >> 56;
    sha256_transform(ctx, ctx->buf);

    // 转换为大端序
    for (i = 0; i < 4; ++i)
    {
        hash[i] = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4] = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8] = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
    }
}

static int sha256_file(int fd, unsigned char hash[32])
{
    int siz;
    sha256_ctx ctx;
    unsigned char buf[4096];
    sha256_init(&ctx);
    while ((siz = read(fd, buf, sizeof(buf))) > 0)
        sha256_update(&ctx, buf, siz);
    sha256_final(&ctx, hash);
    return 0;
}

int main(int argc, char *argv[])
{
    unsigned char hash[32];
    if (argc == 1)
    {
        sha256_file(STDIN_FILENO, hash);
        for (int i = 0; i < 32; i++)
            printf("%02x", hash[i]);
        printf("  -\n");
        return 0;
    }

    for (int i = 1; i < argc; ++i)
    {
        int fd = open(argv[i], O_RDONLY);
        sha256_file(fd, hash);       // compute
        for (int j = 0; j < 32; j++) // hash
            printf("%02x", hash[j]); // hex
        printf("  %s\n", argv[i]);   // filename
        close(fd);
    }

    return 0;
}