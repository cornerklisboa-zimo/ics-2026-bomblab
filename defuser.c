#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
typedef struct {
    uint32_t f0;
    uint32_t f1;
} pair;

static const pair CHAIN[8][4] = {
{{0x00009ddb, 0x00000768}, {0x00007dd2, 0x00000557}, {0x00007b9a, 0x00000465}, {0x000030c7,0x00000962}},
{{0x00007dd2, 0x00000557}, {0x00007b9a, 0x00000465}, {0x000030c7, 0x00000962}, { 0x00006fac, 0x00000a28}},
{{0x00007b9a, 0x00000465}, {0x000030c7, 0x00000962}, {0x00006fac, 0x00000a28}, { 0x000095c8, 0x00000055}},
{{0x000030c7, 0x00000962}, {0x00006fac, 0x00000a28}, {0x000095c8, 0x00000055}, { 0x0000b8c2, 0x00000887}},
{{0x00006fac, 0x00000a28}, {0x000095c8, 0x00000055}, {0x0000b8c2, 0x00000887}, { 0x0000df82, 0x00000106}},
{{0x000095c8, 0x00000055}, {0x0000b8c2, 0x00000887}, {0x0000df82, 0x00000106}, { 0x00003da9, 0x00000251}},
{{0x0000b8c2, 0x00000887}, {0x0000df82, 0x00000106}, {0x00003da9, 0x00000251}, {0x00004b4f, 0x00000614}},
{{0x0000df82, 0x00000106}, {0x00003da9, 0x00000251}, {0x00004b4f, 0x00000614}, { 0x0000acf2, 0x00000b53}}
};

uint32_t phase04_walk(uint32_t a, uint32_t b, uint32_t c,uint32_t d){
    uint32_t res = a;
    a = ((c - b)>>1)+b;
    if (res == a){
        return ( a + UINT32_C(0xd7)) ^ d;
    }
    else{
        uint32_t x = a + d ; //ebx
        if ( res < a){
            c = a - 1;
            a = res;
            return ((3 * phase04_walk(a, b, c, d))^x)^UINT32_C(0x1d);
        }
        else{
            b = 1 + a;
            a = res;
            return ((3* phase04_walk(a, b, c, d)) ^ x) ^UINT32_C(0x11);
        }
    }
}

uint32_t phase05_traverse(uint64_t a, uint32_t* out){
    uint32_t selector = (uint32_t)(a & UINT32_C(0X7));
    uint32_t x = (uint32_t)((a >> 3) & UINT32_C(0X3));//rcx,edi
    uint32_t y = ((uint32_t)(a >> 6) ^ (uint32_t)(a >> 32)) ^UINT32_C(0xc89);
    uint32_t val_1 = CHAIN[selector][0].f0;
    uint32_t val_2 = CHAIN[selector][0].f1;
    uint32_t result = ((y + val_1) * 5) ^ (val_2);
    *out = val_2;
    for ( uint32_t i = 1; i <= x; i++){
        val_1 = CHAIN[selector][i].f0;//rdx
        val_2 = CHAIN[selector][i].f1;//rdx+4
        result = ((result + val_1) * 5) ^ val_2;
        *out = val_2;
    }
    return result;
}

uint32_t phase06_changeValue(uint32_t sele, uint32_t x) {
    switch (sele) {
        case 0: case 7:  // op_4
            return (x >> 1) | (x << 31);
        case 1: case 4:  // op_5
            return ((x >> 7) ^ x) + UINT32_C(0xb1da);
        case 2:  // op_0
            return (x ^ UINT32_C(0x3ded)) + (x << 7);
        case 3:  // op_1
            return (x + UINT32_C(0x51c0)) ^ (x >> 5);
        case 5:  // op_3
            return ((x << 7) + UINT32_C(0x3b9)) ^ (3 * x);
        case 6:  // op_2
            return ((x << 3) | (x >> 29)) ^ UINT32_C(0xdb7);
        default:
            return sele;
    }
}

