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
#include <qstringlist.h>

class QComboBox;
class QLineEdit;

class AppWizardPart;


class ImportDialog : public QDialog
{
    Q_OBJECT
    
public:
    ImportDialog( AppWizardPart *part, QWidget *parent=0, const char *name=0 );
    ~ImportDialog();

protected:
    virtual void accept();
    
private slots:
    void dirButtonClicked();
    
private:
    QStringList importNames;
    QLineEdit *name_edit;
    QLineEdit *dir_edit;
    QComboBox *project_combo;

    AppWizardPart *m_part;

};

#endif
