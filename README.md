# rounding_integer_division
Rounding Integer Division

## TL;DR
In C/C++ integer division truncates towards zero. This is not always the desired behavior.
These two functions are the best ways I have found to round integer division in C++, they work for all integer types and do not overflow.
The __limitation__ is that the divisor must be stricltly positive (and not zero). `rounddiv` rounds the middle point towards zero (down for positive n) while `rounddiv_away` rounds away from zero (up for positive n).
To use in C, replace `T` with any integer type and `template` with `inline`.

```C++
// Round to Zero Division, no overflow
template<typename T>
T rounddiv(T n, T d) {
    T m = n % d;
    T h = d / 2;
    return n / d + (!(n < 0) & (m > h)) - ((n < 0) & ((m + h) < 0));
}

// Round from Zero Division, no overflow
template<typename T>
T rounddiv_away(T n, T d) {
    T m = n % d;
    T h = d / 2 + d % 2;
    return n / d + (!(n < 0) & (m >= h)) - ((n < 0) & ((m + h) <= 0));
}
```

If a negative divisor is required, a conditional operator can be used, but this will overflow when `d` is the most negative value of the type.
```C++
T result = (d < 0) ? -rounddiv(n, -d) : rounddiv(n, d);
```

## Explanation
In C/C++, integer division truncates towards zero. It is suprisingly difficult to do rounding integer division correctly, especially when
performance is a concern. The most common solution is to add half the divisor to the numerator before dividing, but this can overflow, 
producing incorect results. Traditionally, the solution is to use floating point division, which normally converts everything to double 
floating point precision, making it correct for integers up to about 50 bits. However, it will overflow and be inaccurate for large 
64 bit integers, catching many programmers by surprise and being very hard to debug. The floating point solution is also rounding the middle 
point towards plus infinity, which is not consistent with the integer division operator for negative numbers.

The solution presented above solves the problem by comparing the remainder to half of the divisor and adjusting the result accordingly. It is 
accurate for the full range of the integer type. It is also branchless, which is important for performance. In general, these functions 
are only 20 to 30% slower than the normal integer division operator, since the additional operations are very simple for the CPU to perform. 
The reminder is a byproduct of the division, so it is essentially free. The operations on the divisor are also very simple.
The conversions between boolean and integers are much faster than an unpredictable branch, even if more instructions are to be executed. 
For unsigned integer types, some the conditions are known at compile time and will be optimized away by the compiler.  

A cleaner implementation is possible, but it is slower since it requires a second integer division, taking about twice as long as the normal 
truncating integer division. The operating principle, the results and the limitations are exactly the same as the two functions above. 
There is no need to explicitly check the sign, since the reminder has the same sign as the result.

```C++
// Round to nearest integer division, mid toward zero
template<typename T> 
T math_rounddiv(T n, T d) {
    return n / d + (n % d) / (d / 2 + 1);
}

// Round to nearest integer division, mid away from zero
template<typename T> 
T math_rounddiv_away(T n, T d) {
    return n / d + (n % d) / (d / 2 + d % 2);
}
```

## Performance Considerations

The _rid.cpp_ test program tests the performance and the accuracy of various integer divison functions.  The performance 
of calling these functions repeatedly in a loop is not representative of the performance when used sparignly in a larger program, 
or even when the inner loop contains more complex code. For x86 (and x64) CPUs, floating point division can be done in the vector unit, in parallel,
which is not the case for integer division. This means that the performance of floating point division is much better than integer division
when used in a tight loop, but that is not the case when used in a larger program. Conversions from/to floating point are slow, it's only by 
overlapping them with other operations and doing them in parallel that makes the floating point method seems fast. Stick with the integer only 
functions shown above when in doubt.  
It is also important to note that the performance of integer division is very dependent on the CPU. Intel CPUs for example sped up integer 
division considerably in Coffee Lake, it is considerably slower on older CPUs. AMD Zen integer division is fast.

Another important consideration is that the speed of integer division is dependent on the divisor, larger divisors are usually slower. 
Dividing by a value known at compile time can be much faster because the compiler may replace the division by multiplications, shifts and 
logical operations. If certain values of `d` are known to be common, they should be separated to let the compiler optimize for them, 
especially when used in a tight loop.  

For example:
    
```C++
template<typename T>
void vectordiv(vector<T> &v, T d) {
    for (auto &i : v)
        i = rounddiv(i, d);
}
```

If some values for `d` are known to common, it is better to write something like this:

```C++
template<typename T>
void vectordiv(vector<T> &v, T d) {
    if (d == 2) {
        for (T &i : v)
            i = rounddiv(i, T(2));
    } else if (d == 3) {
        for (T &i : v)
            i = rounddiv(i, T(3));
    } else if (d == 4) {
        for (T &i : v)
            i = rounddiv(i, T(4));
    } else if (d == 10) {
        for (T &i : v)
            i = rounddiv(i, T(10));
    } else if (d == 120) {
        for (T &i : v)
            i = rounddiv(i, T(120));
    } else {
        for (T &i : v)
            i = rounddiv(i, d);
    }
}
```

Note that the test is done outside of the loop to minimize the overhead. The compiler will not only eliminate the division 
for those specific values chosen but it can often vectorize the resulting operations, making the function singificantly faster! 
It may look silly to have the similar code for each case, but the performance gain is worth it, and it is better to let the 
compiler figure out the shortcuts. This trick is even more important when the known divisor is negative, since the
extra conditional operator and associated branch is not needed. For the known divisor cases the math version may be faster thant 
the initial one, since the code is simpler for the compiler to optimize.
