
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2019 Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://williamandrewburnson.com/media/Notation%20as%20Composition.pdf > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace belle { 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class Music : private MusicBase, public Paintable {

friend struct Adjust;
friend struct Beaming;
friend struct Density;
friend struct Direction;
friend struct Engravers;
friend struct Hull;
friend struct Select;
friend struct Stems;
friend struct Typesetters;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

public:
    typedef MusicBase::Chord Chord;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Music() : dirty_ (true)
    {
        setScale (kDefaultScale).setOrigin (Inches (kDefaultOriginX, kDefaultOriginY));
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PRIM_CPP11

public:
    Music (const Music&) = delete;
    Music& operator = (const Music&) = delete;
    
#else

private:
    Music (const Music&);
    Music& operator = (const Music&);

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Music& set (MusicBase& base)
    {
        with (base); dirty_ = true; return *this;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Music& setScale (double d)      /* Number of typographic points for the height of a note head. */
    {
        scale_ = Points (d, d); dirty_ = true; return *this;
    }
    
    Music& setOrigin (Points o)     /* Position of the middle staff line of the bottom part from the top. */
    {
        origin_ = o; dirty_ = true; return *this;
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    bool start (Painter& painter) override;     /* Typeset. */
    void paint (Painter& painter) override;     /* Draw. */
    void end   (Painter& painter) override;     /* Release. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    int getNumberOfPages   (Painter& painter) override;
    Points getRequiredSize (Painter& painter) override;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    void paintStart  (Painter& painter);
    void paintLines  (Painter& painter);
    void paintBrace  (Painter& painter);
    void paintAll    (Painter& painter);
    void paintEnd    (Painter& painter);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    void spacingVertical();
    void spacingHorizontal();
    bool spacingEnlarge();
    void spacingBounds();

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    void clean();
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    void proceed (const Typesetter::Partwise& f);
    void proceed (const Typesetter::Instantwise& f);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    Array < Part > & getParts()
    {
        return parts_;
    }
    
    Array < Instant > & getInstants()
    {
        return instants_;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    State::Clef& getClefState()
    {
        return clef_;
    }
    
    State::Group& getGroupState()
    {
        return group_;
    }
    
    State::Chord& getChordState()
    {
        return chord_;
    }

private:
    Points scale_;
    Points origin_;
    Box bounds_;
    bool dirty_;

private:
    Array < Part > parts_;
    Array < Instant > instants_;

private:
    State::Clef clef_;
    State::Group group_;
    State::Chord chord_;

private:
    static const double kDefaultScale;
    static const double kDefaultOriginX;
    static const double kDefaultOriginY;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#ifdef BELLE_COMPILE_INLINE

const double Music::kDefaultScale   = 5.0;
const double Music::kDefaultOriginX = 0.5;
const double Music::kDefaultOriginY = 1.5;

#endif // BELLE_COMPILE_INLINE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace belle

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
