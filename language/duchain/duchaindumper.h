/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
    Copyright 2010 Milian Wolff <mail@milianw.de>
    Copyright 2014 Kevin Funk <kfunk@kde.org>

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

#ifndef KDEVPLATFORM_DUMPCHAIN_H
#define KDEVPLATFORM_DUMPCHAIN_H

#include <language/languageexport.h>

#include <QFlags>
#include <QScopedPointer>

namespace KDevelop
{
class DUContext;


/**
 * @brief Debugging utility function to dump a DUContext including contained declarations.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUChainDumper
{
public:
  enum Feature {
    NoFeatures =        0,
    DumpContext =       1 << 0,
    DumpProblems =      1 << 1
  };
  Q_DECLARE_FLAGS(Features, Feature);

  DUChainDumper(Features features = DumpContext);
  ~DUChainDumper();

  /**
   * Dump DUChain context to stdout
   *
   * NOTE: The DUChain must be readlocked when this is called.
   *
   * @param context The context to dump
   * @param allowedDepth How deep the dump will go into imported contexts, printing all the contents.
   */
  void dump(DUContext* context, int allowedDepth = 0);

private:
  struct Private;
  QScopedPointer<Private> d;
};

}
#endif // KDEVPLATFORM_DUMPCHAIN_H
