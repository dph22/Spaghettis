
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2019 Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* A path with a color to be painted. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace belle {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class Graphic {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Graphic() : color_ (Colors::black())
    {
    }
    
    explicit Graphic (const Path& path, Color color) : path_ (path), color_ (color)
    {
        addExtraSpaceIfPathIsEmpty();
    }
    
    #if PRIM_CPP11
    
    explicit Graphic (Path&& path, Color color) : path_ (std::move (path)), color_ (color)
    {
        addExtraSpaceIfPathIsEmpty();
    }
    
    #endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PRIM_CPP11

public:
    Graphic (const Graphic&) = delete;
    Graphic& operator = (const Graphic&) = delete;
    
#else

private:
    Graphic (const Graphic&);
    Graphic& operator = (const Graphic&);

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    const Path& getPath() const
    {
        return path_;
    }
    
    Box getBounds (const Affine& affine = Affine::identity()) const
    {
        return path_.getBounds (affine);
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    void paint (Painter& painter)
    {
        if (!path_.isEmpty()) {
        //
        painter.setState (Raster().setFillColor (color_));
        painter.draw (path_);
        //
        }
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    void addExtraSpaceIfPathIsEmpty()
    {
        /* Avoid empty path for the spacing algorithm. */
        
        if (path_.isEmpty()) {
        //
        Point pt (House::kSpacingEmpty, House::kSpacingEmpty);
        path_.getBox (mica::Extra) += Box (-pt, pt);
        //
        }
    }
    
private:
    Path path_;
    Color color_;

private:
    PRIM_LEAK_DETECTOR (Graphic)
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace belle

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
