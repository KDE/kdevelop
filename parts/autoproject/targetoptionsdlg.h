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

#ifndef _TARGETOPTIONSDLG_
#define _TARGETOPTIONSDLG_

#include <qdialog.h>

class AutoProjectWidget;
class TargetItem;
class QLineEdit;


class TargetOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    TargetOptionsDialog( AutoProjectWidget *widget, TargetItem *item,
                         QWidget *parent=0, const char *name=0 );
    ~TargetOptionsDialog();

private:
    void init();
    virtual void accept();
    
    QLineEdit *ldlibadd_edit;
    QLineEdit *ldflags_edit;
    QLineEdit *dependencies_edit;

    TargetItem *target;
    AutoProjectWidget *m_widget;
};

#endif
