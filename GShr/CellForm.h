// CellForm.h
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

#ifndef _CELLFORM_H
#define _CELLFORM_H

// Radio buttons in dialog match the ordering in this
// enumeration.

// NOTE: These constants are stored in the game box files!
// DO NOT mess with their values!

enum CellFormType { cformRect = 0, cformBrickHorz, cformBrickVert,
    cformHexFlat, cformHexPnt };

enum class CellStagger { Invalid = -1, Out = 0, In = 1 };

// impl could be in .cpp, but I'm putting it here so behavior is obvious
inline CellStagger operator~(CellStagger e)
{
    switch (e)
    {
        case CellStagger::Out:
        case CellStagger::In:
            static_assert((static_cast<int>(CellStagger::In) ^ 1) == static_cast<int>(CellStagger::Out), "invalid invert logic");
            return static_cast<CellStagger>(static_cast<int>(e) ^ 1);
        default:
            AfxThrowInvalidArgException();
    }
}

enum CellNumStyle { cnsRowCol = 0, cnsColRow = 1, cns0101ByRows = 2,
    cns0101ByCols = 3, cnsAA01ByRows = 4, cnsAA01ByCols = 5};

enum class Edge {
    /* values are chosen that make edges unique (to allow
        using switch) and to allow or'ing together to
        create Corner values */
    Invalid =   0x0000,

    Top =       0x0001,
    Bottom =    0x0011,
    MaskTB =    0x0010,

    Left =      0x0100,
    Right =     0x1100,
    MaskLR =    0x1000,
};

// impl could be in .cpp, but I'm putting it here so behavior is obvious
inline Edge operator~(Edge e)
{
    switch (e)
    {
        case Edge::Top:
        case Edge::Bottom:
            return static_cast<Edge>(static_cast<int>(e) ^ static_cast<int>(Edge::MaskTB));
        case Edge::Left:
        case Edge::Right:
            return static_cast<Edge>(static_cast<int>(e) ^ static_cast<int>(Edge::MaskLR));
        default:
            AfxThrowInvalidArgException();
    }
}

enum class Corner {
    Invalid = 0x000,
    TL = static_cast<int>(Edge::Top) | static_cast<int>(Edge::Left),
    TR = static_cast<int>(Edge::Top) | static_cast<int>(Edge::Right),
    BL = static_cast<int>(Edge::Bottom) | static_cast<int>(Edge::Left),
    BR = static_cast<int>(Edge::Bottom) | static_cast<int>(Edge::Right),
};

// impl could be in .cpp, but I'm putting it here so behavior is obvious
inline Corner operator^(Corner c, Edge e)
{
    switch (c)
    {
        case Corner::TL:
        case Corner::TR:
        case Corner::BL:
        case Corner::BR:
            break;
        default:
            AfxThrowInvalidArgException();
    }
    switch (e)
    {
        case Edge::MaskTB:
        case Edge::MaskLR:
            return static_cast<Corner>(static_cast<int>(c) ^ static_cast<int>(e));
        default:
            AfxThrowInvalidArgException();
    }
}

////////////////////////////////////////////////////////////////
// Various create forms:
// Rectangle --> pCell.CreateCell(cformRect, height, width);
// Bricks --> pCell.CreateCell(cformBrickHorz, height, width, cellStagger);
// Bricks --> pCell.CreateCell(cformBrickVert, height, width, cellStagger);
// HexFlatUp --> pCell.CreateCell(cformHexFlat, height, 0, cellStagger);
// HexPointUp --> pCell.CreateCell(cformHexPnt, width, 0, cellStagger);

class CCellForm
{
// Constructors / Destructors
public:
    CCellForm();
    CCellForm(const CCellForm&) = delete;
    CCellForm& operator=(const CCellForm&) = delete;
    CCellForm(CCellForm&&) = default;
    CCellForm& operator=(CCellForm&&) = default;
    ~CCellForm() = default;

// Attributes
public:
    const CBitmap* GetMask() const { return m_pMask ? &*m_pMask : nullptr; }
    const BITMAP*  GetMaskMemoryInfo() const { return m_pMask != NULL ? &m_bmapMask : NULL; }
    BOOL     HasMask() const { return m_pMask != NULL; }

    CRect GetRect(CB::ssize_t row, CB::ssize_t col) const;
    CSize GetCellSize() const { return CSize(m_rct.right, m_rct.bottom); }
    CellFormType GetCellType() const { return m_eType; }
    CellStagger GetCellStagger() const { return m_nStagger; }

// Operations
public:
    void Clear();
    void CreateCell(CellFormType eType, int nParm1, int nParm2 = 0,
        CellStagger nStagger = CellStagger::Invalid);
    void FindCell(long x, long y, CB::ssize_t& row, CB::ssize_t& col) const;
    void FillCell(CDC& pDC, int xPos, int yPos) const;
    void FrameCell(CDC& pDC, int xPos, int yPos) const;
    CSize CalcBoardSize(size_t nRows, size_t nCols) const;
    BOOL CalcTrialBoardSize(size_t nRows, size_t nCols) const;

    BOOL CompareEqual(const CCellForm& cf) const;

    // ------- //
    static CB::string GetCellNumberStr(CellNumStyle eStyle, size_t row, size_t col);
    // ------- //
    void Serialize(CArchive& ar);

// Implementation
protected:
    CellFormType m_eType;       // Format of cell
    CellStagger  m_nStagger;    // 0 = normal. 1 = alternate
    CRect        m_rct;         // Rectangle enclosing cell
    std::vector<POINT> m_pPoly;  // Zero based set of points
    OwnerOrNullPtr<CBitmap> m_pMask; // Only defined for hexes
    BITMAP       m_bmapMask;    // Only filled if m_pMask is defined
    // ------- //
    static std::vector<POINT> m_pWrk;   // Scratch copy of m_pPoly
    // ------- //
    static void OffsetPoly(std::vector<POINT>& pPoly, int xOff, int yOff);
    // ------- //
    CB::ssize_t CellPhase(CB::ssize_t val) const { return ((val ^ static_cast<int>(m_nStagger)) & 1); }
    void CreateHexMask();
};

#endif

