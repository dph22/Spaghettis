
/* 
    Copyright 2007-2013 William Andrew Burnson. All rights reserved.

    File modified by Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef BELLE_MODERN_FUNCTORS_ENGRAVERS_HPP
#define BELLE_MODERN_FUNCTORS_ENGRAVERS_HPP

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace belle { 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct Engravers {      /* Rely on implicit special member functions. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

class Dummy : public Engraver {

public:
    Pointer < Engraver > clone() const override
    {
        return Pointer < Engraver > (new Dummy (*this));
    }
    
public:
    void engrave (Music& music, NodePtr token, const Pointer < Stamp > & stamp) override;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

class Chord : public Engraver {

public:
    Pointer < Engraver > clone() const override
    {
        return Pointer < Engraver > (new Chord (*this));
    }
    
public:
    void engrave (Music& music, NodePtr token, const Pointer < Stamp > & stamp) override;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

class Flags : public Engraver {

public:
    Pointer < Engraver > clone() const override
    {
        return Pointer < Engraver > (new Flags (*this));
    }
    
public:
    void engrave (Music& music, NodePtr token, const Pointer < Stamp > & stamp) override;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

class Tie : public Engraver {

public:
    Pointer < Engraver > clone() const override
    {
        return Pointer < Engraver > (new Tie (*this));
    }
    
public:
    void engrave (Music& music, NodePtr token, const Pointer < Stamp > & stamp) override;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

class Clef : public Engraver {

public:
    Pointer < Engraver > clone() const override
    {
        return Pointer < Engraver > (new Clef (*this));
    }
    
public:
    void engrave (Music& music, NodePtr token, const Pointer < Stamp > & stamp) override;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

class KeySignature : public Engraver {

public:
    Pointer < Engraver > clone() const override
    {
        return Pointer < Engraver > (new KeySignature (*this));
    }
    
public:
    void engrave (Music& music, NodePtr token, const Pointer < Stamp > & stamp) override;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

class TimeSignature : public Engraver {

public:
    Pointer < Engraver > clone() const override
    {
        return Pointer < Engraver > (new TimeSignature (*this));
    }
    
public:
    void engrave (Music& music, NodePtr token, const Pointer < Stamp > & stamp) override;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

class Barline : public Engraver {

public:
    Pointer < Engraver > clone() const override
    {
        return Pointer < Engraver > (new Barline (*this));
    }
    
public:
    void engrave (Music& music, NodePtr token, const Pointer < Stamp > & stamp) override;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class Beam : public Engraver {

public:
    Pointer < Engraver > clone() const override
    {
        return Pointer < Engraver > (new Beam (*this));
    }
    
public:
    void engrave (Music& music, NodePtr token, const Pointer < Stamp > & stamp) override;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

class Tuplet : public Engraver {

public:
    Pointer < Engraver > clone() const override
    {
        return Pointer < Engraver > (new Tuplet (*this));
    }
    
public:
    void engrave (Music& music, NodePtr token, const Pointer < Stamp > & stamp) override;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace belle

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // BELLE_MODERN_FUNCTORS_ENGRAVERS_HPP