uint32_t phase06_dispatch(uint64_t a){
    uint32_t selector = (uint32_t)a & UINT32_C(0X7);
    uint32_t x = (uint32_t)(a >> 6) ^ (uint32_t)(a >>32);//rdi
    uint32_t y = (uint32_t)a & UINT32_C(0X38);//rbx
    uint32_t result = selector;
    result = phase06_changeValue(selector, x);
    result = result ^ UINT32_C(0Xeb18);
    result = phase06_changeValue((y/8), result);
    return result + UINT32_C(0Xff6);
}
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    (void)challenge;
    // TODO: 根据 phase_01 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t a = (uint32_t)(challenge >> 32);
    uint32_t b = (uint32_t)(challenge >> 16);
    uint32_t c = (uint32_t)(challenge);

    uint32_t r1 = (5*a + b) ^ UINT32_C(0xc263);
    uint32_t r0 = (a + UINT32_C(0xc377)) ^ c;
    uint32_t r2 = 3 * (r0 - r1) + UINT32_C(0x7529);

    values[count-1] = r2;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;
    (void)challenge;
    // TODO: 根据 phase_02 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    values[0] = (( challenge >> 1) & UINT32_C(0x1f) ) *3;
    uint32_t prev = values[0];
    uint32_t move = 26;
    for (size_t i = 1; i < count; i++){
        uint32_t prev_c = prev*3 - i;
        uint32_t result = ((challenge >> move) & UINT32_C(0xf)) + 10 ;
        values[i] = result ^ prev_c;
        prev = values[i];
        move+=5;
    }
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    (void)challenge;
    // TODO: 根据 phase_03 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    values[0] = challenge & UINT32_C( 0x7);
    uint32_t selector = values[0]; //edi
    uint32_t x = (uint32_t)(challenge >> 32);
    x = x ^ ((uint32_t)(challenge >> 3));//esi
    uint32_t result = 0;
    switch( selector){
        case 0://case_4
            result = 3*x + UINT32_C(0x9c87);
            break;
        case 1://case_0
            result = (x ^  UINT32_C(0x9a6b) ) + (x >> 1);
            break;
        case 2://case_3
            result = 5*x + UINT32_C( 0x3a6e2);
            break;
        case 3://case_6
            result = 5*x + UINT32_C( 0x3d7f2);
            break;           
        case 4://case_7
            result = (x ^  UINT32_C(0x2aa0) ) + (x >> 1);
            break;       
        case 5://case_5
            result = (4*x) ^ (UINT32_C(0xa921));
            break;
        case 6://case_2
            result = (4*x) ^ (UINT32_C(0x8257));
            break;
        case 7://case_1
            result = 3*x + UINT32_C(0x9a2f);
            break;    
        default:
            break;
    }
    values[1] = result;
    return write_answer(out, cap, values, count);
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    (void)challenge;
    // TODO: 根据 phase_04 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t a = ((uint32_t)challenge) & UINT32_C(0x7f);
    uint32_t b = 0;
    uint32_t c = UINT32_C(0x7f);
    uint32_t d = (uint32_t)((challenge >> 32) ^ (challenge >> 7));
    uint32_t result = phase04_walk(a, b, c, d);
    values[0] = result;
    return write_answer(out, cap, values, count);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    (void)challenge;
    // TODO: 根据 phase_05 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t last;
    uint32_t result = phase05_traverse(challenge, &last);
    values[0] = result;
    values[1] = last;
    return write_answer(out, cap, values, count);
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    (void)challenge;
    // TODO: 根据 phase_06 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    values[0] = phase06_dispatch(challenge);
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    (void)challenge;
    (void)context;
    (void)out;
    (void)cap;
    // TODO（选做）: 根据 secret_gate、secret_copy 和 secret_phase 的汇编构造完整的第七行，写入 out 后返回 0。
    // 第七行不一定只有整数，可以直接用 snprintf 写入；还没完成时保持返回 1，测试会显示 not attempted。
    uint32_t ch_lo = (uint32_t)(challenge & 0xFFFFFFFF);
    uint32_t ch_hi = (uint32_t)(challenge >> 32);
    uint32_t ctx_lo = (uint32_t)(context & 0xFFFFFFFF);
    uint32_t ctx_hi = (uint32_t)(context >> 32);

    uint32_t a = (uint32_t)(context & 3);

    uint32_t esi = ch_hi ^ ch_lo;
    uint32_t r8d = ctx_hi ^ ctx_lo;

    uint32_t mix_in = esi ^ r8d;
    uint32_t mix_out = (mix_in >> 5) | (mix_in << 27);  // ror 5
    mix_out ^= UINT32_C(0x1c1a);

    uint32_t r9d = (a * UINT32_C(0x281c)) ^ (uint32_t)(context >> 16);
    uint32_t b = mix_out ^ r9d;

    uint32_t c = b ^ (r8d + a + UINT32_C(0x3406) + (esi >> 5));

    // 前32个字符随便填，"OPEN"必须在第32字节位置（即总共36字节前缀）
    snprintf(out, cap, "12345678901234567890123456789012OPEN %u %u %u", a, b, c);    
    
    return 0;
}
