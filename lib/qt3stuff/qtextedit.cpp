/****************************************************************************
** $Id$
**
** Definition of the QTextEdit class
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qtextedit.h"
#include "qcursor.h"

/*!
  \class QTextEdit qtextedit.h
  \brief The QTextEdit widget provides a sophisticated single-page text editor.

  \ingroup basic

  QTextEdit is an advanced WYSIWYG editor supporting rich text
  formatting. It is optimized to handle large text documents and
  respond quickly to user input.

  If you create a new an empty QTextEdit, and want to allow the user
  to edit rich text (HTML), call setTextFormat(Qt::RichText) to be
  sure that text() returns the formatted text.

  QTextEdit extends QTextView with keyboard and mouse handling for
  user input and functions to set/get/modify formatting, text, etc.

  As it is derived from QTextView, it supports the same functions to
  set and load plain and HTML text.

  QTextEdit internally works on paragraphs. A paragraph is a formatted
  string which is word-wrapped to fit into the width of the
  widget. Paragraphs are seperated by hard breaks.

  For user input selections are important. To work with selections use
  setSelection() and QTextView::getSelection(). To set or get the
  position of the cursor use setCursorPosition() or
  getCursorPosition().

  To change the current format (the format at the cursor position) or
  the format of a selection, use setBold(), setItalic(),
  setUnderline(), setFamily(), setPointSize(), setFont() or
  setColor(). To change paragraph formatting use setAlignment() or
  setParagType(). When the cursor is moved, the signals
  currentFontChanged(), currentColorChanged() and
  currentAlignmentChanged() are emitted to inform about the format at
  the cursor position.

  To insert text at the cursor position use insert(). cut(), copy()
  and paste() can also be done through the API. If the text changes,
  the textChanged() signal is emitted, and if the user inserts a new
  line by pressing return/enter returnPressed() is emitted.

  QTextEdit provides command-based undo/redo. To set the depth of the
  command history use setUndoDepth() - it defaults to 100 steps. To
  undo or redo the last opetarion call undo() or redo(). The signals
  undoAvailable() and redoAvailable() inform about when undo or redo
  operations can be executed.

  Here is a list of key-bindings which are implemented for editing

  <ul>
  <li><i> Left Arrow </i> Move the cursor one character left
  <li><i> Right Arrow </i> Move the cursor one character right
  <li><i> Up Arrow </i> Move the cursor one line up
  <li><i> Down Arrow </i> Move the cursor one line down
  <li><i> Page Up </i> Move the cursor one page up
  <li><i> Page Down </i> Move the cursor one page down
  <li><i> Backspace </i> Delete the character to the left of the cursor
  <li><i> Home </i> Move the cursor to the beginning of the line
  <li><i> End </i> Move the cursor to the end of the line
  <li><i> Delete </i> Delete the character to the right of the cursor
  <li><i> Ctrl+A </i> Move the cursor to the beginning of the line
  <li><i> Ctrl+B </i> Move the cursor one character left
  <li><i> Ctrl+C </i> Copy the marked text to the clipboard (also
  <i>Ctrl+Insert</i> under Windows)
  <li><i> Ctrl+D </i> Delete the character to the right of the cursor
  <li><i> Ctrl+E </i> Move the cursor to the end of the line
  <li><i> Ctrl+F </i> Move the cursor one character right
  <li><i> Ctrl+H </i> Delete the character to the left of the cursor
  <li><i> Ctrl+K </i> Delete to end of line
  <li><i> Ctrl+N </i> Move the cursor one line down
  <li><i> Ctrl+P </i> Move the cursor one line up
  <li><i> Ctrl+V </i> Paste the clipboard text into line edit (also
  <i>Shift+Insert</i> under Windows)
  <li><i> Ctrl+X </i> Cut the marked text, copy to clipboard (also
  <i>Shift+Delete</i> under Windows)
  <li><i> Ctrl+Z </i> Undo the last operation
  <li><i> Ctrl+Y </i> Redo the last operation
  <li><i> Ctrl+Left Arrow </i> Move the cursor one word left
  <li><i> Ctrl+Right Arrow </i> Move the cursor one word right
  <li><i> Ctrl+Up Arrow </i> Move the cursor one word up
  <li><i> Ctrl+Down Arrow </i> Move the cursor one word down
  <li><i> Ctrl+Home Arrow </i> Move the cursor to the beginning of the text
  <li><i> Ctrl+End Arrow </i> Move the cursor to the end of the text
  </ul>

    To select (mark) text hold down the Shift key whilst pressing one of
    the movement keystrokes, for example, <i>Shift+Right Arrow</i> will
    select the character to the right, and <i>Shift+Ctrl+Right Arrow</i>
    will select the word to the right, etc.

  All other keys with valid ASCII codes insert themselves into the
  text at the cursor position.
*/

