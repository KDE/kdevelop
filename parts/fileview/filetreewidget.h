/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FILETREEWIDGET_H_
#define _FILETREEWIDGET_H_

#include <qstringlist.h>
#include <klistview.h>

class FileViewPart;
class KDirWatch;


class FileTreeWidget : public KListView
{
    Q_OBJECT

    friend class FileTreeItem;
    
public:
    FileTreeWidget( FileViewPart *part, QWidget *parent=0, const char *name=0 );
    ~FileTreeWidget();

    void openDirectory(const QString &dirName);
    void watchDir(const QString &dirName);

public slots:
    void hideOrShow();
    
private slots:
    void slotDirectoryDirty(const QString &dirName);
    void slotItemExecuted(QListViewItem *item);
    void slotContextMenu(KListView *, QListViewItem *item, const QPoint &p);
    void slotToggleShowNonProjectFiles();

private:
    bool matchesHidePattern(const QString &fileName);
    
    FileViewPart *m_part;
    KDirWatch *m_dirWatch;
    QStringList m_hidePatterns;
    bool m_showNonProjectFiles;
};

#endif
