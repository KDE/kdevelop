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

#ifndef _ADDTARGETDLG_H_
#define _ADDTARGETDLG_H_

#include <qdialog.h>
#include <qstrlist.h>

class QComboBox;
class QLineEdit;
class SubprojectItem;
class AutoProjectWidget;


class AddTargetDialog : public QDialog
{
    Q_OBJECT
    
public:
    AddTargetDialog( AutoProjectWidget *widget, SubprojectItem *item,
                     QWidget *parent=0, const char *name=0 );
    ~AddTargetDialog();

protected:
    virtual void accept();

private slots:
    void primaryChanged();

private:
    QComboBox *primary_combo;
    QComboBox *prefix_combo;
    QLineEdit *name_edit;
    SubprojectItem *subProject;
    AutoProjectWidget *m_widget;
};

#endif
