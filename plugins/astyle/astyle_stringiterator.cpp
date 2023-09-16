/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "astyle_stringiterator.h"
#include <QIODevice>
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

std::streamoff AStyleStringIterator::tellg()
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

std::streamoff AStyleStringIterator::getPeekStart() const
{
    // NOTE: we're not entirely sure if this is the correct implementation.
    // we're trying to work-around https://bugs.kde.org/show_bug.cgi?id=399048
    return m_peekStart == -1 ? 0 : m_peekStart;
}


