/*
 * This file is part of KDevelop
 *
 * Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "quickopen.h"

using namespace KDevelop;

IncludeFileData::IncludeFileData( const KUrl& file, bool isDir ) : m_file(file), m_isDirectory(isDir) {
}

QString IncludeFileData::text() const {
  return m_file.toLocalFile();
}

QString IncludeFileData::htmlDescription() const {
  if(m_isDirectory)
    return QString("Directory " + m_file.prettyUrl());
  else
    return m_file.prettyUrl();
}

void IncludeFileDataProvider::setFilterText( const QString& text ) {
  m_oldText = text;
}

void IncludeFileDataProvider::reset() {
  m_oldText = QString();
}

uint IncludeFileDataProvider::itemCount() const {
  return 5;
}

QList<QuickOpenDataPointer> IncludeFileDataProvider::data( uint start, uint end ) const
{
  //Just for testing, provide some content.
  QList<QuickOpenDataPointer> ret;
  for( uint a = start; a < end; a++ )
    ret << QuickOpenDataPointer( new IncludeFileData( QString("include-file %1").arg(a) ) );
  return ret;
}

