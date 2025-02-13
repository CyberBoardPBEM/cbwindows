// FrmDockTile.h - container window for the tile palette.
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

#ifndef _FRMDOCKTILE_H
#define _FRMDOCKTILE_H

#pragma once

class CTilePalette;

/////////////////////////////////////////////////////////////////////////////
// CDockTilePalette window

class CDockTilePalette : public wxPanel
{
// Construction
public:
    CDockTilePalette();

// Attributes
private:
    CTilePalette*   m_pChildWnd;
// Operations
public:
    void SetChild(CTilePalette* pChildWnd);

// Implementation
public:
    virtual ~CDockTilePalette();
};

#endif
