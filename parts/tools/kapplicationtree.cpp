/*  This file is part of the KDE libraries
    Nicked from KDElibs since KDevApplicationTree is not a public class..

    Copyright (C) 1997 Torben Weis <weis@stud.uni-frankfurt.de>
    Copyright (C) 1999 Dirk A. Mueller <dmuell@gmx.net>
    Portions copyright (C) 1999 Preston Brown <pbrown@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qfile.h>
#include <qdir.h>
#include <qdialog.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qcheckbox.h>
#include <qtooltip.h>
#include <qstyle.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <Q3PtrList>

#include <kapplication.h>
#include <kbuttonbox.h>
#include <kcombobox.h>
#include <kdesktopfile.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klineedit.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmimemagic.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kuserprofile.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>
#include <dcopclient.h>
#include <kmimetype.h>
#include <kservicegroup.h>
#include <klistview.h>
#include <ksycoca.h>
#include <kdebug.h>

#include "kapplicationtree.h"

template class Q3PtrList<QString>;

#define SORT_SPEC (QDir::DirsFirst | QDir::Name | QDir::IgnoreCase)


// ----------------------------------------------------------------------

KDevAppTreeListItem::KDevAppTreeListItem( KListView* parent, const QString & name,
                                    const QPixmap& pixmap, bool parse, bool dir, const QString& p, const QString& c, const QString& dE )
    : Q3ListViewItem( parent, name )
{
    init(pixmap, parse, dir, p, c, dE);
}


// ----------------------------------------------------------------------

KDevAppTreeListItem::KDevAppTreeListItem( Q3ListViewItem* parent, const QString & name,
                                    const QPixmap& pixmap, bool parse, bool dir, const QString& p, const QString& c, const QString& dE )
    : Q3ListViewItem( parent, name )
{
    init(pixmap, parse, dir, p, c, dE);
}


// ----------------------------------------------------------------------

void KDevAppTreeListItem::init(const QPixmap& pixmap, bool parse, bool dir, const QString& _path, const QString& _exec, const QString& _dEntry)
{
    setPixmap(0, pixmap);
    parsed = parse;
    directory = dir;
    path = _path; // relative path
    exec = _exec;
    dEntry = _dEntry;
    exec.simplifyWhiteSpace();
    exec.truncate(exec.find(' '));
}


// ----------------------------------------------------------------------
// Ensure that dirs are sorted in front of files and case is ignored

QString KDevAppTreeListItem::key(int column, bool /*ascending*/) const
{
    if (directory)
        return QString::fromLatin1(" ") + text(column).upper();
    else
        return text(column).upper();
}

void KDevAppTreeListItem::activate()
{
    if ( directory )
        setOpen(!isOpen());
}

void KDevAppTreeListItem::setOpen( bool o )
{
    if( o && !parsed ) { // fill the children before opening
        ((KDevApplicationTree *) parent())->addDesktopGroup( path, this );
        parsed = true;
    }
    Q3ListViewItem::setOpen( o );
}

bool KDevAppTreeListItem::isDirectory()
{
    return directory;
}

// ----------------------------------------------------------------------

KDevApplicationTree::KDevApplicationTree( QWidget *parent, const char* name )
    : KListView( parent, name ), currentitem(0)
{
    addColumn( i18n("Known Applications") );
    setRootIsDecorated( true );

    addDesktopGroup( QString::null );

    connect( this, SIGNAL( currentChanged(Q3ListViewItem*) ), SLOT( slotItemHighlighted(Q3ListViewItem*) ) );
    connect( this, SIGNAL( selectionChanged(Q3ListViewItem*) ), SLOT( slotSelectionChanged(Q3ListViewItem*) ) );
}

// ----------------------------------------------------------------------

bool KDevApplicationTree::isDirSel()
{
    if (!currentitem) return false; // if currentitem isn't set
    return currentitem->isDirectory();
}

// ----------------------------------------------------------------------

void KDevApplicationTree::addDesktopGroup( QString relPath, KDevAppTreeListItem *item)
{
   KServiceGroup::Ptr root = KServiceGroup::group(relPath);
   KServiceGroup::List list = root->entries();

   KDevAppTreeListItem * newItem;
   for( KServiceGroup::List::ConstIterator it = list.begin();
       it != list.end(); it++)
   {
      QString icon;
      QString text;
      QString relPath;
      QString exec;
      QString dEntry;
      bool isDir = false;
      KSycocaEntry *p = (*it);
      if (p->isType(KST_KService))
      {
         KService *service = static_cast<KService *>(p);
         icon = service->icon();
         text = service->name();
         exec = service->exec();
         dEntry = service->desktopEntryPath();
      }
      else if (p->isType(KST_KServiceGroup))
      {
         KServiceGroup *serviceGroup = static_cast<KServiceGroup *>(p);
         icon = serviceGroup->icon();
         text = serviceGroup->caption();
         relPath = serviceGroup->relPath();
         isDir = true;
         if ( text[0] == '.' ) // skip ".hidden" like kicker does
           continue;
         // avoid adding empty groups
         KServiceGroup::Ptr subMenuRoot = KServiceGroup::group(relPath);
         if (subMenuRoot->childCount() == 0)
           continue;
      }
      else
      {
         kdWarning(250) << "KServiceGroup: Unexpected object in list!" << endl;
         continue;
      }

      QPixmap pixmap = SmallIcon( icon );

      if (item)
         newItem = new KDevAppTreeListItem( item, text, pixmap, false, isDir,
                                         relPath, exec, dEntry );
      else
         newItem = new KDevAppTreeListItem( this, text, pixmap, false, isDir,
                                         relPath, exec, dEntry );
      if (isDir)
         newItem->setExpandable( true );
   }
}


// ----------------------------------------------------------------------

void KDevApplicationTree::slotItemHighlighted(Q3ListViewItem* i)
{
    // i may be 0 (see documentation)
    if(!i)
        return;

    KDevAppTreeListItem *item = (KDevAppTreeListItem *) i;

    currentitem = item;

    if( (!item->directory ) && (!item->exec.isEmpty()) )
        emit highlighted( item->text(0), item->exec );
}


// ----------------------------------------------------------------------

void KDevApplicationTree::slotSelectionChanged(Q3ListViewItem* i)
{
    // i may be 0 (see documentation)
    if(!i)
        return;

    KDevAppTreeListItem *item = (KDevAppTreeListItem *) i;

    currentitem = item;

    if( ( !item->directory ) && (!item->exec.isEmpty() ) )
        emit selected( item->text(0), item->exec );
}

// ----------------------------------------------------------------------

void KDevApplicationTree::resizeEvent( QResizeEvent * e)
{
    setColumnWidth(0, width()-QApplication::style().pixelMetric(QStyle::PM_ScrollBarExtent));
    KListView::resizeEvent(e);
}



#include "kapplicationtree.moc"

