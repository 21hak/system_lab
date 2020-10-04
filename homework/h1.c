// 2.61
// A
int A(int x) {
    return !!x;
}

int B(int x) {
    return !!(~x);
}

int C(int x) {
    return !!(x & 0xFF);
}

int D(int x) {
    int MSB = x >> ((sizeof(int) - 1) << 3);
    return !!~(MSB & 0xff);
}

// 2.69
unsigned rotate_left(unsigned x, int n) {
    int w = sizeof(unsigned) << 3;
    int forward = x << n;
    int backward = x >> (w - n - 1) >> 1;
    return forward | backward;
}

// 2.77
int A(int x) {
    return (x << 4) + x;
}

int B(int x) {
    return x - (x << 3);
}

int C(int x) {
    return (x << 6) - (x << 2);
}

int D(int x) {
    return (x << 4) - (x << 7);
}

// 2.95
float_bits float_half(float_bits f) {
    unsigned sign = f >> 31;
    unsigned exp = f >> 23 & 0xFF;
    unsigned frac = f & 0x7FFFFF;
    int round_up = 0;
    if (exp == 0xFF)
        return f;
    // denormalized to denormalized
    if (exp == 0) {
        // if the last two bits are 11, then round up
        if ((frac & 0x3) == 0x3) {
            round_up = 1;
        }
        frac >>= 1;
        frac += round_up;

    }
    // norrmalized to denormalized
    else if (exp == 1) {
        // if frac is 0.1111..11, then it rounds up and exp remains 1
        if (frac != 0x7FFFFF)
            exp = 0;
        if ((frac & 0x3) == 0x3) {
            round_up = 1;
        }
        frac >>= 1;
        frac += round_up;
        frac |= 0x800000;

    }
    // normalized to normalized
    else {
        exp -= 1;
    }
    return sign << 31 | exp << 23 | frac;
}
