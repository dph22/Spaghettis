
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __d_macros_h_
#define __d_macros_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define DSP_UNITBIT             1572864.0   // (1.5 * 2^20)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define DSP_UNITBIT_MSB         0x41380000 
#define DSP_UNITBIT_LSB         0x00000000 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/*
    In the style of R. Hoeldrich (ICMC 1995 Banff).
    
    The trick is based on the IEEE 754 floating-point format. 
    It uses a constant offset to get the integer and the fractional parts split over
    the fourth and fifth bytes.
    Using raw cast to 32-bit integer it is therefore possible to get or to set them
    independently and (that is the goal) efficiently.
    
    Thanks to let me know links to original paper.
    
*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Double precision floating-point representation (binary and hexadecimal). */

// DSP_UNITBIT
// DSP_UNITBIT + 0.5
// DSP_UNITBIT + 0.25
// DSP_UNITBIT + 0.125
// DSP_UNITBIT + 0.0625
// DSP_UNITBIT + 0.9375

// 0 10000010011 1000 00000000 00000000     00000000 00000000 00000000 00000000
// 0 10000010011 1000 00000000 00000000     10000000 00000000 00000000 00000000
// 0 10000010011 1000 00000000 00000000     01000000 00000000 00000000 00000000
// 0 10000010011 1000 00000000 00000000     00100000 00000000 00000000 00000000
// 0 10000010011 1000 00000000 00000000     00010000 00000000 00000000 00000000
// 0 10000010011 1000 00000000 00000000     11110000 00000000 00000000 00000000

// 0x41380000 00000000
// 0x41380000 80000000
// 0x41380000 40000000
// 0x41380000 20000000
// 0x41380000 10000000
// 0x41380000 f0000000

// DSP_UNITBIT + 1.0
// DSP_UNITBIT + 2.0
// DSP_UNITBIT + 4.0

// 0 10000010011 1000 00000000 00000001     00000000 00000000 00000000 00000000
// 0 10000010011 1000 00000000 00000010     00000000 00000000 00000000 00000000
// 0 10000010011 1000 00000000 00000100     00000000 00000000 00000000 00000000 

// 0x41380001 00000000
// 0x41380002 00000000
// 0x41380004 00000000

// DSP_UNITBIT - 0.5
// DSP_UNITBIT - 0.25
// DSP_UNITBIT - 0.125
// DSP_UNITBIT - 0.0625

// DSP_UNITBIT - 1.0
// DSP_UNITBIT - 2.0
// DSP_UNITBIT - 4.0

// 0 10000010011 0111 11111111 11111111     10000000 00000000 00000000 00000000
// 0 10000010011 0111 11111111 11111111     11000000 00000000 00000000 00000000
// 0 10000010011 0111 11111111 11111111     11100000 00000000 00000000 00000000
// 0 10000010011 0111 11111111 11111111     11110000 00000000 00000000 00000000

// 0 10000010011 0111 11111111 11111111     00000000 00000000 00000000 00000000
// 0 10000010011 0111 11111111 11111110     00000000 00000000 00000000 00000000
// 0 10000010011 0111 11111111 11111100     00000000 00000000 00000000 00000000

// 0x4137ffff 80000000
// 0x4137ffff c0000000  
// 0x4137ffff e0000000 
// 0x4137ffff f0000000

// 0x4137ffff 00000000
// 0x4137fffe 00000000
// 0x4137fffc 00000000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define COSINE_TABLE_SIZE       (1 << 9)    // 512

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define COSINE_UNITBIT          (DSP_UNITBIT * COSINE_TABLE_SIZE)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define COSINE_UNITBIT_MSB      0x41c80000 
#define COSINE_UNITBIT_LSB      0x00000000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// DSP_UNITBIT * 512.0
// DSP_UNITBIT * 512.0 + 256.0
// DSP_UNITBIT * 512.0 + 128.0
// ...

// 0 10000011100 1000 00000000 00000000     00000000 00000000 00000000 00000000
// 0 10000011100 1000 00000000 00000000     10000000 00000000 00000000 00000000
// 0 10000011100 1000 00000000 00000000     01000000 00000000 00000000 00000000

// 0x41c80000 00000000
// 0x41c80000 80000000
// 0x41c80000 40000000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Notice that the trick above is broken for index with a large value. */

