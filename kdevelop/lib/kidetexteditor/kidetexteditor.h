/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __ktexteditor_h__
#define __ktexteditor_h__

#include <kparts/part.h>
#include <qwidget.h>

/**
 * This is the kparts interface classes for text editors.
 * The text editors use the Document/View model that allows multiple views into
 * one file (Document)
 *
 * Line numbers passed via this interface must begin at line number zero (0).
 *
 * TODO: See documentation at http://??????? for information on how to use kparts in
 * general. A simple (but sufficient) implementation is shown below.
 *
 * <pre>
 * QHBoxLayout *layout = new QHBoxLayout(this);
 * QSplitter *fixme = new QSplitter(this);
 *
 * KTrader::OfferList offers = KTrader::self()->query( "KTextEditor/Document" );
 * assert( offers.count() >= 1 );
 * KService::Ptr service = *offers.begin();
 * KLibFactory *factory = KLibLoader::self()->factory( service->library() );
 * assert( factory );
 * m_part = static_cast<KTextEditor::Document *>(
 * 						factory->create( this, 0, "KTextEditor::Document" ) );
 * assert( m_part );
 * m_part->createView( fixme, 0 );
 * layout->addWidget(fixme);
 * m_part->setText( "" );
 * </pre>
 *
 * You may also be able to use a dynamic_cast for the document part above
 * (depending on compliation of the library used)
 *
 */

namespace KTextEditor
{

class Document;

/**
 * The View class encapsulates a single view into the document.
 */

class View : public QWidget, public KXMLGUIClient
{
  Q_OBJECT
		
public:
  /**
   * Create a new view to the given document. The document must be non-null.
   */
  View( Document *doc, QWidget *parent, const char *name = 0 );
  virtual ~View();

  /**
   * Acessor to the parent Document.
   */
  virtual Document *document() const; // XXX fix when renaming KXMLGUIClient::document

  virtual void setCursorPosition( int line, int col, bool mark = false ) = 0;
  virtual void getCursorPosition( int *line, int *col ) = 0;

  /**
   *  Inserts text at the current cursor position into the document
   */
  virtual void insertText( const QString &text, bool mark = false );

  /**
   * Overwrite mode is where the char under the cursor is replaced with the
   * char typed by the user
   */
  virtual bool isOverwriteMode() const = 0;

  /**
   * You should reimplement this method.
   * If the internal popupmenu property is enabled, then the implementation
   * is free to handle/use/implement/show a context popupmenu ( see also
   * KContextMenuManager class in kdeui ). If disabled, then the
   * implementation should emit the @ref contextPopupMenu signal.
   */
  virtual void setInternalContextMenuEnabled( bool b );
  virtual bool internalContextMenuEnabled() const;

public slots:
  virtual void setOverwriteMode( bool b ) = 0;

signals:
  /**
   * Connect here when you want to implement a custom popup menu.
   */
  void contextPopupMenu( const QPoint &p );

  /**
   * Connect here if you want to track the scrolling within the editor. This
   * allows you to add specialised borders that displays extra data about
   * particular lines such as breakpoints etc.
   */
  void scrollValueChanged( int value );
  
private:
  class ViewPrivate;
  ViewPrivate *d;
};

class Document : public KParts::ReadWritePart
{
  Q_OBJECT
public:
  Document( QObject *parent = 0, const char *name = 0 );
  virtual ~Document();

  /**
   * Create a view that will display the document data. You can create as many
   * views as you like. When the user modifies data in one view then all other
   * views will be updated as well.
   */
  virtual View *createView( QWidget *parent, const char *name = 0 ) = 0;

  /*
   * Accessor to the list of views.
   */
  virtual QList<View> views() const;

  /**
   * @return All the text from the requested line.
   */
  virtual QString textLine( int line ) const = 0;

  virtual void setSelection( int row_from, int col_from, int row_to, int col_t ) = 0;
  virtual bool hasSelection() const = 0;
  virtual QString selection() const = 0;

  /**
   * @return The current number of lines in the document
   */
  virtual int numLines() const = 0;

  /**
   * Insert line(s) at the given line number. If the line number is -1
   * (the default) then the line is added to end of the document
   */
  virtual void insertLine( const QString &s, int line = -1 ) = 0;

  /**
   * Add the line(s) into the document at the given line and column.
   */
  virtual void insertAt( const QString &s, int line, int col, bool mark = FALSE ) = 0;

  virtual void removeLine( int line ) = 0;

  /**
   * @return the complete document as a single QString
   */
  virtual QString text() const = 0;

  /**
   * @return the number of characters in the document
   */
  virtual int length() const = 0;

public slots:
  /**
   * Set the given text into the view.
   * Warning: This will overwrite any data currently held in this view.
   */
  virtual void setText( const QString &t ) = 0;

signals:

  /**
   * When the user changes the text then this signal is emitted
   * TODO: - explain why and what type of change trigger this?
   */
  void textChanged();

  /**
   */
  void deleted( int startLine, int endLine );
  
  /**
   */
  void inserted( int startLine, int endLine );

protected:
  /**
   * Call this method in your document implementation whenever you created a new
   * view.
   * (like in @ref createView )
   */
  virtual void addView( View *view );

  /**
   * Call this method in your document implementation whenever you delete a view.
   */
  virtual void removeView( View *view );

  QList<View> m_views;

private slots:
		
  /**
   * The view emits a destroyed() signal which is connected to this slot
   * and removed from our internal list. Note: The view* is obtained from
   * the QObject::sender() method.
   */
  void slotViewDestroyed();

private:
  class DocumentPrivate;
  DocumentPrivate *d;
};

};

#endif
