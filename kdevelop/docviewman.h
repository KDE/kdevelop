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
#include "ceditwidget.h"
#include "qextmdichildview.h"
#include "highlight.h"

class KWriteDoc;
class CKDevelop;

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

  /** Retrieve the document pointer.
    * The returned object is of type QObject since we deal with several document types. */
  QObject* docPointer(int docId) const;

  /** Get number of documents */
  int countDocs() const;
  /** Get the ids of all documents */
  QList<int> docs() const;
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

  /** @return the number of documents */
  int docCount() const;
  /** retrieves the document found by its filename */
  int findDoc( const QString& strFileName) const;

public slots:
  /** */
  void slot_gotFocus(QWidget* pView);

signals:
  /** Is emitted when a view handled by the doc view manager receives focus. */
  void sig_viewGotFocus(QWidget* pView);
  /** Is emitted when a view handled by the doc view manager looses focus. */
  void sig_viewLostFocus(QWidget* pView);

   /** is emitted when the las view managed by this instance has been closed */
  void sig_lastViewClosed();
  /** is emitted when the las document managed by this instance has been closed */
  void sig_lastDocClosed();

  /** Is be emitted when data in a doc has changed. */
  void sig_updated(QObject* pDoc, int nChangeFlags);

// attributes
protected:
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

  /** List of the focused views, the view at the end is the recently
      focused view */
  QList<QextMdiChildView>   m_MDICoverList;

  CKDevelop*                m_parent;
  HlManager                 m_highlightManager;
};

#endif //DOCVIEWMAN_H
