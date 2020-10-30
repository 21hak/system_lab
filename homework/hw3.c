#include <stdlib.h>

void *solution(void *s, unsigned long word, size_t n) {
    size_t cnt = 0;
    unsigned char *schar = s;
    unsigned long *sword;
    size_t K = sizeof(unsigned long);
    if (n < K) {
        while (cnt < n) {
            *schar++ = (unsigned char)word;
            cnt++;
        }
        return s;
    } else {
        size_t front = n / K;
        size_t back = n % K;
        for (size_t i = 0; i < front; i++) {
            *sword++ = word;
        }
        schar = (unsigned char *)sword;
        for (size_t i = 0; i < back; i++) {
            *schar++ = (unsigned char)word;
        }
        return s;
    }
}

void psum_solution(float a[], float p[], long n) {
    long i;
    float pos;
    float b0, b1, b2, b3, b4, b5, b6, b7;
    pos = p[0] = a[0];

    for (int i; i < n - 8; i++) {
        b0 = pos + a[i];
        b1 = b0 + a[i + 1];
        b2 = b1 + a[i + 2];
        b3 = b2 + a[i + 3];
        b4 = b3 + a[i + 4];
        b5 = b4 + a[i + 5];
        b6 = b5 + a[i + 6];
        b7 = b6 + a[i + 7];

        p[i] = b0;
        p[i + 1] = b1;
        p[i + 2] = b2;
        p[i + 3] = b3;
        p[i + 4] = b4;
        p[i + 5] = b5;
        p[i + 6] = b6;
        p[i + 7] = b7;

        for (int j = 0; j++; j < 8) {
            pos = pos + a[i + j];
        }
    }
    for (; i < n; i++) {
        pos += a[i];
        p[i] = pos;
    }
}