/* < https://lists.puredata.info/pipermail/pd-dev/2016-11/020873.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_float *cos_tilde_table;

static inline t_float dsp_getCosineAtLUT (double index)
{
    t_float f1, f2, f;
    t_rawcast64 z;
    int i;
    
    // -- Note that index MUST be <= 2^19 (i.e. 1024 * COSINE_TABLE_SIZE).
    
    z.z_d = index + DSP_UNITBIT;
    
    i = (int)(z.z_i[PD_RAWCAST64_MSB] & (COSINE_TABLE_SIZE - 1));   /* Integer part. */
    
    z.z_i[PD_RAWCAST64_MSB] = DSP_UNITBIT_MSB;
    
    f = (t_float)(z.z_d - DSP_UNITBIT);  /* Fractional part. */
    
    /* Linear interpolation. */
    
    f1 = cos_tilde_table[i + 0];
    f2 = cos_tilde_table[i + 1];
    
    return (f1 + f * (f2 - f1));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline double dsp_clipForHoeldrichOverflow (double f)
{
    return PD_CLAMP (f, -1024.0, 1024.0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

// -- Wrap the phase to [0.0, 512.0[ range.

static double inline dsp_wrapCosine (double phase)
{
    t_rawcast64 z;
    z.z_d = phase + COSINE_UNITBIT;
    z.z_i[PD_RAWCAST64_MSB] = COSINE_UNITBIT_MSB;
    return z.z_d - COSINE_UNITBIT;
}

// -- Wrap the phase to [0.0, 1.0[ range.

static double inline dsp_wrapPhasor (double phase)
{
    t_rawcast64 z;
    z.z_d = phase + DSP_UNITBIT;
    z.z_i[PD_RAWCAST64_MSB] = DSP_UNITBIT_MSB;
    return z.z_d - DSP_UNITBIT;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define dsp_getSineAtLUT(index) dsp_getCosineAtLUT ((double)(index) - (COSINE_TABLE_SIZE / 4.0))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_float dsp_4PointsInterpolationWithFloats (t_float f, double a, double b, double c, double d)
{
    double t = c - b;
    
    return (t_float)(b + f * (t - 0.1666667 * (1.0 - f) * ((d - a - 3.0 * t) * f + (d + 2.0 * a - 3.0 * b))));
}

static inline t_float dsp_4PointsInterpolationWithWords (t_float f, t_word *data)
{
    double a = (double)w_getFloat (data + 0);
    double b = (double)w_getFloat (data + 1);
    double c = (double)w_getFloat (data + 2);
    double d = (double)w_getFloat (data + 3);
    
    return dsp_4PointsInterpolationWithFloats (f, a, b, c, d);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Assumed IEEE 754 floating-point format. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that a float can be factorized into two floats. */
/* For one keep the mantissa and set the exponent to zero (i.e. 0x7f with the bias). */
/* For the other keep the exponent and set the mantissa to zero. */
/* Thus the rsqrt is approximated by the product of two (with fast lookup) rsqrt. */

/* < https://en.wikipedia.org/wiki/Fast_inverse_square_root#Newton.27s_method > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define RSQRT_MANTISSA_SIZE         1024
#define RSQRT_EXPONENTIAL_SIZE      256

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WITH_DEADCODE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_sample rsqrt_tableMantissa[];
extern t_sample rsqrt_tableExponential[];

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_sample rsqrt_fastLUT (t_sample f)
{
    t_rawcast32 z;

    z.z_f = f;
    
    if (z.z_f <= 0.0) { return 0.0; }
    else {
    //
    int e = (z.z_i >> 23) & (RSQRT_EXPONENTIAL_SIZE - 1);
    int m = (z.z_i >> 13) & (RSQRT_MANTISSA_SIZE - 1);
    t_sample g = rsqrt_tableExponential[e] * rsqrt_tableMantissa[m];
    
    return (t_sample)(1.5 * g - 0.5 * g * g * g * z.z_f);
    //
    }
}

static inline t_sample sqrt_fastLUT (t_sample f)
{
    return f * rsqrt_fastLUT (f);
}

#endif // PD_WITH_DEADCODE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_sample rsqrt_fastSTD (t_sample f)
{
    return (f <= 0.0 ? 0.0 : ((t_sample)1.0 / sqrtf (f)));
}

static inline t_sample sqrt_fastSTD (t_sample f)
{
    return (f <= 0.0 ? 0.0 : sqrtf (f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define rsqrt_fast  rsqrt_fastSTD
#define sqrt_fast   sqrt_fastSTD

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_macros_h_
