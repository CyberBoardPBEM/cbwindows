// TileSet.cpp
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

BOOL CTileSet::HasTileID(TileID tid) const
{
    for (size_t i = 0; i < m_tidTbl.size(); i++)
    {
        if (m_tidTbl.at(i) == tid)
            return TRUE;
    }
    return FALSE;
}

void CTileSet::RemoveTileID(TileID tid)
{
    for (size_t i = 0; i < m_tidTbl.size(); i++)
    {
        if (m_tidTbl.at(i) == tid)
        {
            m_tidTbl.erase(m_tidTbl.begin() + value_preserving_cast<ptrdiff_t>(i));
            return;
        }
    }
}

size_t CTileSet::FindTileID(TileID tid) const
{
    for (size_t i = 0; i < m_tidTbl.size(); i++)
    {
        if (m_tidTbl.at(i) == tid)
            return i;
    }
    return Invalid_v<size_t>;
}

void CTileSet::AddTileID(TileID tid, size_t nPos /* = Invalid_v<size_t> */)
{
    if (nPos == Invalid_v<size_t>)
        m_tidTbl.push_back(tid);
    else
    {
        ASSERT(nPos <= m_tidTbl.size());
        m_tidTbl.insert(m_tidTbl.begin() + value_preserving_cast<ptrdiff_t>(nPos), tid);
    }
}

void CTileSet::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_strName;
        ar << m_tidTbl;
    }
    else
    {
        ar >> m_strName;
        ar >> m_tidTbl;
    }
}


