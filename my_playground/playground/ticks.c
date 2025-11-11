#include <stdint.h>

/* 
 * Giả sử getticks() dùng CSR cycle hoặc custom perfcounter.
 * Nếu bạn đã có perfcounter.S thì có thể gọi get_cycles() trực tiếp.
 */
extern uint64_t get_cycles(void);

uint64_t getticks(void) {
    return get_cycles();
}

/* Ví dụ một hàm dùng ticks */
uint64_t measure_example_cycles(void) {
    uint64_t start = getticks();
    // Thực hiện một phép tính ví dụ
    for (volatile int i=0;i<100;i++);
    uint64_t end = getticks();
    return end - start;
}