
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2019 Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://en.wikipedia.org/wiki/Key_signature > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace belle { 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace Shapes {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class KeySignature {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    KeySignature (mica::Concept keySignature, mica::Concept clef)
    {
        mica::Concept accidental = mica::map (keySignature, mica::Accidental);
        
        count_ = keySignature.length();
        clefAccidentals_ = mica::map (clef, accidental);
        
        if (accidental == mica::Flat) { glyph_ = 0x0046; }
        else {
            glyph_ = 0x0048;
        } 
    }

#if PRIM_CPP11

public:
    KeySignature (const KeySignature&) = default;
    KeySignature (KeySignature&&) = default;
    KeySignature& operator = (const KeySignature&) = default;
    KeySignature& operator = (KeySignature&&) = default;

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    void addToPath (Path& path) const
    {
        for (int i = 0; i < count_; ++i) {
        //
        int linespace = mica::item (clefAccidentals_, i).getNumerator();
        Affine affine = Affine::translation (Vector (i, linespace * House::kStaffSpaceWidth / 2.0));
        affine *= House::kAffineJoie;
        path.addPath (House::getInstance().getJoie().getGlyph (glyph_)->getPath(), affine);
        //
        }
    }

private:
    mica::Concept clefAccidentals_;
    int count_;
    unicode glyph_;

private:
    PRIM_LEAK_DETECTOR (KeySignature)
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace Shapes

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace belle

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
