//
// rid.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <random>
#include <vector>
#include <limits>
#include <chrono>
#include <cinttypes>
#include <cstring>
#include <cassert>

#if defined(_MSC_VER) && defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <WinBase.h>
#include <processthreadsapi.h>
#endif

// How many numbers to test
constexpr size_t SZ = 500000000;

// Plain division
template<typename T> T plaindiv(T n, T d) {
    return n / d;
}

// Round to nearest integer division, mid toward zero
template<typename T> T math_rounddiv(T n, T d) {
    return n / d + (n % d) / (d / 2 + 1);
}

// Round to nearest integer division, mid away from zero
template<typename T> T math_rounddiv_away(T n, T d) {
    return n / d + (n % d) / (d / 2 + d % 2);
}

// Round to Zero Division, no overflow
template<typename T> static T rounddiv(T n, T d) {
    T m = n % d;
    T h = d / 2;
    return n / d + (!(n < 0) & (m > h)) - ((n < 0) & ((m + h) < 0));
}

// Round from Zero Division, no overflow
template<typename T> static T rounddiv_away(T n, T d) {
    T m = n % d;
    T h = d / 2 + d % 2;
    return n / d + (!(n < 0) & (m >= h)) - ((n < 0) & ((m + h) <= 0));
}

// Plain truncating division done in floating point
template<typename T> T plaindivfp(T n, T d) {
    return static_cast<T>(double(n) / d);
}

// round trip to floating point, symmetric around zero
template<typename T> T rtfp(T n, T d) {
    return static_cast<T>((n > 0) ? ((n + 0.5 * d) / d) : ((n - 0.5 * d) / d));
}

// ceiling via floating point
template<typename T> T rupfp(T n, T d) {
    return static_cast<T>((n + 0.5 * d) / d);
}

// Generic function type for integer division
template<typename T> using divFunc = T(*)(T, T);

// Wrapper to handle each function
template<typename T> T timeit(std::vector<T> &v, divFunc<T> f, T d) {
    T sum = 0;
    for (auto& i : v)
        sum += f(i, d);
    return sum;
}

template<typename T> T timeit(std::vector<T>& v, divFunc<T> f, std::vector<T> &d) {
    T sum = 0;
    for (int i = 0; i < v.size(); i++)
        sum += f(v[i], d[i]);
    return sum;
}

template <typename T> uint64_t ttest(std::vector<T>& v, std::vector<T> &d, const char *name) {
    assert(v.size() == d.size());
    int64_t value(0);
    auto t0 = std::chrono::high_resolution_clock::now();
    if (0 == strcmp(name, "div"))
        value = timeit(v, plaindiv, d);
    else if (0 == strcmp(name, "math_rounddiv_away"))
        value = timeit(v, math_rounddiv_away, d);
    else if (0 == strcmp(name, "math_rounddiv"))
        value = timeit(v, math_rounddiv, d);
    else if (0 == strcmp(name, "rounddiv"))
        value = timeit(v, rounddiv, d);
    else if (0 == strcmp(name, "rounddiv_away"))
        value = timeit(v, rounddiv_away, d);
    else if (0 == strcmp(name, "fpdiv"))
        value = timeit(v, plaindivfp, d);
    else if (0 == strcmp(name, "rtfp"))
        value = timeit(v, rtfp, d);
    else if (0 == strcmp(name, "rupfp"))
        value = timeit(v, rupfp, d);
    else
        assert(false);  
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << name << ": " << value << " : " << 
        1e-6 * std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() << " seconds\n";
    return value;
}

// Time a function with a single divisor
template <typename T> uint64_t ttest(std::vector<T>& v, T d, const char* name) {
    auto t0 = std::chrono::high_resolution_clock::now();
    int64_t value = 0;
    if (0 == strcmp(name, "div"))
        value = timeit(v, plaindiv, d);
    else if (0 == strcmp(name, "math_rounddiv_away"))
        value = timeit(v, math_rounddiv_away, d);
    else if (0 == strcmp(name, "math_rounddiv"))
        value = timeit(v, math_rounddiv, d);
    else if (0 == strcmp(name, "rounddiv"))
        value = timeit(v, rounddiv, d);
    else if (0 == strcmp(name, "rounddiv_away"))
        value = timeit(v, rounddiv_away, d);
    else if (0 == strcmp(name, "fpdiv"))
        value = timeit(v, plaindivfp, d);
    else if (0 == strcmp(name, "rtfp"))
        value = timeit(v, rtfp, d);
    else if (0 == strcmp(name, "rupfp"))
        value = timeit(v, rupfp, d);
    else
        assert(false);
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << name << ": " << value << " : " <<
        1e-6 * std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() << " seconds\n";
    return value;
}

// Test all functions with a vector of values
template<typename T> void timetest(std::vector<T>& v, std::vector<T> &d) {
    // Call each function and time it
    ttest(v, d, "div");
    ttest(v, d, "math_rounddiv_away");
    ttest(v, d, "math_rounddiv");
    ttest(v, d, "rounddiv");
    ttest(v, d, "rounddiv_away");
    ttest(v, d, "fpdiv");
    ttest(v, d, "rtfp");
    ttest(v, d, "rupfp");
}

