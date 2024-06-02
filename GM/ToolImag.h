// ToolImag.h
//
// Copyright (c) 1994-2024 By Dale L. Larson & William Su, All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#ifndef _TOOLIMAG_H
#define _TOOLIMAG_H

enum IToolType { itoolUnknown, itoolPencil, itoolSelect, itoolBrush,
    itoolFill, itoolText, itoolLine, itoolRect, itoolOval,
    itoolFillRect, itoolFillOval, itoolEyeDropper,
    itoolColorChange };

class CBitEditView;

class CImageTool
{
// Constructors
public:
    // Various derived tool types.
    CImageTool(IToolType eType);

// Attributes
public:
    const IToolType m_eToolType;

// Operations
public:
    static CImageTool& GetTool(IToolType eType);
    // ----------- //
    virtual void OnLButtonDown(CBitEditView& pView, int nMods, wxPoint point) /* override */;
    virtual void OnLButtonUp(CBitEditView& pView, int nMods, wxPoint point) /* override */;
    virtual void OnMouseMove(CBitEditView& pView, int nMods, wxPoint point) /* override */;
    virtual wxCursor OnSetCursor(CBitEditView& /*pView*/, wxPoint /*point*/) /* override */
        { return wxNullCursor; }

// Implementation
private:
    // -- Class variables -- //
    static std::vector<CImageTool*> c_toolLib;
protected:
    // Drag related vars....
    static wxPoint c_ptDown;         // Document coords.
    static wxPoint c_ptLast;
};

////////////////////////////////////////////////////////////////////////
// CBitSelectTool

class CBitSelectTool : public CImageTool
{
// Constructors
public:
    CBitSelectTool() : CImageTool(itoolSelect)
        { m_bMoveMode = FALSE; }

// Attributes
public:

// Operations
public:
    void OnLButtonDown(CBitEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonUp(CBitEditView& pView, int nMods, wxPoint point) override;
    void OnMouseMove(CBitEditView& pView, int nMods, wxPoint point) override;
    wxCursor OnSetCursor(CBitEditView& pView, wxPoint point) override;

// Implementation
public:
    BOOL    m_bMoveMode;
    wxRect  m_rctBound;
};

////////////////////////////////////////////////////////////////////////
// CBitPencilTool

class CBitPencilTool : public CImageTool
{
// Constructors
public:
    CBitPencilTool(IToolType eType) : CImageTool(eType) { }

// Attributes
public:

// Operations
public:
    void OnLButtonDown(CBitEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonUp(CBitEditView& pView, int nMods, wxPoint point) override;
    void OnMouseMove(CBitEditView& pView, int nMods, wxPoint point) override;
    wxCursor OnSetCursor(CBitEditView& pView, wxPoint point) override;

// Implementation
public:
};

////////////////////////////////////////////////////////////////////////
// CBitLineTool

class CBitLineTool : public CImageTool
{
// Constructors
public:
    CBitLineTool() : CImageTool(itoolLine) { }

// Attributes
public:

// Operations
public:
    void OnLButtonDown(CBitEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonUp(CBitEditView& pView, int nMods, wxPoint point) override;
    void OnMouseMove(CBitEditView& pView, int nMods, wxPoint point) override;
    wxCursor OnSetCursor(CBitEditView& pView, wxPoint point) override;

// Implementation
public:
};

////////////////////////////////////////////////////////////////////////
// CBitTextTool

class CBitTextTool : public CImageTool
{
// Constructors
public:
    CBitTextTool() : CImageTool(itoolText) { }

// Attributes
public:

// Operations
public:
    void OnLButtonDown(CBitEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonUp(CBitEditView& pView, int nMods, wxPoint point) override {}
    void OnMouseMove(CBitEditView& pView, int nMods, wxPoint point) override {}
    wxCursor OnSetCursor(CBitEditView& pView, wxPoint point) override;

// Implementation
public:
};

////////////////////////////////////////////////////////////////////////
// CBitColorChangeTool

class CBitColorChangeTool : public CImageTool
{
// Constructors
public:
    CBitColorChangeTool() : CImageTool(itoolColorChange) { }

// Attributes
public:

// Operations
public:
    void OnLButtonDown(CBitEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonUp(CBitEditView& pView, int nMods, wxPoint point) override {}
//  void OnMouseMove(CBitEditView& pView, int nMods, wxPoint point) override {}
    wxCursor OnSetCursor(CBitEditView& pView, wxPoint point) override;

// Implementation
public:
};

////////////////////////////////////////////////////////////////////////
// CBitFillTool

class CBitFillTool : public CImageTool
{
// Constructors
public:
    CBitFillTool() : CImageTool(itoolFill) { }

// Attributes
public:

// Operations
public:
    void OnLButtonDown(CBitEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonUp(CBitEditView& pView, int nMods, wxPoint point) override {}
//  void OnMouseMove(CBitEditView& pView, int nMods, wxPoint point) override {}
    wxCursor OnSetCursor(CBitEditView& pView, wxPoint point) override;

// Implementation
public:
};

////////////////////////////////////////////////////////////////////////
// CBitRectTool

class CBitRectTool : public CImageTool
{
// Constructors
public:
    CBitRectTool(IToolType eType) : CImageTool(eType) { }

// Attributes
public:

// Operations
public:
    void OnLButtonDown(CBitEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonUp(CBitEditView& pView, int nMods, wxPoint point) override;
    void OnMouseMove(CBitEditView& pView, int nMods, wxPoint point) override;
    wxCursor OnSetCursor(CBitEditView& pView, wxPoint point) override;

// Implementation
public:
};

////////////////////////////////////////////////////////////////////////
// CBitOvalTool

class CBitOvalTool : public CImageTool
{
// Constructors
public:
    CBitOvalTool(IToolType eType) : CImageTool(eType) { }

// Attributes
public:

// Operations
public:
    void OnLButtonDown(CBitEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonUp(CBitEditView& pView, int nMods, wxPoint point) override;
    void OnMouseMove(CBitEditView& pView, int nMods, wxPoint point) override;
    wxCursor OnSetCursor(CBitEditView& pView, wxPoint point) override;

// Implementation
public:
};

////////////////////////////////////////////////////////////////////////
// CBitDropperTool

class CBitDropperTool : public CImageTool
{
// Constructors
public:
    CBitDropperTool() : CImageTool(itoolEyeDropper) { }

// Attributes
public:

// Operations
public:
    void OnLButtonDown(CBitEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonUp(CBitEditView& pView, int nMods, wxPoint point) override;
//  void OnMouseMove(CBitEditView& pView, int nMods, wxPoint point) override {}
    wxCursor OnSetCursor(CBitEditView& pView, wxPoint point) override;

// Implementation
public:
};

#endif

