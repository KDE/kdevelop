/***************************************************************************
                             doctreeview.h
                             -------------------

    copyright            : (C) 1998 by Sandy Meier
    email                : smeier@rz.uni-potsdam.de
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
#define OLD
#include "klistview.h"
#include <stdio.h>
#include <qtooltip.h>

class DocTreeKDevelopFolder;
class DocTreeKDELibsFolder;
class DocTreeOthersFolder;
class DocTreeProjectFolder;
class DocTreeDocbaseFolder;
class ListViewBookItem;
class CProject;


/**
 * The documentation tree (manual, tutorial, KDE-libs...)
 * @author Sandy Meier & Bernd Gehrmann (rewrite)
 */

class DocTreeView : public KListView
{
    Q_OBJECT 
public: 
    DocTreeView( QWidget *parent=0, const char *name=0 );
    ~DocTreeView();

    void refresh(CProject *prj);
    QString selectedText();
    void changePathes();
    void tip( const QPoint &p, QRect &r, QString &str );
    
protected slots:
    void slotUpdateAPI() { emit signalUpdateAPI();};
    void slotUpdateUserManual(){ emit signalUpdateUserManual();};
    void slotAddDocumentation();
    void slotRemoveDocumentation();
    void slotDocumentationProp();

    void slotSelectionChanged(QListViewItem *item);
    void slotRightButtonPressed(QListViewItem *item, const QPoint &p, int);
	
signals:
    void fileSelected(QString url_file);
    void signalUpdateAPI();
    void signalUpdateUserManual();
private: 
//    CProject* project;
    DocTreeKDevelopFolder *folder_kdevelop;
    DocTreeKDELibsFolder *folder_kdelibs;
    DocTreeOthersFolder *folder_others;
    DocTreeDocbaseFolder *folder_docbase;
    DocTreeProjectFolder *folder_project;

private: // Private classes
  /** Class that handles dynamic tooltips in the doctree. */
  class DocToolTip : public QToolTip
  {
  public:
    DocToolTip(QWidget *parent);
  protected:
    void maybeTip( const QPoint & );
  };
  /** The dynamic tooltip:er. */
  DocToolTip * toolTip;
};

/*************************************/
/* Folder "KDevelop"                */
/*************************************/


/**
 * A list view item that is decorated with a book icon.
 * This typically represents one manual. When the user "opens"
 * the book, the according icon is changed.
 */
class ListViewBookItem : public KListViewItem
{
public:
    ListViewBookItem( KListViewItem *parent,
                      const char *text, const char *filename );
    virtual void setOpen(bool o);
};


/**
 * Here we specialize on a KDevelop book. The constructor takes
 * only the last part of the file name as argument and tries to
 * locate the file according to the locale set.
 * The constructor allows an argument expandable, but
 * the setOpen() implementation is currently nothing more than
 * a dirty hack.
 */
class DocTreeKDevelopBook : public ListViewBookItem
{
public:
    DocTreeKDevelopBook( KListViewItem *parent, const char *text,
                         const char *filename, bool expandable=false )
        : ListViewBookItem(parent, text, locatehtml(filename))
        { setExpandable(expandable); }
    virtual void setOpen(bool o);
    static QString readIndexTitle(const char* book);
    static QString locatehtml(const char *filename);
private:
    void readSgmlIndex(FILE *f);
};

#endif


