// MoveHist.h
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

#ifndef _MOVEHIST_H
#define _MOVEHIST_H

class CHistRecord
{
public:
    CTime   m_timeCreated;          // Time from move file
    CTime   m_timeAbsorbed;         // Time record entered history
    CString m_strTitle;             // User title of move packet
    CString m_strDescr;             // User description of move packet
    OwnerOrNullPtr<CMoveList> m_pMList;            // Move list for this record (>= Ver2.90)
    DWORD   m_dwFilePos;            // Location of move list in game file (< Ver2.90)
    // We need to save the version of the game file
    // when the moves were absorbed.
    int     m_nGamFileVersion;      // Version of Game when saved history (< Ver2.90)

public:
    CHistRecord();
    ~CHistRecord() = default;
    void Serialize(CArchive& ar);
};

// The history table stores a record of playback in
// chronological order.
// ptr since objects xfer between gamdoc and here (TODO:  maybe switch ptr to obj?)
class CHistoryTable : private std::vector<OwnerPtr<CHistRecord>>
{
public:
    ~CHistoryTable() { Clear(); }
    // ------- //
    size_t GetNumHistRecords() const { return size(); }
    void AddNewHistRecord(OwnerPtr<CHistRecord> pHist) { push_back(std::move(pHist)); }
    CHistRecord& GetHistRecord(size_t nIndex)
        { return *at(nIndex).get(); }
    // ------- //
    void Clear();
    void Serialize(CArchive& ar);
};

#endif


