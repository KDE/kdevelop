#include <qptrlist.h>
#include <qtimer.h>
#include <qwhatsthis.h>

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <kurl.h>
#include <kiconloader.h>
#include <klocale.h>

#include <kdevcore.h>

#include "kdevproject.h"
#include "filecreate_part.h"
#include "filecreate_widget2.h"
#include "filecreate_filetype.h"
#include "filecreate_listitem.h"

#include <iostream>

FileCreateWidget2::FileCreateWidget2(FileCreatePart *part)
  : QTable(1,4,0), FileCreateTypeChooser(part), m_selected(NULL)
{
  
  setReadOnly(true);
  setShowGrid(false);
  horizontalHeader()->hide();
  setTopMargin(0);
  verticalHeader()->hide();
  setLeftMargin(0);
  setSelectionMode(SingleRow);
  setFocusStyle(FollowStyle);

  m_iconLoader = KGlobal::iconLoader();

  QWhatsThis::add(this, i18n("Use this to create new files within your project."));

  setDefaultColumnWidths();
  
}


FileCreateWidget2::~FileCreateWidget2()
{
}

void FileCreateWidget2::refresh() {

  disconnect( this, SIGNAL(currentChanged(int,int)), this, SLOT(slotCellSelected(int,int)) );

  empty();

  int row = 0;
  QPtrList<FileCreateFileType> filetypes = m_part->getFileTypes();
  for(FileCreateFileType * filetype = filetypes.first();
      filetype;
      filetype=filetypes.next()) {
    
    if (filetype->enabled()) {
      
      if (filetype->subtypes().count()==0)
        setRow(row++, filetype);

      QPtrList<FileCreateFileType> subtypes = filetype->subtypes();
      for(FileCreateFileType * subtype = subtypes.first();
          subtype!=NULL;
          subtype=subtypes.next()) {
        if (subtype->enabled()) 
          setRow(row++, subtype);
      }
      
    }
    
  }
  if (currentSelection()>-1) removeSelection(currentSelection());

  connect( this, SIGNAL(currentChanged(int,int)), this, SLOT(slotCellSelected(int,int)) );
  
  
}

void FileCreateWidget2::setRow(int row, FileCreateFileType * filetype) {
  if (row+1>numRows()) setNumRows(row+1);
  setText(row, 1, filetype->name() );
  setText(row, 2, filetype->ext() );
  setText(row, 3, filetype->descr() );
  item(row,1)->setWordWrap(true);
  item(row,3)->setWordWrap(true);
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

void FileCreateWidget2::empty() {
  typeForRow.clear();
  int nrows = numRows();
  for(int r=0;r<nrows;r++) removeRow(0);

}

void FileCreateWidget2::setDefaultColumnWidths() {
  // FIXME: this is really nasty - hard-coded widths - but I can't find a
  // way to automatically set column widths based on contents with a QTable
  // Anyone??
  setColumnWidth(0,1);
  setColumnWidth(1,60);
  setColumnWidth(2,30);
  setColumnWidth(3,150);
}

void FileCreateWidget2::slotCellSelected(int row, int col) {
  if (col!=0) {
    setCurrentCell(row, 0);
    return;
  }

  m_selected = typeForRow[row];
  QTimer::singleShot(0, this, SLOT(slotDoSelection()) );  
  
}

void FileCreateWidget2::slotDoSelection() {
  if (m_selected) filetypeSelected(m_selected);
  if (currentSelection()>-1) removeSelection(currentSelection());
}
