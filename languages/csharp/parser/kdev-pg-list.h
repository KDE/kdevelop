/* This file is part of kdev-pg
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEV_PG_LIST_H
#define KDEV_PG_LIST_H

#include "kdev-pg-memory-pool.h"

template <typename _Tp>
struct list_node
{
  _Tp element;
  int index;
  mutable const list_node<_Tp> *next;

  static list_node *create(const _Tp &element, kdev_pg_memory_pool *p)
  {
    list_node<_Tp> *node = new (p->allocate(sizeof(list_node))) list_node();
    node->element = element;
    node->index = 0;
    node->next = node;

    return node;
  }

  static list_node *create(const list_node *n1, const _Tp &element, kdev_pg_memory_pool *p)
  {
    list_node<_Tp> *n2 = list_node::create(element, p);

    n2->index = n1->index + 1;
    n2->next = n1->next;
    n1->next = n2;

    return n2;
  }

  inline const list_node<_Tp> *at(int index) const
  {
    const list_node<_Tp> *node = this;
    while (index != node->index)
      node = node->next;

    return node;
  }

  inline bool has_next() const
  { return index < next->index; }

  inline int count() const
  { return 1 + to_back()->index; }

  inline const list_node<_Tp> *to_front() const
  { return to_back()->next; }

  inline const list_node<_Tp> *to_back() const
  {
    const list_node<_Tp> *node = this;
    while (node->has_next())
      node = node->next;

    return node;
  }
};

template <class _Tp>
inline const list_node<_Tp> *snoc(const list_node<_Tp> *list,
                                  const _Tp &element, kdev_pg_memory_pool *p)
{
  if (!list)
    return list_node<_Tp>::create(element, p);

  return list_node<_Tp>::create(list->to_back(), element, p);
}

#endif // KDEV_PG_LIST_H


