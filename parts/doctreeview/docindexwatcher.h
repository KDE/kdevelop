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

#ifndef DOCINDEXWATCHER_H
#define DOCINDEXWATCHER_H

#include <qobject.h>
#include <qdict.h>

class DocTreeItem;

/**
 * Watches index files for changes and marks the documentation tree as dirty if
 * a change is detected.
 */
class DocIndexWatcher : public QObject
{
  Q_OBJECT

public:
  DocIndexWatcher(QObject* parent = 0L, const char* name = 0L);

  void addItem(DocTreeItem* item);
  void removeItem(const DocTreeItem* item);
  
private slots:
  void slotDirty(const QString& fileName);
  
private:
  QDict<DocTreeItem> m_docs;
};

#endif
