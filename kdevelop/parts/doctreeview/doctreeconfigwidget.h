/***************************************************************************
                             doctreeconfigwidget.h
                             --------------------
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


#ifndef _DOCTREECONFIGWIDGET_H_
#define _DOCTREECONFIFWIDGET_H_

#include <qhbox.h>
#include <qtabwidget.h>

class QListView;
class DocTreeView;


class DocTreeConfigWidget : public QTabWidget
{
    Q_OBJECT
	
public: 
    DocTreeConfigWidget( DocTreeView *doctree, QWidget *parent, const char *name=0 );
    ~DocTreeConfigWidget();

    enum Page { KDevelop, Libraries, Others };
    void showPage(Page page);

private slots:
    void accept();
    void addClicked();
    void editClicked();
    void removeClicked();

private:
    DocTreeView *part;
    QListView *kdevelopView;
    QListView *librariesView;
    QListView *othersView;
    QHBox *kdevelopTab;
    QHBox *librariesTab;
    QHBox *othersTab;
    void readConfig();
    void storeConfig();
};

#endif
