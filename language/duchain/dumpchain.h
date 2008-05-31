/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef DUMPCHAIN_H
#define DUMPCHAIN_H

#include "languageexport.h"
#include <QString>

namespace KTextEditor {
  class SmartRange;
}

namespace KDevelop
{
class DUContext;

class KDEVPLATFORMLANGUAGE_EXPORT DumpChain
{
public:
  DumpChain();
  virtual ~DumpChain();

  ///@param context The context to dump
  ///@param imported internal
  void dump(KDevelop::DUContext* context, bool imported = false);
  
  QString dumpRanges(KTextEditor::SmartRange* range, QString indent = QString());

private:
  int indent;
};
}
#endif // DUMPCHAIN_H
