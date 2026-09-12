// TileSht.cpp
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

#include    "stdafx.h"
#ifdef      GPLAY
    #include    "Gp.h"
    #include    "GameBox.h"
    #include    "GamDoc.h"
#else
    #include    "Gm.h"
    #include    "GmDoc.h"
#endif
#include    "CDib.h"
#include    "Tile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////

CTileSheet::CTileSheet()
{
    m_size = wxSize(0, 0);
    m_sheetHt = 0;
}

//////////////////////////////////////////////////////////////////

void CTileSheet::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << value_preserving_cast<int16_t>(m_size.x);
        ar << value_preserving_cast<int16_t>(m_size.y);

        if (m_pBMap.IsOk())
        {
            CDib dib(*CB::Convert(m_pBMap));
            wxASSERT(dib);
            if (dib)
            {
                ar << uint16_t(1);      // Store "HasBitmap" flag
#ifndef GPLAY
                dib.SetCompressLevel(CB::ToCGamDoc(ar.m_pDocument)->
                    GetCompressLevel());
#endif
                ar << dib;
            }
            else
                ar << uint16_t(0);      // Store "HasBitmap" flag
        }
        else
            ar << uint16_t(0);          // Store "HasBitmap" flag
    }
    else
    {
        ClearSheet();
        int16_t sTmp;
        ar >> sTmp; m_size.x = sTmp;
        ar >> sTmp; m_size.y = sTmp;

        uint16_t wHasBitmap = 1;            // Force for old versions of file
#ifdef GPLAY
        if (CGameBox::GetLoadingVersion() > NumVersion(0, 52))
#else
        if (CGamDoc::GetLoadingVersion() > NumVersion(0, 52))
#endif
            ar >> wHasBitmap;

        if (wHasBitmap)
        {
            CDib dib;
            ar >> dib;
            if (dib)
            {
                m_pBMap = dib;
                CPP20_TRACE("-- Loaded Tile Sheet for cx={}, cy={} tiles. Tile sheet height = {}\n",
                    m_size.x, m_size.y, m_pBMap.GetHeight());
                //  wxASSERT(m_pBMap.GetHeight() < maxSheetHeight);
                m_sheetHt = m_pBMap.GetHeight();
            }
            else
            {
                CPP20_TRACE("CTileSheet::Serialize - Forced truncation of tile sheet. "
                    "No DIB found even though should have one.\n");
                m_sheetHt = 0;
            }
        }
        else
            m_sheetHt = 0;
    }
}

//////////////////////////////////////////////////////////////////

void CTileSheet::SetSize(wxSize size)
{
    wxASSERT(m_sheetHt == 0 && !m_pBMap.IsOk());  // ONLY LEGAL IF NO DATA IN SHEET!!!!!!
    m_size = size;
}

//////////////////////////////////////////////////////////////////

void CTileSheet::CreateTile()
{
    if (m_pBMap.IsOk())
    {
        int bmHeight = m_pBMap.GetHeight() + m_size.y;       // Increase size.

        wxBitmap pBMap = wxBitmap(
            m_pBMap.GetWidth(), bmHeight);
        wxASSERT(pBMap.IsOk());
        {
        wxMemoryDC mDC1(pBMap);          // Dest bitmap
        wxMemoryDC mDC2(m_pBMap);        // Source bitmap

        mDC1.Blit(0, 0, m_size.x, m_sheetHt, &mDC2, 0, 0, wxCOPY);
        mDC1.Blit(0, m_sheetHt, m_size.x, m_size.y, &mDC1, 0, 0, wxSET);
        }

        m_sheetHt = bmHeight;

        m_pBMap = pBMap;
    }
    else
    {
        wxASSERT(m_size != wxSize(0,0));
        CPP20_TRACE("CTileSheet::CreateTile - Creating new TileSheet bitmap\n");
        m_pBMap = wxBitmap(
            m_size.x, m_size.y);
        wxMemoryDC mDC1(m_pBMap);
        mDC1.Blit(0, 0, m_size.x, m_size.y, &mDC1, 0, 0, wxSET);

        m_sheetHt = m_size.y;
    }
}

