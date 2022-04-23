#include "const_val.h"


#define BLOCK_SIZE 64 // Размер блока — 64 байта

typedef uint8_t vect[BLOCK_SIZE]; // Определяем тип vect как 64-байтовый массив


static void X(const uint8_t *a, const uint8_t *b, const uint8_t *c)
{
    for(int i=0; i<64; i++)
    c[i] = a[i]^b[i];
}

static void Add512(const uint8_t *a, const uint8_t *b, const uint8_t *c)
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
    memcpy(state, internal, BLOCK_SIZE)
}

static void L(uint8_t *state)
{
    unit64_t internal_in = (unit64_t*)state;
    unit64_t internal_out[8];
    memset(internal_out, 0x00, BLOCK_SIZE);
    for(int i=7; i>=0; i--)
        for(int j=63; j>=0; j--)
            if((internal_in >> j) & 1)
                internal_out[i] ^= A[63 - j];
    memcpy(state, internal_out, 64)
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