// Test all functions with a vector of values, with a constant divisor
template<typename T> void timetest(std::vector<T>& v, T d) {
    // Call each function and time it
    ttest(v, d, "div");
    ttest(v, d, "math_rounddiv_away");
    ttest(v, d, "math_rounddiv");
    ttest(v, d, "rounddiv");
    ttest(v, d, "rounddiv_away");
    ttest(v, d, "fpdiv");
    ttest(v, d, "rtfp");
    ttest(v, d, "rupfp");
}

// Test all functions with a single divider value
template<typename T> void test(T v, T d) {
    std::cout << "v: " << v << "\n";
    std::cout << "d: " << d << "\n\n";
    std::cout << "div: " << plaindiv(v, d) << "\n";
    std::cout << "math_rounddiv_away: " << math_rounddiv_away(v, d) << "\n";
    std::cout << "math_rounddiv: " << math_rounddiv(v, d) << "\n";
    std::cout << "rounddiv: " << rounddiv(v, d) << "\n";
    std::cout << "rounddiv_away: " << rounddiv_away(v, d) << "\n";
    std::cout << "rtfp: " << rtfp(v, d) << "\n";
    std::cout << "rupfp: " << rupfp(v, d) << "\n\n";
}

template<typename T> void runtest(size_t SZ) {
    // Generate a large array of random numbers covering the entire range of int
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    std::vector<T> v(SZ);
    T d = -1;
    if (8 == sizeof(T)) {
        // Double it up for 64 bit
        for (auto& i : v)
            i = dis(gen);
        for (auto & i : v)
            i = T((int64_t(i) << 32) | dis(gen));
        while (d <= 0)
            d = T((int64_t(d) << 32) | dis(gen));
    }
    else {
        for (auto& i : v)
            i = dis(gen);
        while (d <= 0)
            d = dis(gen);
    }

    std::cout << "\n" << sizeof(T) * 8 << "bit d: " << d << "\n\n";
    timetest(v, d);
}

// Divide by a compile time constant
template<typename T, T N> void runtest(size_t SZ) {
    // Generate a large array of random numbers covering the entire range of int
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    std::vector<T> v(SZ);
    if (8 == sizeof(T)) {
        // Double it up for 64 bit
        for (auto& i : v)
            i = dis(gen);
        for (auto& i : v)
            i = T((int64_t(i) << 32) | dis(gen));
    }
    else {
        for (auto& i : v)
            i = dis(gen);
    }

    std::cout << "\n" << sizeof(T) * 8 << "bit d: " << N << "\n\n";
    timetest(v, N);
}

template<typename T> void runtestvar(size_t SZ) {
    // Generate a large array of random numbers covering the entire range of int
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    std::vector<T> v(SZ);
    std::vector<T> d(SZ, -1);
    if (8 == sizeof(T)) {
        // Double it up for 64 bit
        for (auto& i : v)
            i = T((int64_t(dis(gen)) << 32) | dis(gen));
        // same for the divisors, they are strictly positive
        for (auto& i : d)
            while (i <= 0)
                i = T((int64_t(dis(gen)) << 32) | dis(gen));
    }
    else {
        for (auto& i : v)
            i = dis(gen);
        for (auto& i : d)
            while (i <= 0)
                i = dis(gen);
    }

    std::cout << "\n" << sizeof(T) * 8 << "bit d: var\n\n";
    timetest(v, d);
}

int main_const(size_t SZ)
{
    runtest<int64_t>(SZ);
    runtest<int32_t>(SZ);
    runtest<int16_t>(SZ);
    runtest<int8_t>(SZ);
    return 0;
}

template<int N>
int main_constc(size_t SZ)
{
    runtest<int64_t, int64_t(N)>(int64_t(SZ));
    runtest<int32_t, int32_t(N)>(SZ);
    runtest<int16_t, int16_t(N)>(SZ);
    runtest<int8_t, int8_t(N)>(SZ);
    return 0;
}

int main_var(const size_t SZ)
{
    runtestvar<int64_t>(SZ);
    std::cout << "Unsigned\n";
    runtestvar<uint64_t>(SZ);
    runtestvar<int32_t>(SZ);
    std::cout << "Unsigned\n";
    runtestvar<uint32_t>(SZ);
    runtestvar<int16_t>(SZ);
    std::cout << "Unsigned\n";
    runtestvar<uint16_t>(SZ);
    runtestvar<int8_t>(SZ);
    std::cout << "Unsigned\n";
    runtestvar<uint8_t>(SZ);
    return 0;
}

int main()
{
#if defined(_MSC_VER) && defined(_WIN32)
    // Set the process priority to high
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    // Pin to core 0, either thread
    DWORD tid = GetCurrentThreadId();
    HANDLE thread = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);
    if (thread) {
        DWORD_PTR mask = 3;
        SetThreadAffinityMask(thread, mask);
        CloseHandle(thread);
    }
#endif

    return main_constc<10>(SZ);
    //return main_const(SZ);
    //return main_var(SZ);
    //test(5, 2);
}