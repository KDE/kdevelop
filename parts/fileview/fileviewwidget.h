/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FILEVIEWWIDGET_H_
#define _FILEVIEWWIDGET_H_

#include <klistview.h>

class FileViewPart;
class KDevProject;


class FileViewWidget : public KListView
{
    Q_OBJECT
    
public:
    FileViewWidget(FileViewPart *part);
    ~FileViewWidget();

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
