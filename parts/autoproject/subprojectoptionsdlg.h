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

#ifndef _SUBPROJECTOPTIONSDLG_H_
#define _SUBPROJECTOPTIONSDLG_H_

#include <qtabdialog.h>

class AutoProjectPart;
class AutoProjectWidget;
class SubprojectItem;
class QLineEdit;
class QListView;
class KEditListBox;


class SubprojectOptionsDialog : public QTabDialog
{
    Q_OBJECT
    
public:
    SubprojectOptionsDialog( AutoProjectPart *part, AutoProjectWidget *widget,
                             SubprojectItem *item, QWidget *parent=0, const char *name=0 );
    ~SubprojectOptionsDialog();

private slots:
    void cflagsClicked();
    void cxxflagsClicked();
    void f77flagsClicked();
    void addPrefixClicked();
    void removePrefixClicked();
    
private:
    QWidget *createCompilerTab();
    QWidget *createIncludeTab();
    QWidget *createPrefixTab();
    
    void init();
    virtual void accept();

    QLineEdit *cflags_edit;
    QLineEdit *cxxflags_edit;
    QLineEdit *f77flags_edit;
    QListView *prefix_view;
    KEditListBox *include_view;

    SubprojectItem *subProject;
    AutoProjectWidget *m_widget;
    AutoProjectPart *m_part;
};

#endif
