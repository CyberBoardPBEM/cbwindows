// ClipBrd.cpp
//
// Copyright (c) 1994-2023 By Dale L. Larson & William Su, All Rights Reserved.
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
#include    "CDib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

BOOL IsClipboardBitmap()
{
    return IsClipboardFormatAvailable(CF_DIB);
}

void SetClipboardBitmap(CWnd* pWnd, const CBitmap& pBMap)
{
    LockWxClipboard lockClipbd(std::try_to_lock);
    if (lockClipbd)
    {
        wxBusyCursor busyCursor;

        wxImage img = ToImage(pBMap);
        wxBitmap wxbmp(img);
        if (!wxTheClipboard->SetData(new wxBitmapDataObject(wxbmp)))
        {
            AfxThrowMemoryException();
        }
    }
}

OwnerOrNullPtr<CBitmap> GetClipboardBitmap(CWnd* pWnd)
{
    LockWxClipboard lockClipbd(std::try_to_lock);
    if (!lockClipbd)
        return NULL;

    wxBusyCursor busyCursor;

    wxBitmapDataObject bdo;
    bool rc = wxTheClipboard->GetData(bdo);
    lockClipbd.Unlock();
    if (!rc)
    {
        return NULL;
    }
    wxBitmap wxbmp = bdo.GetBitmap();
    wxImage img = wxbmp.ConvertToImage();
    OwnerPtr<CBitmap> pBMap = ToBitmap(img);
    return pBMap;
}

void SetClipboardBitmap(const wxBitmap& pBMap)
{
    LockWxClipboard lockClipbd(std::try_to_lock);
    if (lockClipbd)
    {
        wxBusyCursor busyCursor;

        if (!wxTheClipboard->SetData(new wxBitmapDataObject(pBMap)))
        {
            AfxThrowMemoryException();
        }
    }
}

wxBitmap GetClipboardBitmap()
{
    LockWxClipboard lockClipbd(std::try_to_lock);
    if (!lockClipbd)
        return NULL;

    wxBusyCursor busyCursor;

    wxBitmapDataObject bdo;
    bool rc = wxTheClipboard->GetData(bdo);
    lockClipbd.Unlock();
    if (!rc)
    {
        return NULL;
    }
    wxBitmap wxbmp = bdo.GetBitmap();
    return wxbmp;
}

