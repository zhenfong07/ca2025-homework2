#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* ================================================
 * CÁC HÀM IN (Giữ nguyên tối ưu hóa I/O)
 * ================================================ */
extern uint64_t getticks(void);

void printstr(const char *ptr, size_t length) {
    register const char* a1_ptr asm("a1") = ptr;
    register size_t a2_len asm("a2") = length;
    asm volatile(
        "li a7, 64\n\t"    // sys_write
        "li a0, 1\n\t"     // stdout
        "ecall\n\t"
        :
        : "r"(a1_ptr), "r"(a2_len) 
        : "a0","a7" 
    );
}

size_t strlen(const char *s) {
    size_t count = 0;
    while (*s++) count++;
    return count;
}

#define TEST_LOGGER(msg)                         \
    do {                                         \
        const char *_m = msg "\n";               \
        printstr(_m, strlen(_m));                \
    } while(0)

static inline uint32_t div10_u32(uint32_t n) {
    return (uint32_t)(((uint64_t)n * 0xCCCCCCCDULL) >> 35);
}

void print_dec_and_newline(unsigned long val) {
    char buf[24];
    char *p = buf + sizeof(buf) - 2; 
    *p = '\n'; 
    p++;
    *p = '\0';
    p--;

    if (val == 0) { 
        *p = '0'; p--; 
    } else {
        uint32_t v = (uint32_t)val;
        while (v) {
            uint32_t q = div10_u32(v);
            // Tối ưu hóa: Dùng phép nhân 32-bit cho r, compiler sẽ tối ưu.
            uint32_t r = v - q * 10; 
            *p = '0' + r;
            p--; v = q;
        }
    }
    p++;
    printstr(p, (buf + sizeof(buf) - 1) - p);
}

/* ================================================
 * Khai báo từ perfcounter.S
 * ================================================ */
extern uint64_t get_cycles(void);
extern uint64_t get_instret(void);

/* ================================================
 * RSQRT + distance 3D (Tối ưu hóa tính toán)
 * ================================================ */
static inline unsigned clz(uint32_t x) {
    if (x==0) return 32;
    unsigned n=0; uint32_t bit=0x80000000;
    while ((x & bit)==0) { n++; bit>>=1; }
    return n;
}

static const uint16_t rsqrt_table[32] = {
    65535,46341,32768,23170,16384,11585,8192,5793,
    4096,2896,2048,1448,1024,724,512,362,
    256,181,128,90,64,45,32,23,
    16,11,8,6,4,3,2,1
};

uint32_t fast_rsqrt(uint32_t x) {
    if (x<=1) return (x==0) ? 0xFFFFFFFFu : 65536; // Tối ưu hóa điều kiện biên

    uint32_t exp = 31 - clz(x);
    if (exp >= 32) exp = 31; 
    
    uint32_t y_base = rsqrt_table[exp];
    uint32_t y_next = (exp == 31) ? 1 : rsqrt_table[exp + 1]; 
    uint32_t delta = y_base - y_next;
    
    // Tối ưu hóa: Đơn giản hóa frac_scaled
    uint32_t x_frac = x - (1U << exp);
    uint32_t frac = (x_frac << 16) >> exp;
    
    // Newton-Raphson: Sử dụng hằng số và khai báo biến trung gian 64-bit 
    // để trình biên dịch có thể tận dụng lệnh MULH/MUL.
    const uint32_t magic_num_3 = (3U << 16);
    
    uint32_t interp = (uint32_t)(((uint64_t)delta * frac) >> 16);
    uint32_t y = y_base - interp;

    // Bước 1
    uint32_t y2 = (uint32_t)(((uint64_t)y * y) >> 16);
    uint32_t xy2 = (uint32_t)((uint64_t)x * y2);
    uint64_t correction_1 = (uint64_t)y * (magic_num_3 - xy2);
    y = (uint32_t)(correction_1 >> 17);

    // Bước 2
    y2 = (uint32_t)(((uint64_t)y * y) >> 16);
    xy2 = (uint32_t)((uint64_t)x * y2);
    uint64_t correction_2 = (uint64_t)y * (magic_num_3 - xy2);
    y = (uint32_t)(correction_2 >> 17);

    return y;
}

uint32_t fast_distance_3d(int32_t dx,int32_t dy,int32_t dz) {
    // Tối ưu hóa: Sử dụng 64-bit cho phép toán, trình biên dịch có thể tận dụng M extension nếu có
    uint64_t dist_sq = (uint64_t)dx * dx + (uint64_t)dy * dy + (uint64_t)dz * dz;
    
    if (dist_sq > 0xFFFFFFFFu) dist_sq >>= 16;
    
    if (dist_sq == 0) return 0;
    
    uint32_t inv = fast_rsqrt((uint32_t)dist_sq);
    
    uint64_t dist_scaled = (uint64_t)dist_sq * inv;
    
    // Rounding (giữ nguyên)
    uint32_t dist = (uint32_t)((dist_scaled + (1U << 15)) >> 16);
    
    return dist;
}

bool test_rsqrt() {
    int32_t dx=3,dy=4,dz=12;
    uint32_t distance = fast_distance_3d(dx,dy,dz);
    TEST_LOGGER("Test fast_distance_3d(3,4,12):");
    TEST_LOGGER("Expected: 13");
    TEST_LOGGER("Actual: ");
    print_dec_and_newline(distance);
    return (distance==13);
}

/* ================================================
 * MAIN
 * ================================================ */
int main(void) {
    uint64_t start_cycles, end_cycles, cycles_c;
    uint64_t start_instret, end_instret, instret_c;
    uint64_t start_ticks, end_ticks, ticks_c;

    TEST_LOGGER("=== Start Test Suite (Problem C: rsqrt) ===\n\n");

    TEST_LOGGER("--- Problem C: fast_rsqrt ---\n");

    start_cycles = get_cycles();
    start_instret = get_instret();
    start_ticks = getticks();

    bool pass_c = test_rsqrt();

    end_cycles = get_cycles();
    end_instret = get_instret();
    end_ticks = getticks();

    cycles_c = end_cycles - start_cycles;
    instret_c = end_instret - start_instret;
    ticks_c = end_ticks - start_ticks;

    TEST_LOGGER("\n== Test Result: ");
    if (pass_c) { TEST_LOGGER("PASSED\n"); }
    else { TEST_LOGGER("FAILED\n"); }

    TEST_LOGGER("\n--- Performance Statistics ---\n");
    TEST_LOGGER("Cycles: ");
    print_dec_and_newline(cycles_c);
    TEST_LOGGER("Instructions: ");
    print_dec_and_newline(instret_c);
    TEST_LOGGER("Ticks: ");
    print_dec_and_newline(ticks_c);

    TEST_LOGGER("\n=== End of Test Suite ===\n");

    return 0; 
}