/*! \enum QTextEdit::KeyboardAction

  This enum is used by doKeyboardAction() to specify which action
  should be exectuted:

  \value ActionBackspace  Delete the character at the left of the cursor

  \value ActionDelete  Delete the character at the right of the cursor

  \value ActionReturn  Split the paragraph at the cursor position

  \value ActionKill Delete the text until the end of the paragraph, or
  if the cursor is at the end of the paragraph, join this and the next
  paragraph.
*/

/*! \enum QTextEdit::MoveDirection

  This enum is used by moveCursor() to specify into which direction
  the cursor should be moved:

  \value MoveLeft  Moves the cursor to the left

  \value MoveRight  Moves the cursor to the right

  \value MoveUp  Moves the cursor up one line

  \value MoveDown  Moves the cursor down one line

  \value MoveHome  Moves the cursor to the begin of the line

  \value MoveEnd Moves the cursor to the end of the line

  \value MovePgUp  Moves the cursor one page up

  \value MovePgDown  Moves the cursor one page down
*/


/*! \fn void QTextEdit::getCursorPosition( int &parag, int &index ) const

  This functions sets the \a parag and \a index parameters to the
  current cursor position.
 */

/*! \fn bool QTextEdit::isModified() const

  This function returns whether the document has been modified by the
  user.
*/

/*! \fn bool QTextEdit::italic() const

  Returns whether the current format (the position where the cursor is
  placed) is italic or not.
*/

/*! \fn bool QTextEdit::bold() const

  Returns whether the current format (the position where the cursor
  is placed) is bold or not.
*/

/*! \fn bool QTextEdit::underline() const

  Returns whether the current format (the position where the cursor
  is placed) is underlined or not.
*/

/*! \fn QString QTextEdit::family() const

  Returns the font family of the current format (the position where
  the cursor is placed).
*/

/*! \fn int QTextEdit::pointSize() const

  Returns the poit size of the font of the current format (the
  position where the cursor is placed).
*/

/*! \fn QColor QTextEdit::color() const

  Returns the color of the current format (the position where the
  cursor is placed).
*/

/*! \fn QFont QTextEdit::font() const

  Returns the font of the current format (the position where the
  cursor is placed).
*/

/*! \fn int QTextEdit::alignment() const

  Returns the alignment of the paragraph at which the cursor is
  currently placed.
*/

/*! \fn bool QTextEdit::isOverwriteMode() const

  Returns TRUE if this editor is in overwrite mode, i.e.  if
  characters typed replace characters in the editor.
*/

/*! \fn int QTextEdit::undoDepth() const

  Returns how many steps the undo/redo history can maximally store.
*/

/*! \fn void QTextEdit::insert( const QString &text, bool indent, bool checkNewLine, bool removeSelected )

  Inserts \a text at the current cursor position. If \a indent is
  TRUE, the paragraph is re-indented. If \a checkNewLine is TRUE,
  newline characters in \a text result in hard breaks. Otherwise, the
  result of newlines is not defined. If \ removeSelected is TRUE,
  selected text is removed before the text is inserted, else the
  inserted text gets selected.
*/

