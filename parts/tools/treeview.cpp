/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "treeview.h"

#include <qdir.h>
#include <qheader.h>
#include <qstringlist.h>
#include <qfileinfo.h>
#include <qdragobject.h>
#include <qdatastream.h>
#include <qcstring.h>
#include <qpopupmenu.h>
#include <qregexp.h>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kiconloader.h>
#include <kdesktopfile.h>
#include <kaction.h>
#include <kservicegroup.h>
#include <ksycoca.h>
#include <ksycocaentry.h>
#include <kservice.h>
#include <kdebug.h>


TreeItem::TreeItem(QListViewItem *parent, const QString& file)
    : QListViewItem(parent), _file(file)
{}


TreeItem::TreeItem(QListViewItem *parent, QListViewItem *after, const QString& file)
    : QListViewItem(parent, after), _file(file)
{}


TreeItem::TreeItem(QListView *parent, const QString& file)
    : QListViewItem(parent), _file(file)
{}


TreeItem::TreeItem(QListView *parent, QListViewItem *after, const QString& file)
    : QListViewItem(parent, after), _file(file)
{}


TreeView::TreeView( QWidget *parent, const char *name )
    : KListView(parent, name)
{
  setRootIsDecorated(true);
  setAllColumnsShowFocus(true);
  setSorting( -1);
  setAcceptDrops(true);
  setDropVisualizer(true);
  setDragEnabled(true);

  addColumn(QString::null);
  header()->hide();

  fill();
}

void TreeView::fill()
{
  clear();
  fillBranch("", 0);
}

void TreeView::fillBranch(const QString& rPath, TreeItem *parent)
{
  KServiceGroup::Ptr root = KServiceGroup::group(rPath);
  if (!root)
    return;

  KServiceGroup::List list = root->entries(true);
  if (list.isEmpty()) 
    return;

  KServiceGroup::List::ConstIterator it = list.begin();
  for (; it != list.end(); ++it) 
  {
    KSycocaEntry * e = *it;

    if (e->isType(KST_KServiceGroup)) 
    {
      KServiceGroup::Ptr g(static_cast<KServiceGroup *>(e));

      // Avoid adding empty groups.
      KServiceGroup::Ptr subMenuRoot = KServiceGroup::group(g->relPath());
      if (subMenuRoot->childCount() == 0)
        continue;
      
      // Ignore dotfiles.
      if ((g->name().at(0) == '.'))
        continue;

      TreeItem *item = 0;

      if (parent)
	item = new TreeItem(parent, "");
      else
	item = new TreeItem(this, "");

      decorateItem(item, g);

      fillBranch(g->name(), item);
    }
    else
    {
       KService::Ptr s(static_cast<KService *>(e));
      
       TreeItem *item = 0;

       if (parent)
         item = new TreeItem(parent, s->desktopEntryPath());
       else
         item = new TreeItem(this, s->desktopEntryPath());

       decorateItem(item, s);
    }
  }
}


void TreeView::decorateItem(TreeItem *item, KServiceGroup::Ptr g)
{
  QString groupCaption = g->caption();
  groupCaption.replace(QRegExp("&"), "&&");

  item->setText(0, groupCaption);
  item->setPixmap(0, KGlobal::iconLoader()->loadIcon(g->icon(), KIcon::Small));
}


void TreeView::decorateItem(TreeItem *item, KService::Ptr s)
{
  QString name = s->name();
  name.replace(QRegExp("&"), "&&");

  item->setText(0, name);
  item->setPixmap(0, s->pixmap(KIcon::Small));
}


#include "treeview.moc"
