/*
  This file is part of kdev-pg
  Copyright 2005, 2006 Roberto Raggi <roberto@kdevelop.org>

  Permission to use, copy, modify, distribute, and sell this software and its
  documentation for any purpose is hereby granted without fee, provided that
  the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation.

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  KDEVELOP TEAM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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


