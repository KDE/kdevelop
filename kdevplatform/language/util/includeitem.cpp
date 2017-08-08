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

#include "includeitem.h"

#include <QDebug>

#include <util/path.h>

using namespace KDevelop;

IncludeItem::IncludeItem()
    : pathNumber(0)
    , isDirectory(false)
{
}

///Constructs the url from basePath and name.
// TODO: port this to Path
QUrl IncludeItem::url() const
{
  QUrl u;
  if( !basePath.isEmpty() ) {
    u = Path(Path(basePath), name ).toUrl();
  }else{
    u = QUrl::fromLocalFile( name );
  }
  return u;
}

QDebug operator<<(QDebug dbg, const IncludeItem& item)
{
    return dbg << item.url();
}