void CTileSheet::DeleteTile(int yLoc)
{
    wxASSERT(m_pBMap.IsOk());
    wxASSERT(yLoc < m_sheetHt - 1);
    if (m_size.y == m_sheetHt)
    {
        CPP20_TRACE("CTileSheet::DeleteTile - Deleting TileSheet bitmap\n");
        // Single tile is all that's left...
        wxASSERT(yLoc == 0);
        m_pBMap = wxBitmap();
        m_sheetHt = 0;
    }
    else
    {
        int bmHeight = m_pBMap.GetHeight() - m_size.y;           // Decrease size.

        wxBitmap pBMap = wxBitmap(
            m_pBMap.GetWidth(), bmHeight);

        {
        wxMemoryDC mDC2(m_pBMap);        // Source bitmap

        wxMemoryDC mDC1(pBMap);          // Dest bitmap

        if (yLoc > 0)
        {
            // There is a piece above the deleted piece.
            mDC1.Blit(0, 0, m_size.x, yLoc, &mDC2, 0, 0, wxCOPY);
        }
        if (yLoc + m_size.y < m_sheetHt)           // See if not last tile
        {
            // There is a piece after the deleted piece.
            mDC1.Blit(0, yLoc, m_size.x, m_sheetHt - (yLoc + m_size.y),
                &mDC2, 0, yLoc + m_size.y, wxCOPY);
        }
        }

        m_sheetHt = bmHeight;

        m_pBMap = pBMap;
    }
}

void CTileSheet::UpdateTile(const wxBitmap& pBMap, int yLoc)
{
    wxASSERT(m_pBMap.IsOk());
    wxASSERT(yLoc < m_sheetHt - 1);
    wxMemoryDC mDC1(m_pBMap);        // Dest bitmap
    wxMemoryDC mDC2;
    mDC2.SelectObjectAsSource(pBMap);          // Source bitmap

    mDC1.Blit(0, yLoc, m_size.x, m_size.y, &mDC2, 0, 0, wxCOPY);
}

wxBitmap CTileSheet::CreateBitmapOfTile(int yLoc) const
{
    wxASSERT(m_pBMap.IsOk());
    wxASSERT(yLoc < m_sheetHt - 1);

    wxBitmap pBMap(m_size);

    {
    wxMemoryDC mDC1;
    mDC1.SelectObjectAsSource(m_pBMap);        // Source bitmap

    wxMemoryDC mDC2(pBMap);          // Activate dest bitmap

    mDC2.Blit(0, 0, m_size.x, m_size.y, &mDC1, 0, yLoc, wxCOPY);
    }

    return pBMap;
}

////////////////////////////////////////////////////////////////////////

void CTileSheet::TileBlt(wxDC& pDC, wxCoord xDst, wxCoord yDst, wxCoord ySrc, wxRasterOperationMode dwRop) const
{
    wxMemoryDC sheetDC;
    sheetDC.SelectObjectAsSource(m_pBMap);
    pDC.Blit(xDst, yDst, m_size.x, m_size.y, &sheetDC, 0, ySrc, dwRop);
}

void CTileSheet::TransBlt(wxDC& pDC, int xDst, int yDst, int ySrc,
    wxColour crTrans) const
{
    wxBitmap wxBMap = m_pBMap.GetSubBitmap(wxRect(wxPoint(0, 0), m_pBMap.GetSize()));
    wxBMap.SetMask(new wxMask(wxBMap, crTrans));
    wxMemoryDC srcDC;
    srcDC.SelectObjectAsSource(wxBMap);
    pDC.Blit(xDst, yDst, m_size.x, m_size.y, &srcDC, 0, ySrc, wxCOPY, true);
}

////////////////////////////////////////////////////////////////////////

