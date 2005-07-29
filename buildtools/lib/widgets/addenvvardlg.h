/* This file is part of the KDE project
   Copyright (C) 2001 Bernd Gehrmann <bernd@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _ADDENVVARDLG_H_
#define _ADDENVVARDLG_H_

#include <qdialog.h>
#include <qlineedit.h>
class QPushButton;

/**
Dialog to add environment variables.
*/
class AddEnvvarDialog : public QDialog
{
    Q_OBJECT

public:
    AddEnvvarDialog( QWidget *parent=0, const char *name=0 );
    ~AddEnvvarDialog();

    QString varname() const
        { return varname_edit->text(); }
    QString value() const
        { return value_edit->text(); }
    void setvarname(const QString name) const
        { varname_edit->setText(name); }
    void setvalue(const QString value) const
        { value_edit->setText(value); }
 private slots:
    void slotTextChanged();
private:
    QLineEdit *varname_edit;
    QLineEdit *value_edit;
    QPushButton *m_pOk;
};

#endif
