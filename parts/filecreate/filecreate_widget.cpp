#include <qptrlist.h>

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <kurl.h>

#include <kdevcore.h>

#include "kdevproject.h"
#include "filecreate_part.h"
#include "filecreate_widget.h"
#include "filecreate_filetype.h"
#include "filecreate_listitem.h"

#include <iostream>

FileCreateWidget::FileCreateWidget(FileCreatePart *part)
  : KListView(0, "filecreate widget"), m_part(part)
{
  
  addColumn("File Type");
  addColumn("Extension");

  setRootIsDecorated(true);
  
  connect( this, SIGNAL(clicked(QListViewItem*)), this, SLOT(slotTypeSelected(QListViewItem*)) );
}


FileCreateWidget::~FileCreateWidget()
{
}

void FileCreateWidget::refresh() {
  clear();
  QPtrList<FileCreateFileType> filetypes = m_part->getFileTypes();
  for(FileCreateFileType * filetype = filetypes.first();
      filetype!=NULL;
      filetype=filetypes.next()) {
    FileCreateListItem * listitem = new FileCreateListItem( this, filetype );
    QPtrList<FileCreateFileType> subtypes = filetype->subtypes();
    for(FileCreateFileType * subtype = subtypes.first();
        subtype!=NULL;
        subtype=subtypes.next()) {
      new FileCreateListItem( listitem, subtype );
    }
  }
}

void FileCreateWidget::slotTypeSelected(QListViewItem * item) {
  FileCreateListItem * fileitem = dynamic_cast<FileCreateListItem*>(item);
  if (!fileitem) return;
  
  const FileCreateFileType * filetype = fileitem->filetype();

  emit filetypeSelected(filetype);
}
  


#include "filecreate_widget.moc"
