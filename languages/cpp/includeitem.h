/* 
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

#ifndef INCLUDEITEM_H
#define INCLUDEITEM_H
#include <kurl.h>

namespace Cpp {
struct IncludeItem {
  IncludeItem() : isDirectory(false), pathNumber(0) {
  }
  ///If this is true, this item represents a sub-directory. Else it represents a file.
  bool isDirectory;
  ///The name of this include-item, starting behind basePath.
  QString name;
  ///The path this item was/is included from.
  KUrl basePath;
  ///Which path in the include-path was used to find this item?
  int pathNumber;
};
}

#endif
