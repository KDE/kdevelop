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

class FileGroupsPart;
class KDevProject;


class FileGroupsWidget : public KListView
{
    Q_OBJECT
    
public:
    FileGroupsWidget(FileGroupsPart *part);
    ~FileGroupsWidget();
    
public slots:
    void refresh();
    void addFiles ( const QStringList& fileList );
    void removeFiles ( const QStringList& fileList );

private slots:
    void slotItemExecuted(Q3ListViewItem *item);
    void slotContextMenu(KListView *, Q3ListViewItem *item, const QPoint &p);
    void slotToggleShowNonProjectFiles();
    void slotToggleDisplayLocation();
    
protected:
    void addFile(const QString &fileName);
    void removeFile(const QString &fileName);

private:
    QStringList allFilesRecursively( QString const & );
    FileGroupsPart *m_part;

    class KToggleAction *m_actionToggleShowNonProjectFiles;
    class KToggleAction *m_actionToggleDisplayLocation;
    int LocationID;
};

#endif

