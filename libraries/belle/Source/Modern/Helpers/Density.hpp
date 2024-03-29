
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2019 Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Density of musical events. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace belle { 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct Density {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

public:
    static bool needExtraSpace (Music& music, const Array < NodePtr > & events) 
    {
        return (getValue (music, events)) > 0;
    }

    static double getUnit (Music& music, const Array < NodePtr > & events)
    {   
        double k = House::kDensityUnit;
        
        int n = getValue (music, events);
        int i = 0;

        while (n > i) { k += House::kDensityExtra; i += House::kDensityStep; }
        
        return k;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

private:
    static int getValue (Music& music, const Array < NodePtr > & events)
    {
        /* One point per events superior to a threshold. */
        
        int n = events.size() - House::kDensityThreshold;
        
        for (int i = 0; i < events.size(); ++i) {
        //
        /* One point more per tetrad. */
        /* One point more per tie. */
        
        if (events[i]->getObject().getAttribute (mica::Kind) == mica::Chord) {
            if (events[i]->getObject().getAttribute (mica::Size).getNumerator() > 3) { n += 1; }
            Array < NodePtr > notes = music.getAllTokensNext (events[i]);
            for (int j = 0; j < notes.size(); ++j) {
                if (notes[j]->getObject().getAttribute (mica::Tie) != mica::Undefined) { n += 1; };
            }
        }
        //
        }
        
        return n;
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace belle

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
