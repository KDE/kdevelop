/***************************************************************************
                          editormgr.h  -  description
                             -------------------
    copyright            : (C) 2000 by KDevelop team
    email                : kdevelop_team@kdevelop.org

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef EDITORMGR_H
#define EDITORMGR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

class KDevEvent;
class KURL;

class KLibFactory;
class KDialogBase;
class KHTMLPart;

namespace KTextEditor {
  class Document;
};

#include "kdeveditormanager.h"
#include "KDevEditorManagerIface.h"

#include <qwidget.h>

class EditorManager : public KDevEditorManager
{
  Q_OBJECT

public:

  /** Constructor for the fileclass of the application */
  EditorManager(QObject* parent = 0, const char* name=0);

  /** Destructor for the fileclass of the application */
  ~EditorManager();

//  void slotEventHandler(KDevEvent* event);

  /** initializes the document generally */
  KTextEditor::Document* newDocument();

  /** loads the document by filename and format and emits the updateViews() signal */
  KTextEditor::Document* openDocument(const KURL& url);

  /** closes the acutal document */
  void closeDocument(const KURL& url);

  /** saves the document under filename and format.*/	
  bool saveDocument(const KURL& url);

  virtual void gotoSourceFile(const KURL& url, int lineNum);
  virtual void gotoDocumentationFile(const KURL& url);

public slots:	
  /** add a opened file to the recent file list and update recent file menu*/
//    void addRecentFile(const QString &file);
  /** clears the document in the actual view to reuse it as the new document */
  void slotFileNew();
  /** open a file and load it into the document*/
  void slotFileOpen();
  /** save a document */
  void slotFileSave();
  /** save a document by a new filename*/
  void slotFileSaveAs();
  /** asks for saving if the file is modified, then closes the actual file and window*/
  void slotFileClose();
  /** print the actual file */
  void slotFilePrint();
	/** closes all documents and quits the application.*/
  void slotFileQuit();
	/** reverts the last user action for the active window */
  void slotEditUndo();
  /** put the marked text/object into the clipboard and remove
   *	it from the document
   */
  void slotEditCut();
  /** put the marked text/object into the clipboard
   */
  void slotEditCopy();
  /** paste the clipboard into the document
   */
  void slotEditPaste();

  void slotEditRedo();
  void slotEditFind();
  void slotEditFindNext();
  void slotEditReplace();

	void slotConfigWidgetRequested(KDialogBase*);

protected:
	void setupGUI();
	
private:


  KTextEditor::Document* findOpenDocument(const KURL& url);
  KHTMLPart* findOpenHTMLViewer(const KURL& url);

  bool documentExists(const KURL& url);

  bool documentIsWritable(const KURL& url);

private:
  QList<KTextEditor::Document> m_documents;
  QList<KHTMLPart> m_htmldocuments;
  KLibFactory* m_factory;
  KDevEditorManagerIface *m_dcop;
};

#endif
