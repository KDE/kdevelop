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

#ifndef _FILEGROUPSWIDGET_H_
#define _FILEGROUPSWIDGET_H_

#include <klistview.h>

class FileViewPart;
class KDevProject;


class FileGroupsWidget : public KListView
{
    Q_OBJECT
    
public:
    FileGroupsWidget(FileViewPart *part);
    ~FileGroupsWidget();

public slots:
    void refresh();
    void addFile(const QString &fileName);
    void removeFile(const QString &fileName);
    
private slots:
    void slotItemExecuted(QListViewItem *item);
    void slotContextMenu(KListView *, QListViewItem *item, const QPoint &p);

private:
    FileViewPart *m_part;
};

#endif
