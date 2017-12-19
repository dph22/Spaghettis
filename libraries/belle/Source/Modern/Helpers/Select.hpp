
/* 
    Copyright 2007-2013 William Andrew Burnson. All rights reserved.

    File modified by Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef BELLE_MODERN_HELPERS_SELECT_HPP
#define BELLE_MODERN_HELPERS_SELECT_HPP

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Determining selected status. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace belle { 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct Select {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static bool isSelected (const NodePtr& island)
{
    return (island->getObject().getAttribute (mica::Status) != mica::Undefined);
}

static bool isGroupSelected (Music& music)
{
    const Array < NodePtr > & tokens (music.getGroupState().getTokens());
    
    /* The group is considered selected if all elements are selected. */
    
    for (int i = 0; i < tokens.size(); ++i) { 
        if (!isSelected (music.getIsland (tokens[i]))) { return false; } 
    }
    
    return true;
}

static bool isPreviousSelected (Music& music, const NodePtr& island)
{
    return (music.getChordState().getAttribute (mica::Status) != mica::Undefined) && isSelected (island);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace belle

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // BELLE_MODERN_HELPERS_SELECT_HPP
