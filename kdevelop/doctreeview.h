/***************************************************************************
                             doctreeview.h
                             -------------------

    copyright            : (C) 1999 The KDevelop Team
    
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


/**
 * The documentation tree (manual, tutorial, KDE-libs...)
 * @author Sandy Meier & Bernd Gehrmann (rewrite)
 */

class DocTreeView : public KListView, public Component
{
    Q_OBJECT 
public: 
    DocTreeView( QWidget *parent=0, const char *name=0 );
    ~DocTreeView();

    void refresh(CProject *prj);
    //    QString selectedText();

    virtual void docPathChanged();

protected slots:
    void slotAddDocumentation();
    void slotRemoveDocumentation();
    void slotDocumentationProp();
    void slotConfigureKDELibs();

    void slotSelectionChanged(QListViewItem *item);
    void slotRightButtonPressed(QListViewItem *item, const QPoint &p, int);
	
signals:
    void fileSelected(QString url_file);

private: 
    CProject* project;
    DocTreeKDevelopFolder *folder_kdevelop;
    DocTreeKDELibsFolder *folder_kdelibs;
    DocTreeOthersFolder *folder_others;
    DocTreeDocbaseFolder *folder_docbase;
    DocTreeProjectFolder *folder_project;
};
#endif


