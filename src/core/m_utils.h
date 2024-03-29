
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_utils_h_
#define __m_utils_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     utils_version                   (char *dest, size_t size);

void        utils_anythingToList            (t_pd *x, t_listmethod fn, t_symbol *s, int argc, t_atom *argv);

t_symbol    *utils_getUnusedBindName        (t_class *c, t_symbol *prefix);

int         utils_isNameAllowedForWindow    (t_symbol *s);

t_id        utils_unique                    (void);
t_error     utils_uniqueWithAtoms           (int argc, t_atom *argv, t_id *u);
void        utils_appendUnique              (t_buffer *b, t_id u);

int         utils_uInt64IsElement           (t_symbol *s);
t_error     utils_uInt64Serialize           (int argc, t_atom *argv, uint64_t *n);
t_error     utils_uInt64Deserialize         (int argc, t_atom *argv, uint64_t *n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_symbol    *utils_getFirstAtomOfObjectAsSymbol     (t_object *x);
t_symbol    *utils_getFirstAtomOfBufferAsSymbol     (t_buffer *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol    *symbol_withAtoms               (int argc, t_atom *argv);
t_symbol    *symbol_withAtom                (t_atom *a);

t_symbol    *symbol_nil                     (void);
t_symbol    *symbol_emptyAsNil              (t_symbol *s);
t_symbol    *symbol_emptyAsDash             (t_symbol *s);

int         symbol_isNil                    (t_symbol *s);
int         symbol_isNilOrDash              (t_symbol *s);
int         symbol_containsWhitespace       (t_symbol *s);

t_symbol    *symbol_dollarToHash            (t_symbol *s);
t_symbol    *symbol_hashToDollar            (t_symbol *s);
t_symbol    *symbol_replaceDoubleDollar     (t_symbol *s);
t_symbol    *symbol_decode                  (t_symbol *s);
t_symbol    *symbol_removeExtension         (t_symbol *s);
t_symbol    *symbol_appendExtensionPatch    (t_symbol *s);
t_symbol    *symbol_appendExtensionHelp     (t_symbol *s);
t_symbol    *symbol_appendExtensionTemplate (t_symbol *s);

t_symbol    *symbol_addPrefix               (t_symbol *s, t_symbol *prefix);
t_symbol    *symbol_addSuffix               (t_symbol *s, t_symbol *suffix);
t_symbol    *symbol_withCopySuffix          (t_symbol *s);

t_symbol    *symbol_makeBind                (t_symbol *s);
t_symbol    *symbol_makeBindIfNot           (t_symbol *s);
t_symbol    *symbol_makeTemplateIdentifier  (t_symbol *s);
t_symbol    *symbol_stripBind               (t_symbol *s);
t_symbol    *symbol_stripTemplateIdentifier (t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_color     color_withRGB                   (int argc, t_atom *argv);
t_color     color_withDigits                (int c);
t_color     color_withEncoded               (t_symbol *s);

t_symbol    *color_toEncoded                (t_color color);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         math_areEquivalent              (t_float a, t_float b, t_float f = PD_EPSILON);
int         math_compareFloat               (t_float a, t_float b);
t_float     math_euclideanDistance          (t_float a, t_float b, t_float c, t_float d);

t_float     math_midiToFrequency            (t_float f);
t_float     math_frequencyToMidi            (t_float f);
t_float     math_rootMeanSquareToDecibel    (t_float f);
t_float     math_decibelToRootMeanSquare    (t_float f);
t_float     math_powerToDecibel             (t_float f);
t_float     math_decibelToPower             (t_float f);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     string_copy                                 (char *dest, size_t size, const char *src);
t_error     string_add                                  (char *dest, size_t size, const char *src);
t_error     string_append                               (char *dest, size_t size, const char *src, int n);
t_error     string_sprintf                              (char *dest, size_t size, const char *format, ...);
t_error     string_addSprintf                           (char *dest, size_t size, const char *format, ...);
t_error     string_addAtom                              (char *dest, size_t size, t_atom *a);
void        string_clear                                (char *dest, size_t size);

t_error     string_escapeOccurrence                     (char *dest, size_t size, const char *chars);

uint64_t    string_hash                                 (const char *s);

int         string_startWith                            (const char *s, const char *isStart);
int         string_endWith                              (const char *s, const char *isEnd);
int         string_contains                             (const char *s, const char *isContained);
int         string_containsOccurrenceAtStart            (const char *s, const char *chars);
int         string_containsOccurrence                   (const char *s, const char *chars);

int         string_indexOfFirstOccurrenceUntil          (const char *s, const char *chars, int n);
int         string_indexOfFirstOccurrenceFrom           (const char *s, const char *chars, int n);
int         string_indexOfFirstOccurrenceFromEnd        (const char *s, const char *chars);
void        string_replaceCharacter                     (char *s, char toBeReplaced, char c);
void        string_removeCharacter                      (char *s, char toBeRemoved);
void        string_removeCharacterAtEnd                 (char *s, char toBeRemoved);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* True if the string start with a dollar following by zero or more numbers. */

static inline int string_containsOneDollarFollowingByNumbers (const char *s)
{
    if (*s != '$') { return 0; } while (*(++s)) { if (*s < '0' || *s > '9') { return 0; } }
    
    return 1;
}

/* True if the string start with a dollar following by a number. */

static inline int string_startWithOneDollarAndOneNumber (const char *s)
{
    PD_ASSERT (s[0] != 0);
    
    if (s[0] != '$' || s[1] < '0' || s[1] > '9') { return 0; }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

// Default colors for IEM objects.

#define COLOR_IEM_BACKGROUND        0xffffff    // White.
#define COLOR_IEM_FOREGROUND        0x000000    // Black.
#define COLOR_IEM_LABEL             0x000000    // Black.
#define COLOR_IEM_PANEL             0xcccccc    // Grey.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define COLOR_SCALAR_WRONG          0xdddddd    // Grey.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

// Hard coded colors for the applications.

#define COLOR_BACKGROUND            0xffffff    // White.
#define COLOR_OPENED                0xdddddd    // Grey.
#define COLOR_NORMAL                0x000000    // Black.
#define COLOR_SELECTED              0x0000ff    // Blue.
#define COLOR_GRAPH                 0xff8080    // Red.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define COLOR_MASK                  0xffffff

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_color color_checked (t_color color)
{
    return (color & COLOR_MASK);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int char_isEnd (char c)
{
    return (c == ',' || c == ';');
}

static inline int char_isEscape (char c)
{
    return (c == '\\');
}

static inline int char_isWhitespace (char c)
{
    return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

static inline int char_isNumeric (char c)
{
    return ((c >= '0' && c <= '9') || (c == '.'));
}

static inline int char_isAlphabeticLowercase (char c)
{
    return (c >= 'a' && c <= 'z');
}

static inline int char_isAlphanumeric (char c)
{
    return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_'));
}

static inline int char_isAlphanumericLowercase (char c)
{
    return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c == '_'));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_utils_h_
