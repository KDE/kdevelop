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

#ifndef _ADDFILEGROUPDLG_H_
#define _ADDFILEGROUPDLG_H_

#include <qdialog.h>
#include <klineedit.h>
class QPushButton;

class AddFileGroupDialog : public QDialog
{
    Q_OBJECT

public:
    AddFileGroupDialog( const QString& old_title="", const QString& old_pattern="", QWidget *parent=0, const char *name=0 );
    ~AddFileGroupDialog();

    QString title() const
        { return title_edit->text(); }
    QString pattern() const
        { return pattern_edit->text(); }
 private slots:
    void slotTextChanged();
private:
    KLineEdit *title_edit;
    KLineEdit *pattern_edit;
    QPushButton *m_pOk;
};

#endif
