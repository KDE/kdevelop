/***************************************************************************
  docviewman.h  -  MDI manager for
                   document classes of KDevelop (KWriteDocs, CDocBrowser)
                   and view classes of KDevelop (CEditWidget, KHTMLView)
                             -------------------

    begin                : 03 Mar 2001
    copyright            : (C) 2001 by Falk Brettschneider
    email                : falk.brettschneider@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DOCVIEWMAN_H
#define DOCVIEWMAN_H

#include <qobject.h>
#include <qlist.h>
#include <qmap.h>

//------------------------------------------------------------------------------
#include "qextmdichildview.h"
#include "highlight.h"
#include "structdef.h"      // needed for TEditInfo

class KWriteDoc;
class CEditWidget;
class CKDevelop;
class KHTMLView;
class CDocBrowser;

//==============================================================================
// class DocViewMan
//------------------------------------------------------------------------------
/** 
  MDI manager for document classes of KDevelop (KWriteDocs, CDocBrowser)
  and view classes of KDevelop (CEditWidget, KHTMLView).
*/
class DocViewMan : public QObject
{
  Q_OBJECT

public: // enumerations
  enum ContentsType {  // as flags to be able to combine them
     Undefined      = 0x00  // null element of this set
    ,Header         = 0x01
    ,Source         = 0x02
    ,HTML           = 0x04
  };

public:
  DocViewMan( CKDevelop* parent);
  ~DocViewMan();

  /** */
  void doSwitchToFile(QString filename, int line, int col,
                     bool bForceReload, bool bShowModifiedBox);
  /** */
  void doSelectURL(const QString& url);

  /** */
  void doCopy();

  /** */
  void doSearch();

  /** */
  void doRepeatSearch(QString &search_text, int back);

  /** */
  void doSearchText(QString &text);

  /** */
  void doClearBookmarks();

  /** */
  void doOptionsEditor();
  /** */
  void doOptionsEditorColors();  
  /** */
  void doOptionsSyntaxHighlightingDefaults();
  /** */  
  void doOptionsSyntaxHighlighting();

  /** */  
  void doTakeOverOfEditorOptions();

  /** */
  CDocBrowser* createCDocBrowser(const QString& url);
  /** */
  KWriteDoc* createKWriteDoc(const QString& strFileName);

  /** */
  ProjectFileType getKWriteDocType(KWriteDoc* pDoc);
  /** */
  bool curDocIsBrowser() { return m_curIsBrowser; };
  /** */
  bool curDocIsHeaderFile();
  /** */
  bool curDocIsCppFile();

  /** Create a new document depending on the requested type.
    * Load the data from file (optionally) and uses the given parameters (optionally).
    * @return the document id (-1 if failed)
    */
  //  QObject* createDoc(int contentsType, const QString& strFileName = 0L);

  /** Load edit document from file. */
  void loadKWriteDoc(KWriteDoc* pDoc, 
                     const QString& strFileName, 
                     int /*mode*/);

  /** */
  bool saveKWriteDoc(KWriteDoc* pDoc, const QString& strFileName);

  /** Load document from file. */
  void loadDoc(int docId, const QString& strFileName, int mode);
  /** Save document to file. */
  bool saveDoc(int docId, const QString& strFileName);


  /** Close a document, causes all views to be closed. */
  // void closeDoc(int docId);

  void closeCDocBrowser(CDocBrowser* pDoc);
  void closeKWriteDoc(KWriteDoc* pDoc);



  ///////////////////////////
  // EditInfo stuff
  ///////////////////////////

  /* get the info structure from the filename */
  // TEditInfo* getInfoFromFilename(const QString &filename);

  /** Remove a specified file from the edit_infos struct
   *  and leave the widgets in a proper state
   *  @param filename           The filename you want to remove.
   */
  // void removeFileFromEditlist(const char *filename);

  /*
    synchronize the "modified"-information of the KWriteDocs 
    with the TEditInfo list
   */
  // void synchronizeDocAndInfo();

  /*
    get the modified files and ask if they should be saved
   */
  void saveModifiedFiles();

  /*
    force reload for the modified file
   */
  void reloadModifiedFiles();

  /** syncs modified-flag in edit_info-structs
   *
   *  @param sFilename   the filename you want to set
   *  @param bModified   changing edit_info-elment to this value
   */
  // bool setInfoModified(const QString &sFilename, bool bModified=true);

  bool noInfoModified();

  // TEditInfo* findInfo(const QString &sFilename);

  // void appendInfo(TEditInfo* info);

  void appendInfoFilenames(QStrList &fileList);

  bool saveFileFromTheCurrentEditWidget();

  void doFileSave(bool project);
  void doFileCloseAll();

  bool doProjectClose();
  void doCloseAllDocs();

