/***************************************************************************
 *   Copyright (C) 2004 by Hamish Rodda                                    *
 *   rodda@kde.org                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "docindexwatcher.h"

#include <kdirwatch.h>

#include "doctreeitem.h"

DocIndexWatcher::DocIndexWatcher(QObject* parent, const char* name)
  : QObject(parent, name)
{
    connect(KDirWatch::self(), SIGNAL(dirty(const QString&)), SLOT(slotDirty(const QString&)));
    connect(KDirWatch::self(), SIGNAL(deleted(const QString&)), SLOT(slotDirty(const QString&)));
}

void DocIndexWatcher::addItem(DocTreeItem* item)
{
    if (!item->indexFileName().isEmpty() && !m_docs.find(item->indexFileName())) {
        m_docs.insert(item->indexFileName(), item);
        KDirWatch::self()->addFile(item->indexFileName());
    }
}

void DocIndexWatcher::removeItem(const DocTreeItem* item)
{
    if (!item->indexFileName().isEmpty()) {
        KDirWatch::self()->removeFile(item->indexFileName());
        m_docs.remove(item->indexFileName());
    }
}

void DocIndexWatcher::slotDirty( const QString & fileName )
{
    for (QDictIterator<DocTreeItem> it = m_docs; it.current(); ++it)
        if (it.currentKey() == fileName) {
            it.current()->setDirty();
            break;
        }
}

#include "docindexwatcher.moc"
