#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
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
    uint32_t low = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t middle = (uint32_t)(challenge >> 16);

    uint32_t step1 =
        (high * UINT32_C(3)) ^ (middle + UINT32_C(0x7a5b));

    uint32_t step0 =
        low * UINT32_C(4) + (high ^ UINT32_C(0x9581));

    uint32_t values[MAX_FIELDS] = {0};
    values[0] =
        (step0 + step1 * UINT32_C(8)) ^ UINT32_C(0x9bfd);

    return write_answer(out, cap, values, 1);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;

    values[0] = (uint32_t)((challenge >> 36) & UINT64_C(63));

    for (unsigned i = 1; i < 6; ++i) {
        uint32_t part =
            (uint32_t)((challenge >> (3 * i + 3)) & UINT64_C(63));

        values[i] = values[i - 1] * UINT32_C(2)
                  + (part ^ (uint32_t)i);
    }

    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t index = (uint32_t)(challenge & UINT64_C(7));
    uint32_t x = (uint32_t)(challenge >> 32)
               ^ (uint32_t)(challenge >> 3);
    uint32_t result = 0;

    switch (index) {
        case 0:
            result = x * UINT32_C(5) + UINT32_C(0x171d8);
            break;
        case 1:
            result = x * UINT32_C(5) + UINT32_C(0xaebb);
            break;
        case 2:
            result = x * UINT32_C(3) + UINT32_C(0x2fcb);
            break;
        case 3:
            result = (x * UINT32_C(2)) ^ UINT32_C(0x48cc);
            break;
        case 4:
            result = x * UINT32_C(5) + UINT32_C(0x2f18e);
            break;
        case 5:
            result = (x * UINT32_C(2)) ^ UINT32_C(0xfe55);
            break;
        case 6:
            result = (x ^ UINT32_C(0xc57b)) + (x >> 1);
            break;
        case 7:
            result = (x ^ UINT32_C(0x19fd)) + (x >> 2);
            break;
    }

    values[0] = index;
    values[1] = result;
    return write_answer(out, cap, values, 2);
}
static uint32_t phase4_walk(uint32_t target, uint32_t low,
                            uint32_t high, uint32_t key) {
    uint32_t mid = low + ((high - low) >> 1);

    if (target == mid) {
        return (mid + UINT32_C(0xf5)) ^ key;
    }

    uint32_t child;
    uint32_t tag;

    if (target < mid) {
        child = phase4_walk(target, low, mid - 1, key);
        tag = UINT32_C(4);
    } else {
        child = phase4_walk(target, mid + 1, high, key);
        tag = UINT32_C(0xb);
    }

    return (child * UINT32_C(5)) ^ (mid + key) ^ tag;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};

    uint32_t target =
        (uint32_t)(challenge & UINT64_C(127));

    uint32_t key =
        (uint32_t)(challenge >> 7)
        ^ (uint32_t)(challenge >> 32);

    values[0] = phase4_walk(target, 0, 127, key);

    return write_answer(out, cap, values, 1);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    static const uint32_t ids[] = {
        2441, 816, 2618, 1890, 1156, 293,
        91, 1551, 2876, 1347, 589
    };

    static const uint32_t node_values[] = {
        45382, 61523, 16502, 64376, 62994, 5010,
        62999, 49087, 2012, 18032, 49200
    };

    unsigned start = (unsigned)(challenge & UINT64_C(7));
    unsigned steps =
        (unsigned)((challenge >> 3) & UINT64_C(3));

    uint32_t result =
        (uint32_t)(challenge >> 6)
        ^ (uint32_t)(challenge >> 32)
        ^ UINT32_C(0x7569);

    uint32_t values[MAX_FIELDS] = {0};

    for (unsigned i = 0; i <= steps; ++i) {
        unsigned pos = start + i;
        result = (result + node_values[pos]) * UINT32_C(5);
        result ^= ids[pos];
        values[1] = ids[pos];
    }

    values[0] = result;
    return write_answer(out, cap, values, 2);
}
static uint32_t phase6_op(unsigned index, uint32_t x) {
    switch (index) {
        case 0:
            return (x >> 5) | (x << 27);
        case 1:
        case 2:
            return ((x >> 2) ^ x) + UINT32_C(0x5867);
        case 3:
            return ((x << 5) | (x >> 27)) ^ UINT32_C(0x7cb3);
        case 4:
        case 5:
            return (x + UINT32_C(0x8e86)) ^ (x >> 7);
        case 6:
            return (x * UINT32_C(8) + UINT32_C(0x42bb))
                 ^ (x * UINT32_C(3));
        case 7:
            return (x ^ UINT32_C(0xc707)) + (x << 5);
        default:
            return 0;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};

    uint32_t x = (uint32_t)(challenge >> 6)
               ^ (uint32_t)(challenge >> 32);

    unsigned first =
        (unsigned)(challenge & UINT64_C(7));
    unsigned second =
        (unsigned)((challenge >> 3) & UINT64_C(7));

    x = phase6_op(first, x);
    x ^= UINT32_C(0xf3b5);
    x = phase6_op(second, x);
    values[0] = x + UINT32_C(0xf631);

    return write_answer(out, cap, values, 1);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t first = (uint32_t)(context & UINT64_C(3));

    uint32_t a = (uint32_t)challenge
               ^ (uint32_t)(challenge >> 32);
    uint32_t b = (uint32_t)context
               ^ (uint32_t)(context >> 32);

    uint32_t mixed =
        ((a ^ b) * UINT32_C(5) + UINT32_C(0xe9d6))
        ^ UINT32_C(0xa696);

    uint32_t second =
        mixed
        ^ (first * UINT32_C(0xe9d6))
        ^ (uint32_t)(context >> 16);

    uint32_t third =
        second ^ (b + first + UINT32_C(0xa696) + (a >> 1));

    int size = snprintf(
        out, cap,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOPEN"
        "%" PRIu32 " %" PRIu32 " %" PRIu32,
        first, second, third
    );

    return size < 0 || (size_t)size >= cap;
}
