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
  /** Create a new document depending on the requested type.
    * Load the data from file (optionally) and uses the given parameters (optionally).
    * @return the document id (-1 if failed)
    */
  int createDoc(int contentsType, const QString& strFileName = 0L);
  /** Load document from file. */
  void loadDoc(int docId, const QString& strFileName, int mode);
  /** Save document to file. */
  bool saveDoc(int docId, const QString& strFileName);
  /** Close a document, causes all views to be closed. */
  void closeDoc(int docId);

  ///////////////////////////
  // EditInfo stuff
  ///////////////////////////

  /* get the info structure from the filename */
  TEditInfo *getInfoFromFilename(const QString &filename);

  /** Remove a specified file from the edit_infos struct
   *  and leave the widgets in a proper state
   *  @param filename           The filename you want to remove.
   */
  void removeFileFromEditlist(const char *filename);

  /*
    synchronize the "modified"-information of the KWriteDocs 
    with the TEditInfo list
   */
  void synchronizeDocAndInfo();

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
  bool setInfoModified(const QString &sFilename, bool bModified=true);

  bool noInfoModified();

  TEditInfo* findInfo(const QString &sFilename);

  void appendInfo(TEditInfo* info);

  void appendInfoFilenames(QStrList &fileList);

  bool saveFileFromTheCurrentEditWidget();

  void doFileCloseAll();

  bool doProjectClose();

  ///////////////////////////
  // Doc stuff
  ///////////////////////////

  /** Retrieve the document pointer.
    * The returned object is of type QObject since we deal with several document types. */
  QObject* docPointer(int docId) const;

  /** get the ids of all documents of this type or a combination of types */
  QList<int> docs( int type = DocViewMan::Undefined) const;
  /** Get the id of a document displayed by a given view */
  int docOfView(QWidget* pView) const;
  /** Get the type of a document. Returns one of the DocViewMan::ContentsType enum. */
  int docType(int docId) const;

  /** Create a new view of given type for a given document.
    * If no view type is given, default 0 isused.
    * If no doc is given (-1 - default) the view is not connected to a document. */
  QWidget* createView(int docId = -1);
  /** Close a view, automatically disconnects document. */
  void closeView(QWidget* pView);

  /** Get number of views handled by this doc view manager. */
  int countViews() const;

   /** get number of views for a document */
  int countViewsOfDoc(int docId) const;
  /** get all view pointer for a document */
  const QList<QWidget> viewsOfDoc(int docId) const;
  /** get the first edit view for a document */
  CEditWidget* getFirstEditView(int docId) const;

  /** @return the number of documents */
  int docCount() const;
  /** retrieves the document found by its filename */
  int findDoc( const QString& strFileName) const;

  /** Get the currently focused CEditWidget view (Note: not the covering MDI widgets but the embedded view) */
  CEditWidget* currentEditView() { return m_pCurEditView; };
  /** Get the currently focused KHTMLView view (Note: not the covering MDI widgets but the embedded view) */
  KHTMLView* currentBrowserView() { return m_pCurBrowserView; };
  /** Get the document of the currently focused CEditWidget view. */
  KWriteDoc* currentEditDoc() { return m_pCurEditDoc; };
  /** Get the document of the currently focused KHTMLView view. */
  CDocBrowser* currentBrowserDoc() { return m_pCurBrowserDoc; };
  /** Get the type of the currently (because of its view) focused document */
  int currentDocType() { return m_currentDocType; };

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
  void sig_viewGotFocus(QWidget* pView, int docType);
  /** Is emitted when a view handled by the doc view manager looses focus. */
  void sig_viewLostFocus(QWidget* pView, int docType);

   /** is emitted when the las view managed by this instance has been closed */
  void sig_lastViewClosed();
  /** is emitted when the las document managed by this instance has been closed */
  void sig_lastDocClosed();

  /** Is be emitted when data in a doc has changed. */
  void sig_updated(QObject* pDoc, int nChangeFlags);

// attributes
private:
  /** internal struct for list of views belonging to a document */
  typedef struct
  {
    QObject*                pDoc;
    int                     docId;          // make search faster
    int                     docType;        // OR'ed combination of DocViewMan::ContentsType flags
    QList<QWidget>          existingViews;
  } DocViewNode;

  /** list of documents and views belonging to it */
  QList<DocViewNode>            m_docsAndViews;

  /** list of info about edit document 
   (should be merged with the above list) */
  QList<TEditInfo> edit_infos;

  /** List of the focused views, the view at the end is the recently
      focused view */
  QList<QextMdiChildView>   m_MDICoverList;

  CKDevelop*                m_pParent;
  HlManager                 m_highlightManager;

  KWriteDoc*                m_pCurEditDoc;
  CEditWidget*              m_pCurEditView;
  CDocBrowser*              m_pCurBrowserDoc;
  KHTMLView*                m_pCurBrowserView;

  int                       m_currentDocType;
};

#endif //DOCVIEWMAN_H