/*! \fn void QTextEdit::setOverwriteMode( bool b )

  Sets overwrite mode if \a b on is TRUE. Overwrite mode means that
  characters typed replace characters in the editor.
*/

/*! \fn void QTextEdit::undo()

  Undoes the last operation.
*/

/*! \fn void QTextEdit::redo()

  Redoes the last operation.
*/

/*! \fn void QTextEdit::cut()

  Cuts the selected text (if there is any) and puts it on the
  clipboard.
*/

/*! \fn void QTextEdit::paste()

  Pastes the text from the clipboard (if there is any) at the current
  cursor position. Only pastes plain text.

  \sa pasteSubType()
*/

/*! \fn void QTextEdit::pasteSubType( const QCString &subtype )

  Pastes the text from the clipboard (if there is any) of the format
  \a subtype (this can be e.g. "plain", "html" ) at the current cursor
  position.
*/

/*! \fn void QTextEdit::clear()

  Removes all text.

*/

/*! \fn void QTextEdit::del()

  Deletes the character on the right side of the text cursor. If a
  text has been marked by the user (e.g. by clicking and dragging) the
  cursor is put at the beginning of the marked text and the marked
  text is removed.
*/

/*! \fn void QTextEdit::indent()

  Re-indents the current paragraph.
*/

/*! \fn void QTextEdit::setItalic( bool b )

  Sets the current format and the selected text (if there is any) to
  italic, if \a b is TRUE, otherwise unsets the italic flag.
*/

/*! \fn void QTextEdit::setBold( bool b )

  Sets the current format and the selected text (if there is any) to
  bold, if \a b is TRUE, otherwise unsets the bold flag.
*/

/*! \fn void QTextEdit::setUnderline( bool b )

  Sets the current format and the selected text (if there is any) to
  underlined, if \a b is TRUE, otherwise unsets the underlined flag.
*/

/*! \fn void QTextEdit::setFamily( const QString &f )

  Sets the family of the current format and the selected text (if
  there is any) to \a f.
*/

/*! \fn void QTextEdit::setPointSize( int s )

  Sets the point size of the current format and the selected text (if
  there is any) to \a s.

  Note that if \a s is zero or negative, the behaviour of this
  function is not defined.
*/

/*! \fn void QTextEdit::setColor( const QColor &c )

  Sets the color of the current format and the selected text (if there
  is any) to \a c.
*/

/*! \fn void QTextEdit::setFont( const QFont &f )

  Sets the font of the current format and the selected text (if there
  is any) to \a f.
*/

/*! \fn void QTextEdit::setAlignment( int a )

  Sets the alignment of the paragraph, at which the cursor is placed,
  to \a a.
*/

/*! \fn void QTextEdit::setParagType( QStyleSheetItem::DisplayMode dm, QStyleSheetItem::ListStyle listStyle )

  Sets the paragraph style of the paragraph at which the cursor is
  placed to \a dm. If \a dm is QStyleSheetItem::DisplayListItem, the
  type of the list item is set to \a listStyle.
*/

/*! \fn void QTextEdit::setCursorPosition( int parag, int index )

  Sets the cursor to the index \a index in the paragraph \a parag.
*/

/*! \fn void QTextEdit::setSelection( int parag_from, int index_from, int parag_to, int index_to, int selNum )

  Sets a selection which starts at the index \a index_from in the
  paragraph \a parag_from and ends at index \a index_to in the
  paragraph \a parag_to.

  Uses the selection settings of selection \a selNum. If this is 0,
  this is the default selection.
*/

/*! \fn void QTextEdit::setSelectionAttributes( int selNum, const QColor &back, bool invertText )

  Sets the background color of the selection \a selNum to \a back and
  specifies whether the text of this selection should be inverted by \a invertText.

*/

/*! \fn void QTextEdit::setModified( bool m )

  Sets the modified flag of the document to \a m.
*/

/*! \fn void QTextEdit::resetFormat()

  Resets the current format to the default format.
*/

