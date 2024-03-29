
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_macros_h_
#define __m_macros_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_SHORT_FILE       (strrchr (__FILE__, '/') ? strrchr (__FILE__, '/') + 1 : __FILE__)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void post_log (const char *, ...);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_WITH_DEBUG

    #define PD_BUG          PD_ASSERT (0)
    #define PD_ASSERT(x)    if (!(x)) { post_log ("*** Assert / %s / line %d", PD_SHORT_FILE, __LINE__); }

#else
    
    #define PD_BUG
    #define PD_ASSERT(x)

#endif // PD_WITH_DEBUG

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < https://locklessinc.com/articles/vectorize/ > */

#if PD_ASSUME_ALIGNED
    #define PD_RESTRICTED   t_sample* __restrict__
    #define PD_ALIGNED(x)   __builtin_assume_aligned((x), 16)
#else
    #define PD_RESTRICTED   t_sample*
    #define PD_ALIGNED(x)   (x)
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_LIKELY(x)        __builtin_expect ((x), 1)
#define PD_UNLIKELY(x)      __builtin_expect ((x), 0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_ATOMS_ALLOCA(x, n)  \
    (x) = (t_atom *)((n) < 64 ? alloca ((n) * sizeof (t_atom)) : PD_MEMORY_GET ((n) * sizeof (t_atom)))
        
#define PD_ATOMS_FREEA(x, n)   \
    if (n >= 64) { PD_MEMORY_FREE ((x)); }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_UNUSED(x)        (void)(x)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define PD_ABORT(x)         if (PD_UNLIKELY (x)) { abort(); }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Roughly every object that is not a scalar. */

#define cast_objectIfConnectable(x)         (class_isBox (pd_class (x)) ? (t_object *)(x) : NULL)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_SECONDS_TO_MILLISECONDS(n)       ((double)(n) * 1000.0)
#define PD_MILLISECONDS_TO_SECONDS(n)       ((double)(n) * 1e-3)
#define PD_SECONDS_TO_MICROSECONDS(n)       ((double)(n) * 1000000.0)
#define PD_MICROSECONDS_TO_SECONDS(n)       ((double)(n) * 1e-6)
#define PD_MILLISECONDS_TO_MICROSECONDS(n)  ((double)(n) * 1000.0)
#define PD_MICROSECONDS_TO_MILLISECONDS(n)  ((double)(n) * 1e-3)
#define PD_SECONDS_TO_NANOSECONDS(n)        ((double)(n) * 1000000000.0)
#define PD_NANOSECONDS_TO_SECONDS(n)        ((double)(n) * 1e-9)
#define PD_MILLISECONDS_TO_NANOSECONDS(n)   ((double)(n) * 1000000.0)
#define PD_NANOSECONDS_TO_MILLISECONDS(n)   ((double)(n) * 1e-6)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_MAX(a,b)                         ((a)>(b)?(a):(b))
#define PD_MIN(a,b)                         ((a)<(b)?(a):(b))

