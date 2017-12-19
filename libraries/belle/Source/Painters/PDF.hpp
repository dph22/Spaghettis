
/* 
    Copyright 2007-2013 William Andrew Burnson. All rights reserved.

    < http://opensource.org/licenses/BSD-2-Clause >

    File modified by Nicolas Danet.
    
*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// ====================================

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Basic implementation of the PDF standard. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www.adobe.com/content/dam/Adobe/en/devnet/acrobat/pdfs/pdf_reference_1-7.pdf > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef BELLE_PAINTERS_PDF_HPP
#define BELLE_PAINTERS_PDF_HPP

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace belle { 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class Pdf : public Pageable {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class Object {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

public:
    Object (int label) : label_ (label), offset_ (0) 
    {
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PRIM_CPP11

public:
    Object (const Object&) = delete;
    Object& operator = (const Object&) = delete;
    
#else

private:
    Object (const Object&);
    Object& operator = (const Object&);

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    void setOffset (int offset)
    {
        offset_ = offset;
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    String getOffset() const
    {
        String s = String::paddedLeft (offset_, 10, '0'); s << " 00000 n ";
        return s;
    }
    
    int getLabel() const
    {
        return label_;
    }
    
    const String& getContent() const
    {
        return content_;
    }

    const String& getDictionary() const
    {
        return dictionary_;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    String asReference() const
    {
        String s; s << label_ << " 0 R";
        return s;
    } 
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    void addToContent (const String& s)
    {
        content_ << s << newLine;
    }
    
    void addToDictionary (const String& s)
    {
        dictionary_ << s << newLine;
    }

private:
    int label_;
    int offset_;
    String content_;
    String dictionary_;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Pdf (const String& filename) : filename_ (filename), size_ (Paper::portrait (Paper::A4))
    {
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PRIM_CPP11

public:
    Pdf (const Pdf&) = delete;
    Pdf& operator = (const Pdf&) = delete;
    
#else

private:
    Pdf (const Pdf&);
    Pdf& operator = (const Pdf&);

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Pdf& setUniqueID (const String& s)
    {
        unique_ = s; return *this;
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    bool originIsTopLeft() const override
    {
        return false;
    }
    
    Points getPageSize() override
    {
        return size_;
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    void setState (const Raster& state) override
    {
        state_ = state;
        
        if (stream_) { 
            stream_->addToContent (state_.asPDFString()); 
        }
    }
    
    void pushAffine (const Affine& affine) override
    {
        String s;
        
        s << Tab << "q" << newLine << affine.asPDFString();
        
        if (stream_) { 
            stream_->addToContent (s); 
        }
    }
    
    void popAffine (int n) override
    {
        String s;
        
        for (int i = 0; i < n; ++i) { s << Tab << "Q" << newLine; }
        
        if (stream_) { 
            stream_->addToContent (s); 
        }
    }
    
    void draw (const Path& path) override
    {
        String s;

        s << path.asPDFString();
        
        bool fill   = (state_.getFillColor().getAlpha() > 0.0);
        bool stroke = (state_.getStrokeColor().getAlpha() > 0.0) && (state_.getWidth() > 0.0);
        
        if (fill && stroke) { s << Tab << "B" << newLine; }
        else if (fill)      { s << Tab << "f" << newLine; }
        else if (stroke)    { s << Tab << "S" << newLine; }
        else { 
            s << Tab << "n" << newLine; 
        }
      
        if (stream_) { stream_->addToContent (s); }
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    void setActivePage (int n) override
    {
        if (n < 0 || n >= contents_.size()) { stream_ = nullptr; }
        else {
        //
        stream_ = contents_[n];
            
        String s;
            
        s << newLine;
        s << Tab << "/DeviceRGB cs" << newLine;
        s << Tab << "/DeviceRGB CS" << newLine;
            
        stream_->addToContent (s);
        //
        }
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    void performPaint (Paintable& toPaint) override
    {
        PRIM_ASSERT (Paper::isSmallerOrEqualThan (toPaint.getRequiredSize (*this), size_));
        
        writeBegin (toPaint);
        writePaint (toPaint);
        writeClose (toPaint);
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    void writeBegin (Paintable& toPaint)
    {
        const int catalog = 0;
        const int info = 1;
        const int tree = 2;
        
        int count = 1;
        int n = Math::max (toPaint.getNumberOfPages (*this), 1);
        
        headers_.add (Pointer < Object > (new Object (count++)));
        headers_.add (Pointer < Object > (new Object (count++)));
        headers_.add (Pointer < Object > (new Object (count++)));
        
        for (int i = 0; i < n; ++i) { pages_.add (Pointer < Object > (new Object (count++))); }
        for (int i = 0; i < n; ++i) { contents_.add (Pointer < Object > (new Object (count++))); }
            
        headers_[catalog]->addToDictionary ("/Type /Catalog");
        headers_[catalog]->addToDictionary (Tab << "/Pages " << headers_[tree].get()->asReference());
        
        headers_[info]->addToDictionary ("/Producer (Belle, Bonne, Sage)");
        
        headers_[tree]->addToDictionary ("/Type /Pages");
        headers_[tree]->addToDictionary (Tab << "/Kids [ " << pages_[0].get()->asReference());
        for (int i = 1; i < n; ++i) { 
        headers_[tree]->addToDictionary (Tab << "        " << pages_[i].get()->asReference());
        }
        headers_[tree]->addToDictionary (Tab << "      ]");
        headers_[tree]->addToDictionary (Tab << "/Count " << pages_.size());
        
        for (int i = 0; i < n; ++i) {
        //
        pages_[i]->addToDictionary ("/Type /Page");
        pages_[i]->addToDictionary (Tab << "/Parent " << headers_[tree].get()->asReference());
        pages_[i]->addToDictionary (Tab << "/Contents " << contents_[i].get()->asReference());
        pages_[i]->addToDictionary (Tab << "/MediaBox [ 0 0 " << size_.getX() << " " << size_.getY() << " ]");
        pages_[i]->addToDictionary (Tab << "/Resources " << "<<  >>");
        //
        }
    }
    
    void writePaint (Paintable& toPaint)
    {
        setActivePage (0);
        toPaint.paint (*this);
        stream_ = nullptr;
    }
    
    void writeClose (Paintable&)
    {
        String output;
        
        output << "%PDF-1.3" << newLine;
        output << "%" << "\xc2\xa5\xc2\xb1\xc3\xab" << newLine;
        output << newLine;
        
        int size = headers_.size() + pages_.size() + contents_.size() + 1;
        
        for (int i = 0; i < headers_.size(); ++i)  { writeObject (headers_[i], output);  }
        for (int i = 0; i < pages_.size(); ++i)    { writeObject (pages_[i], output);    }
        for (int i = 0; i < contents_.size(); ++i) { writeObject (contents_[i], output); }
    
        int XRefLocation = output.length();
        
        output << "xref" << newLine;
        output << "0 " << size << newLine;
        output << "0000000000 65535 f " << newLine;

        for (int i = 0; i < headers_.size(); ++i)  { output << headers_[i]->getOffset() << newLine;  }
        for (int i = 0; i < pages_.size(); ++i)    { output << pages_[i]->getOffset() << newLine;    }
        for (int i = 0; i < contents_.size(); ++i) { output << contents_[i]->getOffset() << newLine; }
      
        Random rand (filename_);
        String unique = (unique_.length() == 0) ? rand.nextID() : unique_;
        
        output << newLine;
        output << "trailer" << newLine;
        output << " << " << "/Root " << headers_[0]->asReference() << newLine;
        output << Tab << "/Info " << headers_[1]->asReference() << newLine;
        output << Tab << "/Size " << size << newLine;
        output << Tab << "/ID [ " << "<" << unique << ">" << newLine;
        output << Tab << "      " << "<" << unique << ">" << newLine;
        output << Tab << "    ]" << newLine;
        output << " >>" << newLine;
        output << "startxref" << newLine;
        output << XRefLocation << newLine;
        output << "%%EOF" << newLine;
        
        headers_.clear();
        pages_.clear();
        contents_.clear();
        
        if (filename_.length()) { File::writeFromString (filename_.toCString(), output); }
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    static void writeObject (Pointer < Object > object, String& output)
    {
        object->setOffset (output.length());

        output << object->getLabel() << " 0 obj" << newLine;
        output << " << " << object->getDictionary();
        
        if (object->getContent().length() == 0) { output << " >>" << newLine; }
        else {
            if (object->getDictionary().length()) { output << Tab; } 
            output << "/Length " << object->getContent().length() << newLine;
            output << " >>" << newLine;
            output << "stream" << newLine;
            output << object->getContent() << newLine;
            output << "endstream" << newLine;
        }
        
        output << "endobj" << newLine;
        output << newLine;
    }
    
private:
    String filename_;
    String unique_;
    Points size_;
    Raster state_;
    Pointer < Object > stream_;
    Array < Pointer < Object > > headers_;
    Array < Pointer < Object > > pages_;
    Array < Pointer < Object > > contents_;

private:
    PRIM_LEAK_DETECTOR (Pdf)
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace belle

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // BELLE_PAINTERS_PDF_HPP
