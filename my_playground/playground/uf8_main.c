#include <stdbool.h>
#include <stdint.h>
#include <stddef.h> // Cần thiết cho kiểu size_t

/* ================================================
 * HÀM IN BARE-METAL (Dùng ECALL cho rv32emu -system)
 * ================================================ */

// Đây là macro 'printstr' gốc từ tệp main.c mẫu.
// Nó sử dụng 'ecall' (syscall) để in, đây là cách
// 'rv32emu -system' mong đợi.
#define printstr(ptr, length) \
    do { \
        asm volatile( \
            "add a7, x0, 0x40;" /* 64 = sys_write */ \
            "add a0, x0, 0x1;"  /* 1 = stdout */ \
            "add a1, x0, %0;"   /* con trỏ buffer */ \
            "mv a2, %1;"        /* độ dài */ \
            "ecall;" \
            : \
            : "r"(ptr), "r"(length) \
            : "a0", "a1", "a2", "a7"); \
    } while (0)

// Cập nhật macro TEST_OUTPUT và TEST_LOGGER
#define TEST_OUTPUT(msg, length) printstr(msg, (size_t) length)

#define TEST_LOGGER(msg)                       \
    {                                          \
        char _msg[] = msg;                     \
        TEST_OUTPUT(_msg, sizeof(_msg) - 1); \
    }

/* ================================================
 * CÁC HÀM HỖ TRỢ IN SỐ (Sửa để dùng printstr)
 * ================================================ */

static unsigned long udiv(unsigned long dividend, unsigned long divisor) {
    if (divisor == 0) return 0;
    unsigned long quotient = 0, remainder = 0;
    for (int i = 31; i >= 0; i--) {
        remainder <<= 1;
        remainder |= (dividend >> i) & 1;
        if (remainder >= divisor) {
            remainder -= divisor;
            quotient |= (1UL << i);
        }
    }
    return quotient;
}

static unsigned long umod(unsigned long dividend, unsigned long divisor) {
    if (divisor == 0) return 0;
    unsigned long remainder = 0;
    for (int i = 31; i >= 0; i--) {
        remainder <<= 1;
        remainder |= (dividend >> i) & 1;
        if (remainder >= divisor) {
            remainder -= divisor;
        }
    }
    return remainder;
}

static void print_hex(unsigned long val) {
    char buf[20];
    char *p = buf + sizeof(buf) - 1;
    *p = '\n'; // Thêm newline
    p--;
    if (val == 0) {
        *p = '0'; p--;
    } else {
        while (val > 0) {
            int digit = val & 0xf;
            *p = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
            p--;
            val >>= 4;
        }
    }
    p++;
    // Dùng printstr thay vì printstr_baremetal
    printstr(p, (buf + sizeof(buf) - p));
}

static void print_dec(unsigned long val) {
    char buf[20];
    char *p = buf + sizeof(buf) - 1;
    *p = '\n'; // Thêm newline
    p--;
    if (val == 0) {
        *p = '0'; p--;
    } else {
        while (val > 0) {
            *p = '0' + umod(val, 10);
            p--;
            val = udiv(val, 10);
        }
    }
    p++;
    // Dùng printstr thay vì printstr_baremetal
    printstr(p, (buf + sizeof(buf) - p));
}

/* ================================================
 * ĐO LƯỜNG HIỆU NĂNG (Giữ nguyên)
 * ================================================ */
extern uint64_t get_cycles(void);
extern uint64_t get_instret(void);


/* ================================================
 * HOMEWORK 1: UF8 IMPLEMENTATION (Mã của bạn)
 * ================================================ */
typedef uint8_t uf8;

static inline unsigned clz(uint32_t x) {
    int n = 32, c = 16;
    do {
        uint32_t y = x >> c;
        if (y) {
            n -= c;
            x = y;
        }
        c >>= 1;
    } while (c);
    return n - x;
}

uint32_t uf8_decode(uf8 fl) {
    uint32_t mantissa = fl & 0x0f;
    uint8_t exponent = fl >> 4;
    uint32_t offset = (0x7FFF >> (15 - exponent)) << 4;
    return (mantissa << exponent) + offset;
}

uf8 uf8_encode(uint32_t value) {
    if (value < 16)
        return value;
    int lz = clz(value);
    int msb = 31 - lz;
    uint8_t exponent = 0;
    uint32_t overflow = 0;
    if (msb >= 5) {
        exponent = msb - 4;
        if (exponent > 15)
            exponent = 15;
        for (uint8_t e = 0; e < exponent; e++)
            overflow = (overflow << 1) + 16;
        while (exponent > 0 && value < overflow) {
            overflow = (overflow - 16) >> 1;
            exponent--;
        }
    }
    while (exponent < 15) {
        uint32_t next_overflow = (overflow << 1) + 16;
        if (value < next_overflow)
            break;
        overflow = next_overflow;
        exponent++;
    }
    uint8_t mantissa = (value - overflow) >> exponent;
    return (exponent << 4) | mantissa;
}


/* ================================================
 * TEST SUITE CHO HW1
 * ================================================ */

static bool test_uf8_roundtrip(void) {
    int32_t previous_value = -1;
    bool passed = true;

    for (int i = 0; i < 256; i++) {
        uint8_t fl = i;
        int32_t value = uf8_decode(fl);
        uint8_t fl2 = uf8_encode(value);

        if (fl != fl2) {
            TEST_LOGGER("FAIL: "); print_hex(fl);
            TEST_LOGGER("  produces value "); print_dec(value);
            TEST_LOGGER("  but encodes back to "); print_hex(fl2);
            passed = false;
        }

        if (value <= previous_value) {
            TEST_LOGGER("FAIL: "); print_hex(fl);
            TEST_LOGGER("  value "); print_dec(value);
            TEST_LOGGER("  <= previous_value "); print_dec(previous_value);
            passed = false;
        }
        previous_value = value;
    }
    return passed;
}

/* ================================================
 * HÀM MAIN() CHÍNH
 * ================================================ */
int main(void) {
    uint64_t start_cycles, end_cycles, cycles_elapsed;
    uint64_t start_instret, end_instret, instret_elapsed;

    TEST_LOGGER("=== Bat dau Test Suite cho Homework 1: UF8 ===\n");

    // Lấy thời gian bắt đầu
    start_cycles = get_cycles();
    start_instret = get_instret();

    // Chạy test suite
    bool all_passed = test_uf8_roundtrip();

    // Lấy thời gian kết thúc
    end_cycles = get_cycles();
    end_instret = get_instret();

    // Tính toán
    cycles_elapsed = end_cycles - start_cycles;
    instret_elapsed = end_instret - start_instret;

    // In kết quả test
    if (all_passed) {
        TEST_LOGGER("== Homework 1: All tests PASSED ==\n");
    } else {
        TEST_LOGGER("== Homework 1: FAILED ==\n");
    }

    // In kết quả hiệu năng
    TEST_LOGGER("Cycles: ");
    print_dec((unsigned long) cycles_elapsed);
    TEST_LOGGER("Instructions: ");
    print_dec((unsigned long) instret_elapsed);
    TEST_LOGGER("\n=== All Tests Completed ===\n");

    return 0; // Sẽ thoát về start.S, sau đó gọi ecall (sys_exit)
}
