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

#ifndef _ADDDOCITEMDLG_H_
#define _ADDDOCITEMDLG_H_

#include <qdialog.h>
#include <qlineedit.h>


class AddDocItemDialog : public QDialog
{
    Q_OBJECT

public: 
    AddDocItemDialog( QWidget *parent=0, const char *name=0 );
    ~AddDocItemDialog();

    QString title() const
        { return title_edit->text(); }
    QString url() const
        { return url_edit->text(); }
    
private slots:
    void fileButtonClicked();

private:
    QLineEdit *title_edit;
    QLineEdit *url_edit;
};

#endif
