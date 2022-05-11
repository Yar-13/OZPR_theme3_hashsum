#include <stdfix.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>

#include "const_val.h"

#define BLOCK_SIZE 64 // Размер блока — 64 байта

typedef uint8_t vect[BLOCK_SIZE]; // Определяем тип vect как 64-байтовый массив


static void X(const uint8_t *a, const uint8_t *b, uint8_t *c)
{
    for(int i=0; i<64; i++)
    c[i] = a[i]^b[i];
}

static void Add512(const uint8_t *a, const uint8_t *b, uint8_t *c)
{
    int internal = 0;
    for(int i=0; i<64; i++)
    {
        internal = a[i] + b[i] + (internal >> 8);
        c[i] = internal & 0xff;
    }
}

static void S(uint8_t *state)
{
    vect internal;
    for(int i=63; i>=0; i--)
        internal[i] = Pi[state[i]];
    memcpy(state, internal, BLOCK_SIZE);
}

static void P(uint8_t *state)
{
    vect internal;
    for(int i=63; i>=0; i--)
        internal[i] = state[Tau[i]];
    memcpy(state, internal, BLOCK_SIZE);
}

static void L(uint8_t *state)
{
    uint64_t internal_in = (uint64_t*)state;
    uint64_t internal_out[8];
    memset(internal_out, 0x00, BLOCK_SIZE);
    for(int i=7; i>=0; i--)
        for(int j=63; j>=0; j--)
            if((internal_in >> j) & 1)
                internal_out[i] ^= A[63 - j];
    memcpy(state, internal_out, 64);
}


typedef struct Context
{
    vect buffer; // Буфер для очередного блока хешируемого сообщения
    vect hash; // Итоговый результат вычислений
    vect h; // Промежуточный результат вычислений
    vect N; //
    vect Sigma; //Контрольная сумма
    vect v_0; // Инициализационный вектор
    vect v_512; // Инициализационный вектор
    size_t buf_size; // Размер оставшейся части сообщения (которая оказалась меньше очередных 64 байт)
    int hash_size;   // Размер хеш-суммы (512 или 256 бит)
} TContext;

static void GetKey(uint8_t *K, int i)
{
    X(K, C[i], K);
    S(K);
    P(K);
    L(K);
}

static void E(uint8_t *K, const uint8_t *m, uint8_t *state)
{
    memcpy(K, K, BLOCK_SIZE);
    X(m, K, state);
    for(int i=0; i<12; i++)
    {
        S(state);
        P(state);
        L(state);
        GetKey(K, i);
        X(state, K, state);
    }
}

static void G(uint8_t *h, uint8_t *N, const uint8_t *m)
{
    vect K, internal;
    X(N, h, K);
    S(K);
    P(K);
    L(K);

    E(K, m, internal);

    X(internal, h, internal);
    X(internal, m, h);
}

static void Padding(TContext *CTX)
{
    vect internal;
    if(CTX->buf_size < BLOCK_SIZE)
    {
        memset(internal, 0x00, BLOCK_SIZE);
        memcpy(internal, CTX->buffer, CTX->buf_size);
        internal[CTX->buf_size] = 0x01;
        memcpy(CTX->buffer, internal, BLOCK_SIZE);
    }
}

void Init(TContext *CTX, uint16_t hash_size)
{
    memset(CTX, 0x00, sizeof(TContext));
    //printf("checkinit\n");
    if(hash_size == 256)
        memset(CTX->h, 0x01, BLOCK_SIZE);
    else
        memset(CTX->h, 0x00, BLOCK_SIZE);
    CTX->hash_size = hash_size;
    CTX->v_512[1] = 0x02;
}

static void Stage_2(TContext *CTX, const uint8_t *data)
{
    G(CTX->h, CTX->N, data);
    Add512(CTX->N, CTX->v_512, CTX->N);
    Add512(CTX->Sigma, data, CTX->Sigma);

}

static void Stage_3(TContext *CTX)
{
    vect internal;
    memset(internal, 0x00, BLOCK_SIZE);
    internal[1] = ((CTX->buf_size * 8) >> 8) & 0xff;
    internal[0] = (CTX->buf_size * 8) & 0xff;

    Padding(CTX);
    G(CTX->h, CTX->N, (const uint8_t*)&(CTX->buffer));

    Add512(CTX->N, internal, CTX->N);
    Add512(CTX->Sigma, CTX->buffer, CTX->Sigma);

    G(CTX->h, CTX->v_0, (const uint8_t*)&(CTX->N));
    G(CTX->h, CTX->v_0, (const uint8_t*)&(CTX->Sigma));

    memcpy(CTX->hash, CTX->h, BLOCK_SIZE);
}

void Update(TContext *CTX, const uint8_t *data, size_t len)
{
    while((len > 63) && (CTX->buf_size)==0)
    {
        Stage_2(CTX, data);
        data += 64;
        len -= 64;
    }

    size_t chk_size;
    while(len)
    {
        chk_size = 64 - CTX->buf_size;
        if(chk_size > len) chk_size = len;
        memcpy(&CTX->buffer[CTX->buf_size], data, chk_size);
        CTX->buf_size += chk_size;
        len -= chk_size;
        data += chk_size;
        if(CTX->buf_size == 64)
        {
            Stage_2(CTX, CTX->buffer);
            CTX->buf_size = 0;
        }
    }
}

void Final(TContext *CTX)
{
    Stage_3(CTX);
    CTX->buf_size = 0;
}

/*uint8_t * Hash(const char * file, uint16_t hash_size)   
{
    uint8_t FILE_BUFFER_SIZE = 4096;
    TContext *CTX;
    printf("check\n");
    Init(CTX, hash_size);
    printf("check\n");
    size_t buffer = malloc((size_t) FILE_BUFFER_SIZE);
    size_t len;
    while ((len = fread(buffer, (size_t) 1, (size_t) FILE_BUFFER_SIZE, file)))
    {
        Update(CTX, buffer, len);
    }
    Final(CTX);
    return CTX->hash;
}*/