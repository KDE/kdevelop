/***************************************************************************
                             doctreeview.h
                             -------------------
    copyright            : (C) 1999 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef _DOCTREEVIEW_H_
#define _DOCTREEVIEW_H_

#include "component.h"
#include "klistview.h"


class DocTreeKDevelopFolder;
class DocTreeKDELibsFolder;
class DocTreeOthersFolder;
class DocTreeProjectFolder;
class DocTreeDocbaseFolder;
class CProject;


class DocTreeView : public KListView, public Component
{
    Q_OBJECT 
public: 
    DocTreeView( QWidget *parent=0, const char *name=0 );
    ~DocTreeView();

    // This is currently called by DocTreeViewConfigWidget if the
    // changes are accepted. Maybe this could be done through
    // the component system (but maybe not ;-)
    void configurationChanged();

protected:
    // Component notifications:
    virtual void docPathChanged();
    virtual void createConfigWidget(CustomizeDialog *parent);
    virtual void projectClosed();
    virtual void projectOpened(CProject *prj);

protected slots:
    void slotConfigure();
    void slotLeftButtonPressed(QListViewItem *item);
    void slotRightButtonPressed(QListViewItem *item, const QPoint &p, int);
	
signals:
    void fileSelected(const QString &url_file);
    void projectAPISelected();
    void projectManualSelected();

private: 
    QListViewItem *contextItem;
    DocTreeKDevelopFolder *folder_kdevelop;
    DocTreeKDELibsFolder *folder_kdelibs;
    DocTreeOthersFolder *folder_others;
    DocTreeDocbaseFolder *folder_docbase;
    DocTreeProjectFolder *folder_project;
};
#endif


