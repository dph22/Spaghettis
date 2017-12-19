
/* 
    Copyright 2007-2013 William Andrew Burnson. All rights reserved.

    File modified by Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef PRIM_MATH_HPP
#define PRIM_MATH_HPP

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace prim {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern const double kDeg;
extern const double kE;
extern const double kPi;
extern const double kTwoPi;
extern const double kHalfPi;
extern const double kLog2Inv;
extern const double kRoot2;
  
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace Math {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int32  abs      (int32 m);
int64  abs      (int64 m);
double abs      (double f);

int32  sign     (int32 m);
int64  sign     (int64 m);
double sign     (double f);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int roots (double a, double b, double& root);
int roots (double a, double b, double c, double& root1, double& root2);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel > */

inline int numberOfSetBits (uint32 v)
{
    v = v - ((v >> 1) & 0x55555555);
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
    return  static_cast < int > ((((v + (v >> 4)) & 0xf0f0f0f) * 0x1010101) >> 24);
}

template < class T > bool isPowerOfTwo (T v)
{
   return ((v & (v - 1)) == 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

template < class T > static T min (const T& a, const T& b)
{
    return (a < b ? a : b);
}

template < class T > static T max (const T& a, const T& b)
{
    return (a > b ? a : b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

template < class T > static void ascending (T& a, T& b)
{
    using std::swap;
    
    if (a > b) { swap (a, b); }
}

template < class T > static void descending (T& a, T& b)
{
    using std::swap;
    
    if (a < b) { swap (a, b); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

template < class T > static bool isBetween (const T& c, const T& a, const T& b)     /* Included. */
{
    return ((b >= a && c >= a && c <= b) || (a > b && c >= b && c <= a));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < http://en.wikipedia.org/wiki/Euclidean_algorithm#Implementations > */
/* < https://en.wikipedia.org/wiki/Least_common_multiple > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

template < class T > T GCD (T a, T b)
{
    if (a == 0 && b == 0) { return 0; }
    
    if (a == std::numeric_limits < T >::min() || b == std::numeric_limits < T >::min()) { return 0; }
    else {
        a = Math::abs (a);
        b = Math::abs (b);
    }
    
    if (a == 0) { return b; }
    else if (b == 0) { return a; }
    else {
    //
    T t;
    
    while (b != 0) {
        t = b;
        b = a % b;
        a = t;
    }
    
    return a;
    //
    }
}

template < class T > T LCM (T a, T b)
{
    if (a == 0 || b == 0) { return 0; }
    else {
        a = Math::abs (a);
        b = Math::abs (b);

        return (a / Math::GCD (a, b)) * b;
    }
}
   
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace Math

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#ifdef PRIM_COMPILE_INLINE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

const double kDeg     = 0.0174532925199432957692369076848861271344287188854;    /* From degrees to radians. */
const double kE       = 2.7182818284590452353602874713526624977572470937000;
const double kPi      = 3.1415926535897932384626433832795028841971693993751;
const double kTwoPi   = 6.2831853071795864769252867665590057683943387987502;
const double kHalfPi  = 1.5707963267948966192313216916397514420985846996876;
const double kLog2Inv = 1.4426950408889634073599246810018921374266459541530;
const double kRoot2   = 1.4142135623730950488016887242096980785696718753769;    /* Square root of 2. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int32 Math::abs (int32 m)
{
    return (m >= 0 ? m : -m);
}

int64 Math::abs (int64 m)
{
    return (m >= 0 ? m : -m);
}

double Math::abs (double f)
{
    return (f >= 0.0 ? f : -f);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int32 Math::sign (int32 m)
{
    if (m > 0) { return 1; } else if (m < 0) { return -1; } 
    else { 
        return 0; 
    }
}

int64 Math::sign (int64 m)
{
    if (m > 0) { return 1; } else if (m < 0) { return -1; }
    else {
        return 0;
    }
}

double Math::sign (double f)
{
    if (f > 0.0) { return 1.0; } else if (f < 0.0) { return -1.0; }
    else {
        return 0.0;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://en.wikipedia.org/wiki/Linear_equation > */
/* < http://en.wikipedia.org/wiki/Quadratic_equation > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int Math::roots (double a, double b, double& root)
{
    if (a != 0.0) { root = -b / a; return 1; }
    else {
        return 0;
    }
}

int Math::roots (double a, double b, double c, double& root1, double& root2)
{
    if (a == 0.0) { return roots (b, c, root1); }
    else { 
    //
    double discriminantSquared = b * b - 4 * a * c;
    
    if (discriminantSquared > 0.0) {
    //
    double discriminant = sqrt (discriminantSquared);
    
    if (discriminant > 0.0) {
    //
    double t = -0.5 * (b + sign (b) * discriminant);    /* < http://stackoverflow.com/a/900119 > */
    
    if (t != 0.0) {                 
        root1 = c / t;
        if (a != 0.0) { root2 = t / a; }
        else {
            return 1;
        }
        
    } else {
        root1 = (-b + discriminant) / (2.0 * a);
        root2 = (-b - discriminant) / (2.0 * a);
    }
    
    ascending (root1, root2);
    
    return 2;
    //
    } else if (discriminant == 0.0) {
    //
    root1 = 0.5 * b / a;
    root2 = 0.0;
    
    return 1;
    //
    } 
    //
    }

    root1 = root2 = 0.0; return 0;
    //
    }
}
 
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PRIM_COMPILE_INLINE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace prim

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // PRIM_MATH_HPP
