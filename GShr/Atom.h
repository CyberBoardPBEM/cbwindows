// Atom.h
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

#ifndef _ATOM_H
#define _ATOM_H

#include <forward_list>

template<typename Atom>
class AtomList
{
private:
    struct Node;
    using WeakPtr = std::weak_ptr<Node>;
    using List = std::forward_list<WeakPtr>;

public:
    using AtomID = std::shared_ptr<Node>;
    using iterator = typename List::iterator;

    AtomList() = default;
    AtomList(const AtomList&) = delete;
    AtomList& operator=(const AtomList&) = delete;
    ~AtomList() = default;

    iterator begin() { return list.begin(); }
    iterator end() { return list.end(); }
    size_t size() const
    {
        ASSERT(value_preserving_cast<size_t>(std::distance(list.begin(), list.end())) == sizeList);
        return sizeList;
    }
    void clear() { list.clear(); }

    AtomID Register(Atom&& atom)
    {
        iterator it = std::find_if(begin(), end(),
                                    [&atom](const WeakPtr& p)
                                    {
                                        return **p.lock() == atom;
                                    });
        if (it != end())
        {
            return it->lock();
        }
        AtomID retval(new Node(*this, std::move(atom)));
        list.push_front(retval);
        ++sizeList;
        return retval;
    }

private:
    class Node
    {
    public:
        Node(AtomList& l, Atom&& a) :
            list(l),
            atom(std::move(a))
        {
        }
        ~Node()
        {
            list.RemoveExpired();
        }

        const Atom* get() { return &atom; }
        const Atom* operator->() { return &atom; }
        const Atom& operator*() { return atom; }

    private:
        AtomList& list;
        // atom is effectively a key, hence const
        const Atom atom;
    };

    void RemoveExpired()
    {
        for (iterator before_it = list.before_begin(),
                        it = list.begin() ;
            it != list.end() ;
            before_it = it++)
        {
            if (it->expired())
            {
                list.erase_after(before_it);
                --sizeList;
                return;
            }
        }
        ASSERT(!"no expired ptr");
    }

    List list;
    // forward_list doesn't have size()
    size_t sizeList = size_t(0);
};
#endif
