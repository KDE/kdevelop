/***************************************************************************
 *   Copyright (C) 1999-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002 by Sebastian Kratzert                              *
 *   skratzert@gmx.de                                                      *
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "doctreeitem.h"

#include <klistview.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kcursor.h>
#include <kapplication.h>

#include "docindexwatcher.h"

static DocIndexWatcher* m_watcher = 0L;

static DocIndexWatcher* watcher()
{
    if (!m_watcher)
        m_watcher = new DocIndexWatcher();
    
    return m_watcher;
}

DocTreeItem::DocTreeItem(KListView *parent, Type type, const QString &text, const QString &context, bool lazy)
  : QListViewItem(parent, text)
  , m_typ(type)
  , m_context(context)
  , m_current(false)
  , m_lazy(lazy)
{
    init();
}

DocTreeItem::DocTreeItem(DocTreeItem *parent, Type type, const QString &text, const QString &context, bool lazy)
  : QListViewItem(parent, text)
  , m_typ(type)
  , m_context(context)
  , m_current(false)
  , m_lazy(lazy)
{
    init();
}

DocTreeItem::~DocTreeItem()
{
    if (!m_indexFileName.isEmpty())
        watcher()->removeItem(this);
}

void DocTreeItem::init()
{
    QString icon;
    if (m_typ == Folder)
        icon = "folder";
    else if (m_typ == Book)
        icon = "contents";
    else
        icon = "document";
    setPixmap(0, SmallIcon(icon));
    
    if (m_lazy)
        setExpandable(true);
    /* gets called in setup()
    else
        refresh();*/
}

void DocTreeItem::setFileName(const QString &fn)
{ 
    m_fileName = fn;
}

QString DocTreeItem::fileName()
{ 
    return m_fileName;
}

QString DocTreeItem::context() const
{ 
    return m_context;
}

DocTreeItem::Type DocTreeItem::type() const
{
    return m_typ;
}

bool DocTreeItem::isCurrent() const
{
    return m_current;
}

void DocTreeItem::setDirty()
{
    m_current = false;
}

void DocTreeItem::setOpen(bool open)
{
    if (m_lazy && open && !isCurrent()) {
        kapp->setOverrideCursor(KCursor::waitCursor());
        refresh();
        kapp->restoreOverrideCursor();
    }
      
    QListViewItem::setOpen(open);
}

void DocTreeItem::refresh()
{
    clear();
    m_current = true;
}

void DocTreeItem::clear()
{
    QListViewItem *child = firstChild();
    while (child)
    {
        QListViewItem *old = child;
        child = child->nextSibling();
        delete old;
    }
}

const QString & DocTreeItem::indexFileName( ) const
{
    return m_indexFileName;
}

void DocTreeItem::setIndexFileName( const QString & fileName )
{
    if (!m_indexFileName.isEmpty())
        watcher()->removeItem(this);
    
    m_indexFileName = fileName;
    watcher()->addItem(this);
}
