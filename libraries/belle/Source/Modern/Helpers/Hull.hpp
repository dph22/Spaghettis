
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2019 Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace belle { 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct Hull {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Compute the convex hull of a group of stamps into another context. */

static Polygon getHull (Music& music, const Array < NodePtr > & tokens, const Affine& context)
{
    Path path;
        
    for (int i = 0; i < tokens.size(); ++i) {
        NodePtr island (music.getIsland (tokens[i]));
        Pointer < Stamp > stamp = island->getObject().getStamp();
        path.addPath (stamp->getPath (stamp->getContext()));
    }

    return Path (path, Affine::inverse (context)).getHull();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace belle

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
