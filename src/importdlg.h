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

#ifndef _IMPORTDLG_H_
#define _IMPORTDLG_H_

#include <qdialog.h>

class ServiceComboBox;
class QComboBox;
class QLineEdit;


class ImportDialog : public QDialog
{
    Q_OBJECT
    
public:
    ImportDialog( QWidget *parent=0, const char *name=0 );
    ~ImportDialog();

protected:
    virtual void accept();
    
private slots:
    void dirButtonClicked();
    
private:
    QLineEdit *dir_edit;
    ServiceComboBox *project_combo;
    QComboBox *language_combo;
    //    ServiceComboBox *language_combo;
};

#endif
