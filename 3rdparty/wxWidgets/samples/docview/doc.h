/////////////////////////////////////////////////////////////////////////////
// Name:        samples/docview/doc.h
// Purpose:     Document classes
// Author:      Julian Smart
// Modified by: Vadim Zeitlin: merge with the MDI version and general cleanup
// Created:     04/01/98
// RCS-ID:      $Id: doc.h 62089 2009-09-24 16:04:06Z JS $
// Copyright:   (c) 1998 Julian Smart
//              (c) 2008 Vadim Zeitlin
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SAMPLES_DOCVIEW_DOC_H_
#define _WX_SAMPLES_DOCVIEW_DOC_H_

#include "wx/docview.h"
#include "wx/cmdproc.h"
#include "wx/vector.h"
#include "wx/image.h"

// This sample is written to build both with wxUSE_STD_IOSTREAM==0 and 1, which
// somewhat complicates its code but is necessary in order to support building
// it under all platforms and in all build configurations
//
// In your own code you would normally use std::stream classes only and so
// wouldn't need these typedefs
#if wxUSE_STD_IOSTREAM
    typedef wxSTD istream DocumentIstream;
    typedef wxSTD ostream DocumentOstream;
#else // !wxUSE_STD_IOSTREAM
    typedef wxInputStream DocumentIstream;
    typedef wxOutputStream DocumentOstream;
#endif // wxUSE_STD_IOSTREAM/!wxUSE_STD_IOSTREAM

// ----------------------------------------------------------------------------
// The document class and its helpers
// ----------------------------------------------------------------------------

// Represents a line from one point to the other
struct DoodleLine
{
    DoodleLine() { /* leave fields uninitialized */ }

    DoodleLine(const wxPoint& pt1, const wxPoint& pt2)
        : x1(pt1.x), y1(pt1.y), x2(pt2.x), y2(pt2.y)
    {
    }

    wxInt32 x1;
    wxInt32 y1;
    wxInt32 x2;
    wxInt32 y2;
};

typedef wxVector<DoodleLine> DoodleLines;

// Contains a list of lines: represents a mouse-down doodle
class DoodleSegment
{
public:
    DocumentOstream& SaveObject(DocumentOstream& stream);
    DocumentIstream& LoadObject(DocumentIstream& stream);

    bool IsEmpty() const { return m_lines.empty(); }
    void AddLine(const wxPoint& pt1, const wxPoint& pt2)
    {
        m_lines.push_back(DoodleLine(pt1, pt2));
    }
    const DoodleLines& GetLines() const { return m_lines; }

private:
    DoodleLines m_lines;
};

typedef wxVector<DoodleSegment> DoodleSegments;


// The drawing document (model) class itself
class DrawingDocument : public wxDocument
{
public:
    DrawingDocument() : wxDocument() { }

    DocumentOstream& SaveObject(DocumentOstream& stream);
    DocumentIstream& LoadObject(DocumentIstream& stream);

    // add a new segment to the document
    void AddDoodleSegment(const DoodleSegment& segment);

    // remove the last segment, if any, and copy it in the provided pointer if
    // not NULL and return true or return false and do nothing if there are no
    // segments
    bool PopLastSegment(DoodleSegment *segment);

    // get direct access to our segments (for DrawingView)
    const DoodleSegments& GetSegments() const { return m_doodleSegments; }

private:
    void DoUpdate();

    DoodleSegments m_doodleSegments;

    DECLARE_DYNAMIC_CLASS(DrawingDocument)
};


// ----------------------------------------------------------------------------
// Some operations (which can be done and undone by the view) on the document:
// ----------------------------------------------------------------------------

// Base class for all operations on DrawingDocument
class DrawingCommand : public wxCommand
{
public:
    DrawingCommand(DrawingDocument *doc,
                   const wxString& name,
                   const DoodleSegment& segment = DoodleSegment())
        : wxCommand(true, name),
          m_doc(doc),
          m_segment(segment)
    {
    }

protected:
    bool DoAdd() { m_doc->AddDoodleSegment(m_segment); return true; }
    bool DoRemove() { return m_doc->PopLastSegment(&m_segment); }

private:
    DrawingDocument * const m_doc;
    DoodleSegment m_segment;
};

// The command for adding a new segment
class DrawingAddSegmentCommand : public DrawingCommand
{
public:
    DrawingAddSegmentCommand(DrawingDocument *doc, const DoodleSegment& segment)
        : DrawingCommand(doc, "Add new segment", segment)
    {
    }

    virtual bool Do() { return DoAdd(); }
    virtual bool Undo() { return DoRemove(); }
};

// The command for removing the last segment
class DrawingRemoveSegmentCommand : public DrawingCommand
{
public:
    DrawingRemoveSegmentCommand(DrawingDocument *doc)
        : DrawingCommand(doc, "Remove last segment")
    {
    }

    virtual bool Do() { return DoRemove(); }
    virtual bool Undo() { return DoAdd(); }
};


// ----------------------------------------------------------------------------
// wxTextDocument: wxDocument and wxTextCtrl married
// ----------------------------------------------------------------------------

class wxTextDocument : public wxDocument
{
public:
    wxTextDocument() : wxDocument() { }

    virtual bool OnCreate(const wxString& path, long flags); 

    virtual wxTextCtrl* GetTextCtrl() const = 0;

    virtual bool IsModified() const;
    virtual void Modify(bool mod);

protected:
    virtual bool DoSaveDocument(const wxString& filename);
    virtual bool DoOpenDocument(const wxString& filename);

    void OnTextChange(wxCommandEvent& event);

    wxDECLARE_NO_COPY_CLASS(wxTextDocument);
    DECLARE_CLASS(wxTextDocument)
};

// ----------------------------------------------------------------------------
// A very simple text document class
// ----------------------------------------------------------------------------

class TextEditDocument : public wxTextDocument
{
public:
    TextEditDocument() : wxTextDocument() { }
    virtual wxTextCtrl* GetTextCtrl() const;

    wxDECLARE_NO_COPY_CLASS(TextEditDocument);
    DECLARE_DYNAMIC_CLASS(TextEditDocument)
};

// ----------------------------------------------------------------------------
// A basic image document class
// ----------------------------------------------------------------------------

class wxImageDocument : public wxDocument
{
protected:
    wxImage m_image;
public:
    wxImageDocument();

    wxImage*       GetImage()       { return &m_image; }
    const wxImage& GetImage() const { return m_image; }

    bool SaveFile(wxOutputStream*, wxBitmapType) const;

public:
    virtual ~wxImageDocument();
    virtual bool DeleteContents();

    virtual bool DoOpenDocument(const wxString& file);
    virtual bool DoSaveDocument(const wxString& file);

    virtual bool DoOpenDocument(wxInputStream*);
    virtual bool DoSaveDocument(wxOutputStream*) const;

    wxDECLARE_NO_COPY_CLASS(wxImageDocument);
    DECLARE_DYNAMIC_CLASS(wxImageDocument)
};

#endif // _WX_SAMPLES_DOCVIEW_DOC_H_