/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2003 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2003 Harald Fernengel <harry@kdevelop.org>
   Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>

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
#ifndef __KDEV_PARTCONTROLLER_H__
#define __KDEV_PARTCONTROLLER_H__


#include <kurl.h>
#include <kparts/partmanager.h>

namespace KParts
{
  class ReadOnlyPart;
}

enum DocumentState
{
	Clean,
	Modified,
	Dirty,
	DirtyAndModified
};

class KDevPartController : public KParts::PartManager
{
  Q_OBJECT

public:

  KDevPartController(QWidget *parent);

  /**
   * Call before a call to @ref editDocument to set the encoding of the document to be opened
   * @param encoding the encoding to open as
   */
  virtual void setEncoding(const QString &encoding) = 0;
  
  /**
   * Open a new or existing document
   * @param url the URL of the document to open
   * @param lineNum the line number to place the cursor at, if applicable
   * @param col the column number to place the cursor at, if applicable
   */
  virtual void editDocument(const KURL &url, int lineNum=-1, int col=-1) = 0;
  
  /**
   * Show a HTML document in the documentation viewer
   * @param url the URL of the document to view
   * @param newWin if true, the new window will be created instead of using current
   */
  virtual void showDocument(const KURL &url, bool newWin = false) = 0;
  
  /**
   * Embed a part into the main area of the mainwindow
   * @param part the part to embed
   * @param name the name of the part
   * @param shortDescription currently not used (??)
   */
  virtual void showPart( KParts::Part* part, const QString& name, const QString& shortDescription ) = 0;
  
  /**
   * Find the embedded part corresponding to a given URL
   * @param url the URL of the document
   * @return the corresponding part, 0 if not found.
   */
  virtual KParts::ReadOnlyPart * partForURL( const KURL & url ) = 0;
    
  /**
   * Find the embedded part corresponding to a given main widget
   * @param widget the parts main widget
   * @return the corresponding part, 0 if not found
   */
  virtual KParts::Part * partForWidget( const QWidget * widget ) = 0;
  
  /**
   * Returns a list of open documents
   * @return the list of URLs 
   */
  virtual KURL::List openURLs() = 0;

  /**
   * Save all open files
   */
  virtual void saveAllFiles() = 0;
  
  /**
   * Save a list of files
   * @param list the list of URLs to save
   */
  virtual void saveFiles( const KURL::List & list) = 0;
  
  /**
   * Reload all open files
   */
  virtual void revertAllFiles() = 0;
  
  /**
   * Reload a list of files
   * @param list the list of URLs to reload
   */
  virtual void revertFiles( const KURL::List & list ) = 0;
  
  /**
   * Close all open files
   */
  virtual bool closeAllFiles() = 0;
  
  /**
   * Close a list of files
   * @param list a list of URLs for the files to close
   */
  virtual bool closeFiles( const KURL::List & list ) = 0;
  
  /**
   * Close this part (close the window/tab for this part)
   * @param part the part to close
   * @return 
   */
  virtual bool closePart( KParts::Part * part ) = 0;  
  
  /**
   * Activate this part
   * @param part the part to activate
   */
  virtual void activatePart( KParts::Part * part ) = 0;

  /**
   * Check the state of a document
   * @param url the URL to check 
   * @return the DocumentState enum corresponding to the document state
   */
  virtual DocumentState documentState( KURL const & url ) = 0;

signals:

  /**
   * Emitted when a document has been saved.
   */
  void savedFile( const KURL & );

  /**
   * Emitted when a document has been loaded.
   */
  void loadedFile( const KURL & );

  /**
   * Emitted when a file has been modified outside of KDevelop
   */
  void fileDirty( const KURL & );
  
  /**
   * this is typically emitted when an editorpart does "save as"
   * which will change the part's URL
   */
  void partURLChanged( KParts::ReadOnlyPart * );

  /**
   *  this is emitted when the document changes, 
   *  either internally or on disc
   */
  void documentChangedState( const KURL &, DocumentState );
  

};



#endif