  ///////////////////////////
  // Doc stuff
  ///////////////////////////

  // New interface
  /** Get a list of all the KWriteDoc */
  QList<KWriteDoc> getKWriteDocList() const;
  /** */
  QObject* getDocFromFilename(const QString& strFileName) const;
  /** */
  KWriteDoc* getKWDocFromFilename(const QString& strFileName) const;
  /** */
  QString docName(QObject* doc) const;

  /** Retrieve the document pointer.
    * The returned object is of type QObject since we deal with several document types. */
  // QObject* docPointer(int docId) const;
  /** Retrieve the document pointer and make sure that it is a KWriteDoc */
  // KWriteDoc* kwDocPointer(int docId) const;

  /** get the ids of all documents of this type or a combination of types */
  // QList<int> docs( int type = DocViewMan::Undefined) const;
  /** Get the id of a document displayed by a given view */
  // int docOfView(QWidget* pView) const;
  /** Get the type of a document. Returns one of the DocViewMan::ContentsType enum. */
  // int docType(int docId) const;


  /** */
  void addQExtMDIFrame(QWidget* pNewView);

  /** */
  void doCreateNewView();
  /** */
  CEditWidget* createEditView(KWriteDoc* pDoc);
  /** */
  KHTMLView* createBrowserView(CDocBrowser* pDoc);
  /** get the first edit view for an edit document */
  CEditWidget* getFirstEditView(KWriteDoc* pDoc) const;

  /** Create a new view of given type for a given document.
    * If no view type is given, default 0 isused.
    * If no doc is given (-1 - default) the view is not connected to a document. */
  // QWidget* createView(int docId = -1);
  /** Close a view, automatically disconnects document. */
  void closeView(QWidget* pView);

  /** */
  void closeEditView(CEditWidget* pView);
  /** */
  void closeBrowserView(KHTMLView* pView);

  /** Get number of views handled by this doc view manager. */
  // int countViews() const;

   /** get number of views for a document */
  // int countViewsOfDoc(int docId) const;
  /** get all view pointer for a document */
  // const QList<QWidget> viewsOfDoc(int docId) const;

  /** @return the number of documents */
  int docCount() const;
  /** retrieves the document found by its filename */
  // int findDoc(const QString& strFileName) const;
  /** retrieves the KWriteDoc found by its filename */
  KWriteDoc* findKWriteDoc(const QString& strFileName) const;
  /** */
  KWriteDoc* findKWriteDoc();
  /** */
  CDocBrowser* findCDocBrowser();

  /** Get the currently focused CEditWidget view (Note: not the covering MDI widgets but the embedded view) */
  CEditWidget* currentEditView() { return m_pCurEditView; };
  /** Get the currently focused KHTMLView view (Note: not the covering MDI widgets but the embedded view) */
  KHTMLView* currentBrowserView() { return m_pCurBrowserView; };
  /** Get the document of the currently focused CEditWidget view. */
  KWriteDoc* currentEditDoc() { return m_pCurEditDoc; };
  /** Get the document of the currently focused KHTMLView view. */
  CDocBrowser* currentBrowserDoc() { return m_pCurBrowserDoc; };

  /** Get the type of the currently (because of its view) focused document */
  //  int currentDocType() { return m_currentDocType; };

  /** Install the new bookmark popup */
  void installBMPopup(QPopupMenu *p);

public slots:
  /** Is called whenever the MDI view gets focus. The update of pointers for the "current..."-methods is made here */
  void slot_gotFocus(QextMdiChildView* pMDICover);
  /** Updates the bookmarks for each document */
  void updateBMPopup();
  /** shows the desired bookmark (eventually, switches to file and activates it) */
  void gotoBookmark(int n);
  
signals:
  /** Is emitted when a view handled by the doc view manager receives focus. */
  void sig_viewGotFocus(QWidget* pView);
  /** Is emitted when a view handled by the doc view manager looses focus. */
  void sig_viewLostFocus(QWidget* pView);

   /** is emitted when the las view managed by this instance has been closed */
  void sig_lastViewClosed();
  /** is emitted when the las document managed by this instance has been closed */
  void sig_lastDocClosed();

// attributes
private:

  // New interface
  /** List of all documents */
  QList<QObject> m_documentList;

  /** List of the focused views, the view at the end is the recently
      focused view */
  QList<QextMdiChildView>   m_MDICoverList;

  CKDevelop*                m_pParent;
  HlManager                 m_highlightManager;

  KWriteDoc*                m_pCurEditDoc;
  CEditWidget*              m_pCurEditView;
  CDocBrowser*              m_pCurBrowserDoc;
  KHTMLView*                m_pCurBrowserView;

  bool                      m_curIsBrowser;
};

#endif //DOCVIEWMAN_H

