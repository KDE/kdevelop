/***************************************************************************
                          kdevsession.h  -  description
                             -------------------
    begin                : Mon Mar 26 2001
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
#ifndef _KDEVSESSION_H_
#define _KDEVSESSION_H_

#include <qlist.h>

//to avoid includes
class QextMdiChildView;
class DocViewMan;

//------------------------------------------------------------------------------
/**
   Saving, loading of the KDevelop session
*/
class KDevSession
{
// methods
public:

  KDevSession(DocViewMan* pDocViewMan, const QString& fileName = "");
  ~KDevSession() {};

  /** file I/O functions - opens the file and execute save/load
   */
  bool saveToFile(const QString& fileName);
  bool restoreFromFile(const QString& fileName);

  /** recreates views of a document and their properties */
  void recreateViews( QObject* pDoc, QDomElement docEl);
  /** */
  void saveViewGeometry( QWidget* pView, QDomElement viewEl);
  /** */
  void loadViewGeometry( QWidget* pView, QDomElement viewEl);

//attributes
  /** The modules doc and view manager */
  DocViewMan*              m_pDocViewMan;

private: //to avoid use (not implemented)
  KDevSession(const KDevSession&);
  KDevSession& operator=(const KDevSession&);
};

//------------------------------------------------------------------------------
#endif
