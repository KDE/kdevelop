/***************************************************************************
  docviewman.h  -  MDI manager for
                   document classes of KDevelop (Kate::Documents, CDocBrowser)
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

//-----------------------------------------------------------------------------
#include "qextmdichildview.h"
//#include "highlight.h"
#include "cproject.h"

namespace KParts { class Factory; }
namespace Kate { class Document;
                 class View; }
class CKDevelop;
class KHTMLView;
class CDocBrowser;
class CKDevAccel;
class ClassStore;
class KDevCodeCompletion;
class KDevCodeTemplate;
class KPopupMenu;
class KConfig;

//=============================================================================
// class DocViewMan
//-----------------------------------------------------------------------------
/**
  MDI manager for document classes of KDevelop (Kate::Documents, CDocBrowser)
  and view classes of KDevelop (CEditWidget, KHTMLView).
*/
class DocViewMan : public QObject
{
  Q_OBJECT

public:
  DocViewMan( CKDevelop* parent );
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
  void doOptionsEditor();
  /** */
  //void doOptionsEditorColors();
  /** */
  //void doOptionsSyntaxHighlightingDefaults();
  /** */
  //void doOptionsSyntaxHighlighting();

  /** */
  void doTakeOverOfEditorOptions(Kate::View* pView = 0L);

  /** Get the modified files and ask if they should be saved */
  void saveModifiedFiles();

  /** Force reload for the modified file */
  void reloadModifiedFiles();

  /** */
  int checkAndSaveFileOfCurrentEditView(bool bDoModifiedInsideCheck = true, Kate::View* pCurEditView = 0L);

  /** */
  bool doFileClose();
  /** */
  void doFileCloseAll();
  /** */
  bool doProjectClose();
  /** */
  void doCloseAllDocs();

  ///////////////////////////
  // Bookmark stuff
  ///////////////////////////

  /** */
  void doBookmarksToggle();

  /** */
  void doBookmarksClear();

  /** */
  void doBookmarksNext();

  /** */
  void doBookmarksPrevious();

  /** */
  void doClearBookmarks();

  /** set the KPopupMenu pointer for the doc bookmark popup */
  void setDocBMPopup(KPopupMenu *p);

  /** */
  void connectBMPopup();

  /** */
  void readBookmarkConfig(KConfig* theConfig);
  /** */
  void writeBookmarkConfig(KConfig* theConfig);

  /** */
  QString getBrowserMenuItem(int index);

  ///////////////////////////
  // Doc stuff
  ///////////////////////////

  /** Get the document of the currently focused CEditWidget view. */
  Kate::Document* currentEditDoc() { return m_pCurEditDoc; };
  /** Get the document of the currently focused KHTMLView view. */
  CDocBrowser* currentBrowserDoc() { return m_pCurBrowserDoc; };

  /** */
  bool curDocIsBrowser() { return m_curIsBrowser; };
  /** */
  bool curDocIsHeaderFile();
  /** */
  bool curDocIsCppFile();

  /** */
  ProjectFileType getKWriteDocType(Kate::Document* pDoc);

  /** Get a list of all the Kate::Document */
  QList<Kate::Document> getKWriteDocList() const;
  /** Get a list of all the DocBrowser */
  QList<CDocBrowser> getDocBrowserList() const;

  /** Retrieves the document found by its filename */
  QObject* findDocFromFilename(const QString& strFileName) const;
  /** Retrieves the Kate::Document found by its filename */
  Kate::Document* findKWriteDoc(const QString& strFileName) const;
  /** Find if there is another Kate::Document in the doc list */
  Kate::Document* findKWriteDoc();
  /** Find if there is another CDocBrowser in the doc list */
  CDocBrowser* findCDocBrowser();
  /** Get the name of a document */
  QString docName(QObject* doc) const;
  /** Find out if no document has been modified */
  bool noDocModified();

  /** Close a browser document, causes all views to be closed. */
  void closeCDocBrowser(CDocBrowser* pDoc);
  /** Close an editor document, causes all views to be closed. */
  void closeKWriteDoc(Kate::Document* pDoc);

  /** */
  CDocBrowser* createCDocBrowser(const QString& url);
  /** */
  Kate::Document* createKWriteDoc(const QString& strFileName);

// not called anymore (rokrau 6/25/01)
//  /** Load edit document from file. */
//  void loadKWriteDoc(Kate::Document* pDoc,
//                     const QString& strFileName,
//                     int /*mode*/);

// not called anymore (rokrau 6/25/01)
  /** Save edit document in a file */
  //bool saveKWriteDoc(Kate::Document* pDoc, const QString& strFileName);

  /** @return the number of documents */
  int docCount() const;

  ///////////////////////////
  // View stuff
  ///////////////////////////

  /** Get the currently focused CEditWidget view
      (Note: not the covering MDI widgets but the embedded view) */
  Kate::View* currentEditView() { return m_pCurEditView; };
  /** Get the currently focused KHTMLView view
      (Note: not the covering MDI widgets but the embedded view) */
  KHTMLView* currentBrowserView() { return m_pCurBrowserView; };

