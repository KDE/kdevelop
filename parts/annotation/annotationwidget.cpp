/***************************************************************************
 *   Copyright (C) 2005 by Mathieu Chouinard                               *
 *   mchoui@e-c.qc.ca                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "annotationwidget.h"

#include <kurl.h>
#include <klibloader.h>
#include <kparts/part.h>
#include <kdebug.h>
#include <klocale.h>
#include <qlayout.h>
#include <kpushbutton.h>
#include <klistview.h>
#include <qheader.h>
#include <klineedit.h>
#include <ktextedit.h>
#include <kmessagebox.h>
#include <qsplitter.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/document.h>
#include <kconfig.h>
#include <qtooltip.h>
#include <kpopupmenu.h>
#include <qregexp.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qdragobject.h>
#include <qtimer.h>
#include <kcombobox.h>

#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevpartcontroller.h>

#include "annotationpart.h"
#include "annotationitem.h"

annotationWidget::annotationWidget(annotationPart *part)
    : KListView(0, "annotation widget"), m_part(part)
{
  m_list.setAutoDelete(TRUE);  
  setSorting( -1 );
  addColumn( "" );
  setFullWidth(true);
  header()->hide();
  setAcceptDrops(true);
  setDragEnabled(false);
  setDropVisualizer(false);
  setRootIsDecorated(true);
  annotationItem* itemRoot=new annotationItem(this,"Root","root");
  m_list.append(itemRoot);
  
  connect( this, SIGNAL( contextMenuRequested ( QListViewItem *, const QPoint & , int ) ),
	     this, SLOT( AnnotshowPopup(QListViewItem *, const QPoint & , int ) ) );
}

annotationWidget::~annotationWidget()
{
}
/*void maybeTip( const QPoint & )
{

} */



/*!
    \fn annotationWidget::AnnotShowPopup(QListViewItem* item, const QPoint &p,int))
 */
void annotationWidget::AnnotShowPopup(QListViewItem* item, const QPoint &p,int)
{
  KPopupMenu popup;

  annotationItem * selectedItem = static_cast<annotationItem *>(item);
  if ( item ) {
    popup.insertTitle( selectedItem->getName() );

    popup.insertItem( i18n("Add Item..."), this, SLOT( slotAdd() ) );
    popup.insertItem( i18n("Edit..."), this, SLOT( slotEdit() ) );
    popup.insertItem( i18n("Remove"), this, SLOT( slotRemove() ) );

  } else {
    popup.insertTitle(i18n("Annotations"));

    popup.insertItem( i18n("Add Annotation..."), this, SLOT( slotAdd() ) );
  }

  popup.exec(p);
  
}
# include "annotationwidget.moc"