#define PD_ABS(a)                           ((a)<0?-(a):(a))
#define PD_CLAMP(u,a,b)                     ((u)<(a)?(a):(u)>(b)?(b):(u))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www.math-solutions.org/graphplotter.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_HALF_PI                          1.5707963267948966192313216916398
#define PD_PI                               3.1415926535897932384626433832795
#define PD_TWO_PI                           6.283185307179586476925286766559
#define PD_LOG_TWO                          0.69314718055994530941723212145818
#define PD_LOG_TEN                          2.3025850929940456840179914546844
#define PD_E                                2.7182818284590452353602874713527

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_seed time_makeRandomSeed (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < http://en.wikipedia.org/wiki/Linear_congruential_generator > */

#define PD_RAND48_INIT(s)                   ((s) = (t_rand48)time_makeRandomSeed() & 0xffffffffffffULL)
#define PD_RAND48_NEXT(s)                   ((s) = (((s) * 0x5deece66dULL + 0xbULL) & 0xffffffffffffULL))
#define PD_RAND48_UINT32(s)                 (PD_RAND48_NEXT (s) >> 16)
#define PD_RAND48_DOUBLE(s)                 (PD_RAND48_UINT32 (s) * (1.0 / 4294967296.0))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define pd_class(x)                         (*((t_pd *)(x)))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define cast_pd(x)                          ((t_pd *)(x))
#define cast_iem(x)                         ((t_iem *)(x))
#define cast_gobj(x)                        ((t_gobj *)(x))
#define cast_glist(x)                       ((t_glist *)(x))
#define cast_scalar(x)                      ((t_scalar *)(x))
#define cast_object(x)                      ((t_object *)(x))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define IS_NULL(atom)                       ((atom)->a_type == A_NULL)
#define IS_SEMICOLON(atom)                  ((atom)->a_type == A_SEMICOLON)
#define IS_COMMA(atom)                      ((atom)->a_type == A_COMMA)
#define IS_POINTER(atom)                    ((atom)->a_type == A_POINTER)
#define IS_FLOAT(atom)                      ((atom)->a_type == A_FLOAT)
#define IS_SYMBOL(atom)                     ((atom)->a_type == A_SYMBOL)
#define IS_DOLLAR(atom)                     ((atom)->a_type == A_DOLLAR)
#define IS_DOLLARSYMBOL(atom)               ((atom)->a_type == A_DOLLARSYMBOL)
#define IS_VOID(atom)                       ((atom)->a_type == A_VOID)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define SET_NULL(atom)                      ((atom)->a_type = A_NULL)
#define SET_SEMICOLON(atom)                 ((atom)->a_type = A_SEMICOLON, (atom)->a_w.a_index = 0)
#define SET_COMMA(atom)                     ((atom)->a_type = A_COMMA, (atom)->a_w.a_index = 0)
#define SET_POINTER(atom, gp)               ((atom)->a_type = A_POINTER, (atom)->a_w.a_gpointer = (gp))
#define SET_FLOAT(atom, f)                  ((atom)->a_type = A_FLOAT, (atom)->a_w.a_float = (f))
#define SET_SYMBOL(atom, s)                 ((atom)->a_type = A_SYMBOL, (atom)->a_w.a_symbol = (s))
#define SET_DOLLAR(atom, n)                 ((atom)->a_type = A_DOLLAR, (atom)->a_w.a_index = (n))
#define SET_DOLLARSYMBOL(atom, s)           ((atom)->a_type = A_DOLLARSYMBOL, (atom)->a_w.a_symbol = (s))
#define SET_VOID(atom, p)                   ((atom)->a_type = A_VOID, (atom)->a_w.a_void = (p))
#define SET_OBJECT(atom, p)                 SET_VOID (atom, (void *)(p))
#define SET_BUFFER(atom, p)                 SET_VOID (atom, (void *)(p))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define GET_POINTER(atom)                   ((atom)->a_w.a_gpointer)
#define GET_FLOAT(atom)                     ((atom)->a_w.a_float)
#define GET_SYMBOL(atom)                    ((atom)->a_w.a_symbol)
#define GET_DOLLAR(atom)                    ((atom)->a_w.a_index)
#define GET_VOID(atom)                      ((atom)->a_w.a_void)
#define GET_OBJECT(atom)                    (t_gobj *)GET_VOID (atom)
#define GET_BUFFER(atom)                    (t_buffer *)GET_VOID (atom)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define IS_SEMICOLON_OR_COMMA(atom)         ((IS_SEMICOLON(atom)) || (IS_COMMA(atom)))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define ADDRESS_FLOAT(atom)                 &((atom)->a_w.a_float)
#define ADDRESS_SYMBOL(atom)                &((atom)->a_w.a_symbol)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

double  atomic_float64ReadRelaxed   (t_float64Atomic *);
void    atomic_float64WriteRelaxed  (double, t_float64Atomic *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void w_setFloat (t_word *w, t_float f)
{
    atomic_float64WriteRelaxed (f, &w->w_float);
}

static inline t_float w_getFloat (t_word *w)
{
    return atomic_float64ReadRelaxed (&w->w_float);
}

static inline void w_setSymbol (t_word *w, t_symbol *s)
{
    w->w_symbol = s;
}

static inline t_symbol *w_getSymbol (t_word *w)
{
    return w->w_symbol;
}

static inline void w_setArray (t_word *w, t_array *a)
{
    w->w_array = a;
}

static inline t_array *w_getArray (t_word *w)
{
    return w->w_array;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define inlet_new2(x, type)                 inlet_new (cast_object ((x)), cast_pd ((x)), (type), sym__inlet2)
#define inlet_new3(x, type)                 inlet_new (cast_object ((x)), cast_pd ((x)), (type), sym__inlet3)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_INT_MAX                          0x7fffffff
#define PD_FLT_MAX                          DBL_MAX
#define PD_EPSILON                          1E-9

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_IS_POWER_2(v)                    (!((v) & ((v) - 1)))
#define PD_NEXT_POWER_2(v)                  sys_nextPowerOfTwo ((uint64_t)(v))
#define PD_TO_RADIANS(degrees)              ((PD_PI * (degrees)) / 180.0)
#define PD_IS_ALIGNED_16(p)                 (((unsigned long)(p) & 0xfUL) == 0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Notice that it returns zero with an argument of zero. */

static inline uint64_t sys_nextPowerOfTwo (uint64_t v)
{
    v--;
    v |= (v >> 1);
    v |= (v >> 2);
    v |= (v >> 4);
    v |= (v >> 8);
    v |= (v >> 16);
    v |= (v >> 32);
    v++;
    
    return v;
}

static inline int sys_ilog2 (uint32_t v)
{
    int k = -1; while (v) { v >>= 1; k++; } return k;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Assumed IEEE 754 floating-point format. */

typedef union {
    float       z_f;
    uint32_t    z_i;
    } t_rawcast32;

typedef union {
    double      z_d;
    uint32_t    z_i[2];
    uint64_t    z_u;
    } t_rawcast64;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < https://en.wikipedia.org/wiki/Endianness#Floating_point > */

#if PD_LITTLE_ENDIAN

    #define PD_RAWCAST64_MSB        1                                                              
    #define PD_RAWCAST64_LSB        0

#else
                                                                      
    #define PD_RAWCAST64_MSB        0
    #define PD_RAWCAST64_LSB        1

#endif // PD_LITTLE_ENDIAN

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* True if zero, denormal, infinite, or NaN. */

static inline int PD_FLOAT32_IS_INVALID_OR_ZERO (float f)
{
    t_rawcast32 z;
    z.z_f = f;
    z.z_i &= 0x7f800000;
    return ((z.z_i == 0) || (z.z_i == 0x7f800000));
}

static inline int PD_FLOAT64_IS_INVALID_OR_ZERO (double f)
{
    t_rawcast64 z;
    z.z_d = f;
    z.z_i[PD_RAWCAST64_MSB] &= 0x7ff00000;
    return ((z.z_i[PD_RAWCAST64_MSB] == 0) || (z.z_i[PD_RAWCAST64_MSB] == 0x7ff00000));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Workaround required with ffast-math flag. */

static inline int PD_FLOAT64_IS_INF (double f)
{
    t_rawcast64 z;
    z.z_d = f;
    return (z.z_u == 0x7ff0000000000000UL || z.z_u == 0xfff0000000000000UL);
}

static inline int PD_FLOAT64_IS_NAN (double f)
{
    t_rawcast64 z;
    z.z_d = f;
    z.z_i[PD_RAWCAST64_MSB] &= 0x7ff00000;
    return ((z.z_i[PD_RAWCAST64_MSB] == 0x7ff00000) && !PD_FLOAT64_IS_INF (f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* True if exponent falls out (-64, 64) range. */
/* Notice that it is also true for zero, denormal, infinite, or NaN. */

static inline int PD_FLOAT32_IS_BIG_OR_SMALL (float f)
{
    t_rawcast32 z;
    z.z_f = f;
    return ((z.z_i & 0x20000000) == ((z.z_i >> 1) & 0x20000000)); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int PD_FLOAT64_IS_INTEGER (double f)
{
    return (trunc (f) - f == 0.0);
}

static inline int PD_FLOAT32_IS_INTEGER (float f)
{
    return (truncf (f) - f == 0.0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_macros_h_