void CTileSheet::TransBltThruDIBSectMonoMask(wxDC& pDC, int xDst, int yDst, int ySrc,
    wxColour crTrans, const wxBitmap& pMaskBMapInfo) const
{
    /* TODO:  when CB::Convert() no longer required, compare
        performance of these different implementations */
#if 0
    // can't modify bitmap while it's selected
    class DCBitmapChanger
    {
    public:
        DCBitmapChanger(wxMemoryDC& d, wxBitmap select) :
            dc(d),
            oldBitmap(dc.GetSelectedBitmap())
        {
            wxASSERT(!oldBitmap.HasAlpha());
            dc.SelectObject(select);
        }
        ~DCBitmapChanger()
        {
            // modifying oldBitmap causes it to gain alpha channel
            oldBitmap.ResetAlpha();
            dc.SelectObject(oldBitmap);
        }
        wxBitmap& GetOldBitmap() { return oldBitmap; }
    private:
        wxMemoryDC& dc;
        wxBitmap oldBitmap;
    } setBitmap(dynamic_cast<wxMemoryDC&>(pDC), wxNullBitmap);
    wxBitmap& hBMapDest = setBitmap.GetOldBitmap();

    BITMAP  bmapTile;

    memset(&bmapTile, 0, sizeof(BITMAP));

    m_pBMap->GetObject(sizeof(BITMAP), &bmapTile);
    wxASSERT(bmapTile.bmBits != NULL && hBMapDest.IsOk());

    wxPoint pntOrg = pDC.GetLogicalOrigin();
    wxASSERT(pntOrg == wxPoint(0, 0) || !"untested code");
    xDst += pntOrg.x;
    yDst += pntOrg.y;

    wxASSERT(bmapTile.bmBitsPixel == 24);
    ptrdiff_t srcStride = -bmapTile.bmWidthBytes;
    const std::byte* srcRowStart = static_cast<const std::byte*>(bmapTile.bmBits) +
                                        (bmapTile.bmHeight - 1 - ySrc) * -srcStride;
    wxASSERT(hBMapDest.GetDepth() == 32);
    wxAlphaPixelData hBMapDestData(hBMapDest);
    wxAlphaPixelData::Iterator destRowStart(hBMapDestData);
    destRowStart.MoveTo(hBMapDestData, xDst, yDst);
    wxASSERT(pMaskBMapInfo.GetDepth() == 24);
    // KLUDGE:  only reading mask, so const_cast safe
    wxNativePixelData pMaskBMapInfoData(const_cast<wxBitmap&>(pMaskBMapInfo));
    wxNativePixelData::Iterator maskRowStart(pMaskBMapInfoData);

    for (int y = 0 ;
        y < std::min(value_preserving_cast<int>(m_size.cy), pMaskBMapInfo.GetHeight()) &&
                yDst + y < hBMapDest.GetHeight() ;
        ++y)
    {
        if (yDst + y >= 0)
        {
            const WIN_RGBTRIO* src = reinterpret_cast<const WIN_RGBTRIO*>(srcRowStart);
            wxAlphaPixelData::Iterator dest = destRowStart;
            wxNativePixelData::Iterator mask = maskRowStart;
            for (int x = 0 ;
                x < std::min(value_preserving_cast<int>(m_size.cx), pMaskBMapInfo.GetWidth()) &&
                        xDst + x < hBMapDest.GetWidth() ;
                ++x)
            {
                wxColour crMask = wxColour(mask.Red(), mask.Green(), mask.Blue());
                ++mask;
                wxColour crSrc = CB::Convert(COLORREF(*src++));
                if (xDst + x >= 0)
                {
                    if (crMask == RGB(0, 0, 0) &&   // Only black mask bits get through
                        crSrc != crTrans)
                    {
                        dest.Red() = crSrc.Red();
                        dest.Green() = crSrc.Green();
                        dest.Blue() = crSrc.Blue();
                    }
                }
                ++dest;
            }
        }
        srcRowStart += srcStride;
        destRowStart.OffsetY(hBMapDestData, 1);
        maskRowStart.OffsetY(pMaskBMapInfoData, 1);
    }
#else
    wxPoint pntOrg = pDC.GetLogicalOrigin();
    xDst += pntOrg.x;
    yDst += pntOrg.y;

    wxBitmap accum(m_size);
    {
        wxMemoryDC accumDC(accum);
        {
            wxMemoryDC tileDC;
            tileDC.SelectObjectAsSource(m_pBMap);
            accumDC.Blit(wxPoint(0, 0), accum.GetSize(), &tileDC, wxPoint(0, ySrc), wxCOPY);
        }
        {
            wxBitmap trans(accum.GetSize());
            {
                wxMemoryDC transDC(trans);
                transDC.SetPen(wxPen(crTrans));
                transDC.SetBrush(wxBrush(crTrans));
                transDC.DrawRectangle(wxPoint(0, 0), trans.GetSize());
            }
            trans.SetMask(new wxMask(pMaskBMapInfo, *wxBLACK));
            accumDC.DrawBitmap(trans, 0, 0, true);
        }
    }
    accum.SetMask(new wxMask(accum, crTrans));
    pDC.DrawBitmap(accum, xDst, yDst, true);
#endif
}

////////////////////////////////////////////////////////////////////////

void CTileSheet::ClearSheet()
{
    m_size = wxSize(0, 0);
    m_pBMap = wxBitmap();
}
