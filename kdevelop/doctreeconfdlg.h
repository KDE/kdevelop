/***************************************************************************
                             doctreeconfdlg.h
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


#ifndef DOCTREECONFDLG_H
#define DOCTREECONFDLG_H

#include <qtabdialog.h>

class QListView;


class DocTreeConfigDialog : public QTabDialog
{
    Q_OBJECT
	
public: 
    enum Page { KDevelop, Libraries, Others };
    
    DocTreeConfigDialog(Page page, QWidget *parent, const char *name=0);
    ~DocTreeConfigDialog();

    virtual void accept();

private slots:
    void addClicked();
    void editClicked();
    void removeClicked();

private:
    QListView *kdevelopView;
    QListView *librariesView;
    QListView *othersView;
    void readConfig();
    void storeConfig();
};

#endif

