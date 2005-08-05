/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <qptrlist.h>
#include <qtimer.h>
#include <qwhatsthis.h>

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>

#include <kdevcore.h>

#include "kdevproject.h"
#include "filecreate_part.h"
#include "filecreate_widget2.h"
#include "filecreate_filetype.h"
#include "filecreate_listitem.h"

namespace FileCreate {

  FriendlyWidget::FriendlyWidget(FileCreatePart *part)
    : QTable(0,4,0), TypeChooser(part), m_selected(NULL)
  {

    setReadOnly(true);
    setShowGrid(false);
    horizontalHeader()->hide();
    setTopMargin(0);
    verticalHeader()->hide();
    setLeftMargin(0);
    setSelectionMode(SingleRow);
    setFocusStyle(FollowStyle);
    setColumnStretchable(3, true);

    m_iconLoader = KGlobal::iconLoader();

    QWhatsThis::add(this, i18n("Use this to create new files within your project."));

    setDefaultColumnWidths();

  }


  FriendlyWidget::~FriendlyWidget()
  {
  }

  void FriendlyWidget::setCurrent(const FileType * current) {
    int changeToRow = -1;
    QMap<int,FileType*>::Iterator it;
    kdDebug(9034) << "Checking " << current->descr() << " for matches in row..." << endl;
    for ( it = typeForRow.begin(); it != typeForRow.end() && changeToRow==-1; ++it ) {
      kdDebug(9034) << "Checking: " << it.data()->descr() << endl;
      if (it.data()==current)
	changeToRow=it.key();
      else kdDebug(9034) << "No match!" << endl;
    }

    // If an exact match is not found (e.g. current points to a 'parent' type) then
    // look instead for an extension match
    if (changeToRow==-1) {
      for(it = typeForRow.begin(); it!= typeForRow.end() && changeToRow==-1; ++it) {
	if (it.data()->ext() == current->ext() )
	  changeToRow = it.key();
      }
    }

    if (changeToRow!=-1) {
      m_current = current;
      kdDebug(9034) << "Found row, setting current to row " << changeToRow << endl;
      slotCellSelected(changeToRow,0);
      clearSelection();
      selectRow(changeToRow);
    }

  }

  void FriendlyWidget::refresh() {

    disconnect( this, SIGNAL(currentChanged(int,int)), this, SLOT(slotCellSelected(int,int)) );

    empty();

    int row = 0;
    QPtrList<FileType> filetypes = m_part->getFileTypes();
    for(FileType * filetype = filetypes.first();
	filetype;
	filetype=filetypes.next()) {

      if (filetype->enabled()) {

	if (filetype->subtypes().count()==0)
	  setRow(row++, filetype);

	QPtrList<FileType> subtypes = filetype->subtypes();
	for(FileType * subtype = subtypes.first();
	    subtype;
	    subtype=subtypes.next()) {
	  if (subtype->enabled())
	    setRow(row++, subtype);
	}

      }

    }
    resizeCells();
    if (currentSelection()>-1) removeSelection(currentSelection());

    connect( this, SIGNAL(currentChanged(int,int)), this, SLOT(slotCellSelected(int,int)) );


  }

  void FriendlyWidget::setRow(int row, FileType * filetype) {
    if (row+1>numRows()) setNumRows(row+1);
    setText(row, 1, filetype->name() );
    setText(row, 2, filetype->ext() );
    setText(row, 3, filetype->descr() );
    item(row,1)->setWordWrap(true);
    item(row,3)->setWordWrap(true);
    //setRowStretchable(row,true);
    QPixmap iconPix = m_iconLoader->loadIcon(filetype->icon(), KIcon::Desktop, KIcon::SizeMedium,
					     KIcon::DefaultState, NULL,
					     true);
    if (!iconPix.isNull()) {
      setPixmap(row, 0, iconPix);
      setRowHeight(row, iconPix.height()+4 );
      if (iconPix.width()+4>columnWidth(0))
	setColumnWidth(0, iconPix.width()+4 );
    }

    typeForRow[row]=filetype;

  }

  void FriendlyWidget::empty() {
    typeForRow.clear();
    while(numRows()) removeRow(0);
  }

  void FriendlyWidget::setDefaultColumnWidths() {
    // set some defaults - resizeCells will later ensure that column widths
    // and row heights are set big enough for the cell contents
    setColumnWidth(0,1);
    setColumnWidth(1,60);
    setColumnWidth(2,30);
    setColumnWidth(3,150);
  }

  void FriendlyWidget::slotCellSelected(int row, int col) {
    if (col!=0) {
      setCurrentCell(row, 0);
      return;
    }

    m_selected = typeForRow.contains(row) ? typeForRow[row] : NULL;
    QTimer::singleShot(0, this, SLOT(slotDoSelection()) );

  }

  void FriendlyWidget::slotDoSelection() {
    kdDebug(9034) << "widget2: slotDoSelection" << endl;
    if (m_selected) filetypeSelected(m_selected);
    kdDebug(9034) << "widget2: slotDoSelection middle" << endl;
    if (currentSelection()>-1) removeSelection(currentSelection());
    kdDebug(9034) << "widget2: slotDoSelection ending" << endl;
  }

  void FriendlyWidget::resizeCells() {
    for(int r=0;r<numRows();r++) resizeRow(r);
    for(int c=0;c<numCols();c++) resizeColumn(c);
  }

  void FriendlyWidget::resizeRow(int row) {
    if (row>=numRows() || row<0) return;
    int maxHeight = 0;

    for(int c=0;c<numCols();c++) {
      QTableItem* i = item( row, c );
      if( !i )
         continue;

      QSize size = i->sizeHint();
      maxHeight = size.height()>maxHeight ? size.height() : maxHeight;
    }
    setRowHeight(row,maxHeight+2); // bit of extra room
  }

  void FriendlyWidget::resizeColumn(int col) {
    if (col>=numCols() || col<0) return;
    int maxWidth = 0;
    for(int r=0;r<numRows();r++) {

      QTableItem* i = item( r, col );
      if( !i )
         continue;

      QSize size = item(r,col)->sizeHint();
      maxWidth = size.width()>maxWidth ? size.width() : maxWidth;
    }
    setColumnWidth(col,maxWidth+2); // bit of extra room
  }

#if QT_VERSION < 0x030100
  void FriendlyWidget::selectRow(int row) {
    if (numCols()>0 && row<numRows()) {
      QTableSelection sel;
      sel.init(row,0);
      sel.expandTo(row,numCols());
      addSelection(sel);
    }
  }
#endif

}

#include "filecreate_widget2.moc"
