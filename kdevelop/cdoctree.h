/***************************************************************************
                    cdoctree.h -
                             -------------------                                         

    begin                : 3 Oct 1998                                        
    copyright            : (C) 1998 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
#ifndef CDOCTREE_H
#define CDOCTREE_H

#include <qwidget.h>
#include <kiconloader.h>
#include <kapp.h>
#include "cproject.h"
#include <kpopmenu.h>
#include "ctreeview.h"


/** the documentation tree (manual,tutorial,KDE-libs...)
  *@author Sandy Meier
  */

class CDocTree : public CTreeView 
{
  Q_OBJECT 
public: 
  /** construtor */
  CDocTree(QWidget*parent=0,const char* name=0, KConfig* config=0); 
  /** destructor */
  ~CDocTree();

  /** starts the refresh */
  void refresh(CProject* prj);
  QString selectedText(){return m_text;}

protected: // Implementations of virtual methods.

  /** Initialize popupmenus. */
  void initPopups();

  /** Get the current popupmenu. */
  KPopupMenu *getCurrentPopup();

 protected slots:
  void slotAddDocumentation();
  void slotRemoveDocumentation();
  void slotDocumentationProp();
  void slotSelectionChanged(QListViewItem* item);

 signals:
  void fileSelected(QString url_file);

private: // Popupmenus
  KPopupMenu others_pop;
  KPopupMenu doc_pop;

private: // Private attributes
  KConfig* config_kdevelop;
  CProject* project;
  QString m_text;
};
#endif


