// Tile.h
//
// Copyright (c) 1994-2020 By Dale L. Larson, All Rights Reserved.
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

#include    "stdafx.h"
#include    "Tile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////
//                    CTile - Class

int CTile::GetWidth() const
{
    ASSERT(m_pTS != NULL);
    return m_pTS->GetWidth();
}

int CTile::GetHeight() const
{
    ASSERT(m_pTS != NULL);
    return m_pTS->GetHeight();
}

CSize CTile::GetSize() const
{
    return m_pTS != NULL ? m_pTS->GetSize() : m_size;
}

void CTile::BitBlt(CDC& pDC, int x, int y, DWORD dwRop) const
{
    if (m_pTS != NULL)
        m_pTS->TileBlt(pDC, x, y, m_yLoc, dwRop);
    else if (m_crTrans != m_crSmall)
    {
        // Only draw color patch if not the transparent color.
        CBrush oBrsh;
        oBrsh.CreateSolidBrush(m_crSmall);
        CBrush* pPrvBrsh = pDC.SelectObject(&oBrsh);
        pDC.PatBlt(x, y, m_size.cx, m_size.cy, PATCOPY);
        pDC.SelectObject(pPrvBrsh);
    }
}

void CTile::BitBlt(wxDC & pDC, wxCoord x, wxCoord y, wxRasterOperationMode dwRop/* = wxCOPY*/) const
{
    if (m_pTS != NULL)
        m_pTS->TileBlt(pDC, x, y, m_yLoc, dwRop);
    else if (m_crTrans != m_crSmall)
    {
        // Only draw color patch if not the transparent color.
        wxPen pen(m_crSmall);
        pDC.SetPen(pen);
        wxBrush brush(m_crSmall);
        pDC.SetBrush(brush);
        pDC.DrawRectangle(x, y, m_size.cx, m_size.cy);
    }
}

void CTile::StretchBlt(CDC& pDC, int x, int y, int cx, int cy, DWORD dwRop) const
{
    if (m_pTS != NULL)
        m_pTS->StretchBlt(pDC, x, y, cx, cy, m_yLoc, dwRop);
    else if (m_crTrans != m_crSmall)
    {
        CBrush oBrsh;
        oBrsh.CreateSolidBrush(m_crSmall);
        CBrush* pPrvBrsh = pDC.SelectObject(&oBrsh);
        pDC.PatBlt(x, y, cx, cy, PATCOPY);
        pDC.SelectObject(pPrvBrsh);
    }
}

// Transparent BitBlt
void CTile::TransBlt(CDC& pDC, int x, int y, const BITMAP* pMaskBMapInfo /* = NULL */) const
{
    if (m_pTS != NULL)
    {
        if (m_crTrans != noColor)
        {
            if (pMaskBMapInfo != NULL)
            {
                m_pTS->TransBltThruDIBSectMonoMask(pDC, x, y, m_yLoc,
                    m_crTrans, *pMaskBMapInfo);
            }
            else
                m_pTS->TransBlt(pDC, x, y, m_yLoc, m_crTrans);
        }
        else
            m_pTS->TileBlt(pDC, x, y, m_yLoc, SRCCOPY);
    }
    else if (m_crTrans != m_crSmall)
    {
        // Only draw color patch if not the transparent color.
        CBrush oBrsh;
        oBrsh.CreateSolidBrush(m_crSmall);
        CBrush* pPrvBrsh = pDC.SelectObject(&oBrsh);
        pDC.PatBlt(x, y, m_size.cx, m_size.cy, PATCOPY);
        pDC.SelectObject(pPrvBrsh);
    }
}

// Updates the tile image in-place
void CTileUpdatable::Update(const CBitmap& pBMap)
{
    /* CTileSheet only ever gets created as non-const instances,
        so const_cast is well-defined */
    const_cast<CTileSheet&>(CheckedDeref(m_pTS)).UpdateTile(pBMap, m_yLoc);
}

OwnerPtr<CBitmap> CTile::CreateBitmapOfTile() const
{
    return CheckedDeref(m_pTS).CreateBitmapOfTile(m_yLoc);
}

