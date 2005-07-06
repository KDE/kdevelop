/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef FASTLIST_H
#define FASTLIST_H

#include "smallobject.h"

template <typename Tp>
struct ListNode
{
  Tp element;
  int index;
  mutable const ListNode<Tp> *next;

  static ListNode *create(const Tp &element, pool *p)
  {
    ListNode<Tp> *node = new (p->allocate(sizeof(ListNode))) ListNode();
    node->element = element;
    node->index = 0;
    node->next = node;

    return node;
  }

  static const ListNode *merge(const ListNode<Tp> *n1, const ListNode<Tp> *n2)
  {
    const ListNode<Tp> *n1_front = n1->toFront();
    const ListNode<Tp> *n1_back = n1->toBack();

    const ListNode<Tp> *n2_front = n2->toFront();
    const ListNode<Tp> *n2_back = n2->toBack();

    n1_back->next = n2_front;
    n2_back->next = n1_front;

    return n2_back;
  }

  static ListNode *create(const ListNode *n1, const Tp &element, pool *p)
  {
    ListNode<Tp> *n2 = ListNode::create(element, p);

    n2->index = n1->index + 1;
    n2->next = n1->next;
    n1->next = n2;

    return n2;
  }

  inline const ListNode<Tp> *at(int index) const
  {
    const ListNode<Tp> *node = this;
    while (index != node->index)
      node = node->next;

    return node;
  }

  inline bool hasNext() const
  { return index < next->index; }

  inline int count() const
  { return 1 + toBack()->index; }

  inline const ListNode<Tp> *toFront() const
  { return toBack()->next; }

  inline const ListNode<Tp> *toBack() const
  {
    const ListNode<Tp> *node = this;
    while (node->hasNext())
      node = node->next;

    return node;
  }
};

template <class Tp>
inline const ListNode<Tp> *snoc(const ListNode<Tp> *list, 
				const Tp &element, pool *p)
{
  if (!list)
    return ListNode<Tp>::create(element, p);

  return ListNode<Tp>::create(list->toBack(), element, p);
}

#endif // FASTLIST_H


