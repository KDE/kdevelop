/* This  is part of KDevelop
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

#include "definition.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

using namespace KTextEditor;

//BEGIN TextPosition

TextPosition::TextPosition( const KTextEditor::Cursor & position, const KUrl & url )
{
  setTextPosition(position, url);
}

TextPosition::TextPosition( KTextEditor::SmartCursor * position )
{
  setTextPosition(position);
}

const KTextEditor::Cursor & TextPosition::textPosition( ) const
{
  return m_position;
}

const KUrl & TextPosition::url( ) const
{
  return m_url;
}

void TextPosition::setTextPosition( const KTextEditor::Cursor & position, const KUrl & url )
{
  m_position = position;
  m_url = url;
}

void TextPosition::setTextPosition( KTextEditor::SmartCursor* position )
{
  m_position = *position;
  m_url = position->document()->url();
}

//END

//BEGIN TextRange

TextRange::TextRange(const Range& range, const KUrl& url)
  : m_range(new Range(range))
  , m_url(url)
{
}

TextRange::TextRange(SmartRange* range)
  : m_range(0)
{
  setSmartTextRange(range);
}

Range & TextRange::textRange( )
{
  return *m_range;
}

const Range & TextRange::textRange( ) const
{
  return *m_range;
}

SmartRange * TextRange::smartTextRange( ) const
{
  if (m_range->isSmartRange())
    return static_cast<SmartRange*>(m_range);

  return 0;
}

void TextRange::setTextRange( const Range & range, const KUrl& url )
{
  if (smartTextRange())
    smartTextRange()->removeWatcher(this);

  delete m_range;
  m_range = new Range(range);
  m_url = url;
}

void TextRange::rangeDeleted( SmartRange * range )
{
  setTextRange(*range, range->document()->url());
}

const KUrl& TextRange::url( ) const
{
  return m_url;
}

void TextRange::setSmartTextRange( KTextEditor::SmartRange* range )
{
  m_range = range;
  m_url = range->document()->url();
}

//END

// kate: indent-width 2;
