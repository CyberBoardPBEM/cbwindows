// DragDrop.cpp
//
// Copyright (c) 2022-2024 By Bill Su, All Rights Reserved.
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

#include "stdafx.h"

#include "DragDrop.h"

// keep SubInfos constructed to match m_dragType
void DragInfo::SetDragType(DragType dt)
{
    // destruct old subInfo
    switch (m_dragType)
    {
        case DRAG_TILE:
            subInfos.m_tile.~SubInfo<DRAG_TILE>();
            break;
        case DRAG_TILELIST:
            subInfos.m_tileList.~SubInfo<DRAG_TILELIST>();
            break;
        case DRAG_PIECE:
            subInfos.m_piece.~SubInfo<DRAG_PIECE>();
            break;
        case DRAG_PIECELIST:
            subInfos.m_pieceList.~SubInfo<DRAG_PIECELIST>();
            break;
        case DRAG_MARKER:
            subInfos.m_marker.~SubInfo<DRAG_MARKER>();
            break;
        case DRAG_SELECTLIST:
            subInfos.m_selectList.~SubInfo<DRAG_SELECTLIST>();
            break;
        case DRAG_SELECTVIEW:
            subInfos.m_selectView.~SubInfo<DRAG_SELECTVIEW>();
            break;
    }
    m_dragType = DRAG_INVALID;

    // construct new subInfo
    switch (dt)
    {
        case DRAG_TILE:
            new (&subInfos.m_tile) SubInfo<DRAG_TILE>;
            break;
        case DRAG_TILELIST:
            new (&subInfos.m_tileList) SubInfo<DRAG_TILELIST>;
            break;
        case DRAG_PIECE:
            new (&subInfos.m_piece) SubInfo<DRAG_PIECE>;
            break;
        case DRAG_PIECELIST:
            new (&subInfos.m_pieceList) SubInfo<DRAG_PIECELIST>;
            break;
        case DRAG_MARKER:
            new (&subInfos.m_marker) SubInfo<DRAG_MARKER>;
            break;
        case DRAG_SELECTLIST:
            new (&subInfos.m_selectList) SubInfo<DRAG_SELECTLIST>;
            break;
        case DRAG_SELECTVIEW:
            new (&subInfos.m_selectView) SubInfo<DRAG_SELECTVIEW>;
            break;
    }
    m_dragType = dt;
}

// keep SubInfos constructed to match m_dragType
void DragInfoWx::SetDragType(DragType dt)
{
    // destruct old subInfo
    switch (m_dragType)
    {
        case DRAG_TILE:
            subInfos.m_tile.~SubInfo<DRAG_TILE>();
            break;
        case DRAG_TILELIST:
            subInfos.m_tileList.~SubInfo<DRAG_TILELIST>();
            break;
#if 0
        case DRAG_PIECE:
            subInfos.m_piece.~SubInfo<DRAG_PIECE>();
            break;
        case DRAG_PIECELIST:
            subInfos.m_pieceList.~SubInfo<DRAG_PIECELIST>();
            break;
        case DRAG_MARKER:
            subInfos.m_marker.~SubInfo<DRAG_MARKER>();
            break;
        case DRAG_SELECTLIST:
            subInfos.m_selectList.~SubInfo<DRAG_SELECTLIST>();
            break;
        case DRAG_SELECTVIEW:
            subInfos.m_selectView.~SubInfo<DRAG_SELECTVIEW>();
            break;
#endif
        case DRAG_INVALID:
            // do nothing
            break;
        default:
            CbThrowBadCastException();
    }
    m_dragType = DRAG_INVALID;

    // construct new subInfo
    switch (dt)
    {
        case DRAG_TILE:
            new (&subInfos.m_tile) SubInfo<DRAG_TILE>;
            break;
        case DRAG_TILELIST:
            new (&subInfos.m_tileList) SubInfo<DRAG_TILELIST>;
            break;
#if 0
        case DRAG_PIECE:
            new (&subInfos.m_piece) SubInfo<DRAG_PIECE>;
            break;
        case DRAG_PIECELIST:
            new (&subInfos.m_pieceList) SubInfo<DRAG_PIECELIST>;
            break;
        case DRAG_MARKER:
            new (&subInfos.m_marker) SubInfo<DRAG_MARKER>;
            break;
        case DRAG_SELECTLIST:
            new (&subInfos.m_selectList) SubInfo<DRAG_SELECTLIST>;
            break;
        case DRAG_SELECTVIEW:
            new (&subInfos.m_selectView) SubInfo<DRAG_SELECTVIEW>;
            break;
#endif
        case DRAG_INVALID:
            // do nothing
            break;
        default:
            CbThrowBadCastException();
    }
    m_dragType = dt;
}
