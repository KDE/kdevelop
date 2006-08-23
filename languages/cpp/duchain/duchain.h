/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef DUCHAIN_H
#define DUCHAIN_H

#include <QObject>

#include <kurl.h>

class DUContext;

/**
 * Holds references to all top level source file contexts.
 *
 * \todo to pull the sorting off properly, will need to know the location of
 *       the defines used to pull in URLs other than the source file URL.
 *
 * \todo need to do some tricky reference counting; need to know the file include tree.
 *       will probably have to assume an acyclic structure for this (ie. header guards)
 *       otherwise the code wouldn't compile anyway.
 */
class DUChain : public QObject
{
  Q_OBJECT

public:
  DUContext* chainForDocument(const KUrl& document);
  void addDocumentChain(const KUrl& document, DUContext* chain);

  void clear();

  static DUChain* self();

public slots:
  void removeDocumentChain(const KUrl& document);

private:
  static DUChain* s_chain;
  QMap<KUrl, DUContext*> m_chains;
};

#endif // DUCHAIN_H

// kate: indent-width 2;
