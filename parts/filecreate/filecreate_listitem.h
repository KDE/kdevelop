#ifndef __FILECREATE_LISTITEM_H__
#define __FILECREATE_LISTITEM_H__

#include <qlistview.h>
#include <qobject.h>

#include "filecreate_filetype.h"

class FileCreateListItem : public QListViewItem {

public:
  FileCreateListItem(QListView * listview, const FileCreateFileType * filetype);
  FileCreateListItem(FileCreateListItem * listitem, const FileCreateFileType * filetype);
  virtual ~FileCreateListItem() { }

  const FileCreateFileType * filetype() const { return m_filetype; }
  
private:
  const FileCreateFileType * m_filetype;
  
};

#endif

