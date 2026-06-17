#include <riscv_vector.h>
#include <cstdint>
#include <cstdio>

int main() {
    const int n = 32;
    uint8_t input[n];
    uint8_t output[n];

    for (int i = 0; i < n; i++) {
        input[i] = i;
    }

    int i = 0;

    while (i < n) {
        size_t vl = __riscv_vsetvl_e8m1(n - i);

        vuint8m1_t v = __riscv_vle8_v_u8m1(&input[i], vl);
        v = __riscv_vadd_vv_u8m1(v, v, vl);
        __riscv_vse8_v_u8m1(&output[i], v, vl);

        i += vl;
    }

    for (int i = 0; i < n; i++) {
        printf("%d ", output[i]);
    }
    printf("\n");

    return 0;
}
