/* This file is part of KDevelop
*  Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.

*/
#include "astyle_stringiterator.h"

#include <string>

AStyleStringIterator::AStyleStringIterator(const QString &text)
    : ASSourceIterator()
    , m_content(text)
    , m_is(&m_content, QIODevice::ReadOnly)
    , m_peekStart(-1)
{
}


AStyleStringIterator::~AStyleStringIterator()
{
}

astyle::streamoff AStyleStringIterator::tellg()
{
  return m_is.pos();
}

int AStyleStringIterator::getStreamLength() const
{
  return m_content.size();
}

bool AStyleStringIterator::hasMoreLines() const
{
  return !m_is.atEnd();
}


std::string AStyleStringIterator::nextLine(bool emptyLineWasDeleted)
{
  Q_UNUSED(emptyLineWasDeleted)
  return m_is.readLine().toUtf8().data();
}

std::string AStyleStringIterator::peekNextLine()
{
    if (m_peekStart == -1) {
        m_peekStart = m_is.pos();
    }
    return m_is.readLine().toUtf8().data();
}

void AStyleStringIterator::peekReset()
{
    if(m_peekStart != -1)
        m_is.seek(m_peekStart);
    m_peekStart = -1; // invalid
}

astyle::streamoff AStyleStringIterator::getPeekStart() const
{
    // NOTE: we're not entirely sure if this is the correct implementation.
    // we're trying to work-around https://bugs.kde.org/show_bug.cgi?id=399048
    return m_peekStart == -1 ? 0 : m_peekStart;
}


