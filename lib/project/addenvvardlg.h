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

#ifndef _ADDENVVARDLG_H_
#define _ADDENVVARDLG_H_

#include <qdialog.h>
#include <qlineedit.h>


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

private:
    QLineEdit *varname_edit;
    QLineEdit *value_edit;
};

#endif