  /** Add a MDI frame around a newly created view */
  void addQExtMDIFrame(QWidget* pNewView, bool bShow, const QPixmap& icon);

  /** */
  void doCreateNewView();
  /** */
  Kate::View* createEditView(Kate::Document* pDoc, bool bShow);
  /** */
  KHTMLView* createBrowserView(CDocBrowser* pDoc, bool bShow);
  /** get the first edit view for an edit document */
  Kate::View* getFirstEditView(Kate::Document* pDoc) const;

  /** Close a view, automatically disconnects document. */
  bool closeView(QWidget* pView);

  /** */
  void closeEditView(Kate::View* pView);
  /** */
  void closeBrowserView(KHTMLView* pView);

  /** Get number of views handled by this doc view manager. */
  // int countViews() const;

   /** get number of views for a document */
  // int countViewsOfDoc(int docId) const;
  /** get all view pointer for a document */
  // const QList<QWidget> viewsOfDoc(int docId) const;

  /**
   * This method can be called from various places.
   * CKDevelop calls it in its constructor, DocViewMan calls it from addQExtMDIFrame(..).
   * Anyway, it sets the application accelerators for any _toplevel_ window of KDevelop. */
  void initKeyAccel( CKDevAccel* accel, QWidget* pTopLevelWidget);

public slots:
  /** Is called whenever the MDI view has been activated.
   * The update of pointers for the "current..."-methods is made here */
  void slot_viewActivated(QextMdiChildView* pMDICover);
  /** Helper method for initKeyAccel(CKDevAccel* accel, QWidget* pTopLevelWidget), acts as slot entry with
   *  the right interface for signals from the QextMDI mainframe class. */
  void initKeyAccel( QWidget* pTopLevelWidget);
  /**
   * Called when a file is removed via RMB in the LFV or RFV. */
  void slotRemoveFileFromEditlist(const QString &absFilename);

  /** Updates the bookmarks for each editor document */
  void updateCodeBMPopup();
  /** shows the desired editor bookmark
   * (eventually, switches to file and activates it) */
  void gotoCodeBookmark(int n);
  /** shows the desired document bookmark
   * (eventually, switches to file and activates it) */
  void gotoDocBookmark(int n);
  /** The last view has been closed, set the mainwidget caption to default */
  void slotResetMainFrmCaption();

  // ***************************
  // Edit slots
  // ***************************

  /** swich construction for the toolbar icons, selecting the right slots */
  void slotToolbarClicked(int);

  /** Undo last editing step */
  void slotEditUndo();
  /** Redo last editing step */
  void slotEditRedo();
  /** cuts a selection to the clipboard */
  void slotEditCut();
  /** copies a selection to the clipboard */
  void slotEditCopy();
  /** inserts the clipboard contents to the cursor position */
  void slotEditPaste();
  /** inserts a file at the cursor position */
  void slotEditInsertFile();
  /** opens the search dialog for the editing widget */
  void slotEditSearch();
  /** repeat last search */
  void slotEditRepeatSearch(int back=0);
  /** repeat last search backwards*/
  void slotEditRepeatSearchBack();
  /** selects the whole editing widget text */
  void slotEditSelectAll();
  /** inverts the selection */
  void slotEditInvertSelection();
  /** remove all text selections */
  void slotEditDeselectAll();

    /** expand text */
    void slotEditExpandText();
    /** code completetion */
    void slotEditCompleteText();
    /** expand template */
    void slotEditExpandTemplate();

  // view activation by accel
  void activateView1();
  void activateView2();
  void activateView3();
  void activateView4();
  void activateView5();
  void activateView6();
  void activateView7();
  void activateView8();
  void activateView9();
  void activateView10();

signals:
  /** Is emitted when a view handled by the doc view manager receives focus. */
  void sig_viewActivated(QWidget* pView);
  /** Is emitted when a view handled by the doc view manager looses focus. */
  void sig_viewLostFocus(QWidget* pView);

  /** Is emitted when the last view managed by this instance
      has been closed */
  void sig_lastViewClosed();
  /** Is emitted when the last document managed by this instance
      has been closed */
  void sig_lastDocClosed();

  /** Is emitted when the status message should be changed */
  void sig_newStatus(const QString& text);

// attributes
private:

  /** List of all documents */
  QList<QObject>            m_documentList;

  /** List of the focused views, the view at the end is the recently
      focused view */
  QList<QextMdiChildView>   m_MDICoverList;

  CKDevelop*                m_pParent;
  //HlManager                 m_highlightManager;

  QStrList                  m_docBookmarksList;
  QStrList                  m_docBookmarksTitleList;
  KPopupMenu*               m_pDocBookmarksMenu;

  Kate::Document*           m_pCurEditDoc;
  Kate::View*               m_pCurEditView;
  CDocBrowser*              m_pCurBrowserDoc;
  KHTMLView*                m_pCurBrowserView;

  bool                      m_curIsBrowser;
  bool                      m_bCloseActionPending;


  KParts::Factory*          m_pKateFactory;
  KDevCodeCompletion*       m_pCodeCompletion;
  KDevCodeTemplate*         m_pCodeTemplate;
};

#endif //DOCVIEWMAN_H

