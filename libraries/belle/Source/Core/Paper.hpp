
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2019 Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://en.wikipedia.org/wiki/ISO_216 > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace belle {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class Paper {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    static Inches landscape (Inches size)
    {
        return Inches (Math::max (size.getX(), size.getY()), Math::min (size.getX(), size.getY()));
    }
    
    static Inches portrait (Inches size)
    {
        return Inches (Math::min (size.getX(), size.getY()), Math::max (size.getX(), size.getY()));
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    static bool isSmallerThan (const Inches& a, const Inches& b)
    {
        return (a.getX() < b.getX()) && (a.getY() < b.getY());
    }
    
    static bool isGreaterThan (const Inches& a, const Inches& b)
    {
        return (a.getX() > b.getX()) && (a.getY() > b.getY());
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    static bool isSmallerOrEqualThan (const Inches& a, const Inches& b)
    {
        return (a.getX() <= b.getX()) && (a.getY() <= b.getY());
    }
    
    static bool isGreaterOrEqualThan (const Inches& a, const Inches& b)
    {
        return (a.getX() >= b.getX()) && (a.getY() >= b.getY());
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    static const Millimeters A2()   { return Millimeters (420.0, 594.0); }
    static const Millimeters A3()   { return Millimeters (297.0, 420.0); }
    static const Millimeters A4()   { return Millimeters (210.0, 297.0); }
    static const Millimeters A5()   { return Millimeters (148.0, 210.0); }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    static const Inches Legal()     { return Inches (8.5, 14.0); }
    static const Inches Letter()    { return Inches (8.5, 11.0); }
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace belle

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