/*! \fn void QTextEdit::setUndoDepth( int d )

  Sets the number of steps the undo/redo history can maximally store
  to \a d.
*/

/*! \fn void QTextEdit::undoAvailable( bool yes )

  This signal is emitted when the availability of undo changes.  If \a
  yes is TRUE, then undo() will work until undoAvailable( FALSE ) is
  next emitted.
*/

/*! \fn void QTextEdit::modificationChanged( bool m )

  This signal is emitted when the modification of the document
  changed. If \a m is TRUE, the document got modified, else the
  modification state has been reset to unmodified.
*/

/*! \fn void QTextEdit::redoAvailable( bool yes )

  This signal is emitted when the availability of redo changes.  If \a
  yes is TRUE, then redo() will work until redoAvailable( FALSE ) is
  next emitted.
*/

/*! \fn void QTextEdit::currentFontChanged( const QFont &f )

  This signal is emitted if the font of the current format (the format
  at the position where the cursor is placed) has changed.

  \a f contains the new font.
*/

/*! \fn void QTextEdit::currentColorChanged( const QColor &c )

  This signal is emitted if the color of the current format (the
  format at the position where the cursor is placed) has changed.

  \a c contains the new color.
*/

/*! \fn void QTextEdit::currentAlignmentChanged( int a )

  This signal is emitted if the alignment of the current paragraph
  (the paragraph at which the cursor is placed) has changed.

  \a a contains the new alignment.
*/

/*! \fn void QTextEdit::cursorPositionChanged( QTextCursor *c )

  This signal is emitted if the position of the cursor changed. \a c
  points to the text cursor object.
*/

/*! \fn void QTextEdit::returnPressed()

  This signal is emitted if the user pressed the RETURN or ENTER key.
*/

/*! \fn void QTextEdit::setFormat( QTextFormat *f, int flags )

  This functions sets the current format and the selected text (if
  there is any) to \a f. Only the fields of \a f which are specified
  by the \a flags are used.
*/

/*! \fn void QTextEdit::ensureCursorVisible()

  Ensures that the cursor is visible by scrolling the view if needed.
*/

/*! \fn void QTextEdit::placeCursor( const QPoint &pos, QTextCursor *c )

  Places the cursor \a c at the character which is closest to \a pos
  (in contents coordinates). If \a c is 0, the default text cursor is
  used.
*/

/*! \fn void QTextEdit::moveCursor( MoveDirection direction, bool shift, bool control )

  Moves the text cursor into the \a direction. As this is normally
  used by some keyevent handler, \a shift and \a control specify the
  state of the key modifiers which have an influence on the cursor
  moving.
*/

/*! \overload void QTextEdit::moveCursor( MoveDirection direction, bool control )
*/

/*! \fn void QTextEdit::removeSelectedText()

  Deletes the text which is currently selected (if there is any
  selected).
*/

/*! \fn void QTextEdit::doKeyboardAction( KeyboardAction action )

  Exectutes the keyboard action \a action. This is normally called by
  a key event handler.
*/

/*! \fn QTextCursor *QTextEdit::textCursor() const

  Returns the text cursor if the editor. QTextCursor is not in the
  public API, but for special cases you might use it anyway. But the
  API of it might change in an incompatible manner in the future.
*/

/*! \fn bool QTextEdit::getFormat( int parag, int index, QFont &font, QColor &color )

  This function sets \a font to the font and \a color to the color of
  the character at position \a index in the paragraph \a parag.

  If \a parag or \a index is out of range, FALSE is returned, else
  TRUE is returned.
*/

/*! Constructs a QTextEdit. The \a parent and \a name arguments are as
  for QWidget. */

QTextEdit::QTextEdit( QWidget *parent, const char *name )
    : QTextView( parent, name )
{
#ifndef QT_NO_CURSOR
    viewport()->setCursor( ibeamCursor );
#endif
}

/*! \reimp */

QTextEdit::~QTextEdit()
{
}

