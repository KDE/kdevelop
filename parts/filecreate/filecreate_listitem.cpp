
#include "filecreate_listitem.h"

#include <kglobal.h>
#include <kiconloader.h>
//#include <kicon.h>

#include <qpixmap.h>

FileCreateListItem::FileCreateListItem(QListView * listview, const FileCreateFileType * filetype) :
  QListViewItem(listview, filetype->name(), filetype->ext() ), m_filetype(filetype) {

  KIconLoader * loader = KGlobal::iconLoader();
  QPixmap iconPix = loader->loadIcon(filetype->icon(), KIcon::Small, 0,
                                     KIcon::DefaultState, NULL,
                                     true);
  if (!iconPix.isNull())
    setPixmap(0, iconPix);
  
}


FileCreateListItem::FileCreateListItem(FileCreateListItem * listitem, const FileCreateFileType * filetype) :
  QListViewItem(listitem, filetype->name(), filetype->ext() ), m_filetype(filetype) {

  KIconLoader * loader = KGlobal::iconLoader();
  QPixmap iconPix = loader->loadIcon(filetype->icon(), KIcon::Small, 0,
                                     KIcon::DefaultState, NULL,
                                     true);
  if (!iconPix.isNull())
    setPixmap(0, iconPix);

}




