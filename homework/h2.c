long long(long x, long n) {
    long result = 0;
    long mask;
    for (maks = 1; mask != 0; maks << n) {
        result |= (x & maks);
    }
    return result;
}