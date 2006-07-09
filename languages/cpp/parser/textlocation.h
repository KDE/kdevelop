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

#ifndef TEXTLOCATION_H
#define TEXTLOCATION_H

#include <QList>
#include <QPair>

#include <kurl.h>

#include <ktexteditor/range.h>
#include <ktexteditor/rangefeedback.h>

namespace KTextEditor { class SmartCursor; }

/**
 * A class for representing a position in a specific text document,
 * whether the file is loaded or not.
 *
 * This class is only meant for temporary use, and thus does not try
 * to track the position in the document when the document contents
 * change.
 */
class TextPosition
{
public:
  /**
   * Constructor which accepts a text \a position and the \a url to which it refers.
   */
  TextPosition(const KTextEditor::Cursor& position, const KUrl& url);

  /**
   * Constructor which accepts a smart cursor for purposes of information
   * extraction only.
   */
  TextPosition(KTextEditor::SmartCursor* position);

  /// Default destructor
  inline virtual ~TextPosition() {}

  /**
   * Returns the text range covered by this object.
   */
  const KTextEditor::Cursor& textPosition() const;

  /**
   * Returs the file where this context begins (and may or may not wholly contain).
   */
  const KUrl& url() const;

  /**
   * Sets the text position.
   */
  void setTextPosition(const KTextEditor::Cursor& position, const KUrl& url);

  /**
   * As with the constructor, this function takes a smart cursor for extraction
   * of position and url details only, it does not take ownership of the smart cursor.
   * \overload
   */
  void setTextPosition(KTextEditor::SmartCursor* position);

private:
  KTextEditor::Cursor m_position;
  KUrl m_url;
};

/**
 * A class for representing an object with an associated text range,
 * whether the file is loaded or not.
 */
class TextRange : public KTextEditor::SmartRangeWatcher
{
public:
  TextRange(const KTextEditor::Range& range = KTextEditor::Range::invalid(), const KUrl& url = KUrl());
  TextRange(KTextEditor::SmartRange* range);

  /**
   * Returns the text range covered by this object.
   */
  KTextEditor::Range& textRange();

  /**
   * Returns the text range covered by this object.
   */
  const KTextEditor::Range& textRange() const;

  /**
   * Returs the file where this context begins (and may or may not wholly contain).
   */
  const KUrl& url() const;

  /**
   * Sets a range of text covered by this object.
   */
  void setTextRange(const KTextEditor::Range& range, const KUrl& url);

  /**
   * Returns the smart text range, if the file is loaded in memory.
   */
  KTextEditor::SmartRange* smartTextRange() const;

  /**
   * Set a smart text range.  When the range is deleted (eg. the open file is
   * closed), this will be detected, and the object will switch to static range.
   */
  void setSmartTextRange(KTextEditor::SmartRange* range);

  /// Override: detect when the range is deleted.
  virtual void rangeDeleted(KTextEditor::SmartRange* range);

  /**
   * Test to see if this range contains \a position.
   *
   * \todo follow include heirachy to test other documents
   */
  bool contains(const TextPosition& position) const;

private:
  KTextEditor::Range* m_range;
  KUrl m_url;
};

#endif // TEXTLOCATION_H

// kate: indent-width 2;
