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

#ifndef _APPWIZARDDIALOG_H_
#define _APPWIZARDDIALOG_H_

class AppWizardPart;
class QComboBox;
class QLineEdit;
class QMultiLineEdit;
class QRadioButton;
class QVBox;
class KTempFile;

#include <qlist.h>
#include <qwizard.h>


class AppWizardDialog : public QWizard
{
    Q_OBJECT

public:
    AppWizardDialog( AppWizardPart *part, QWidget *parent=0, const char *name=0 );
    ~AppWizardDialog();

protected:
    virtual void accept();

private slots:
    void textChanged();
    void licenseChanged();
    
private:
    QStringList templateNames;
    KTempFile *tempFile;
    QList<QRadioButton> templateButtons;

    QVBox *firstpage, *secondpage;
    QLineEdit *appname_edit, *dest_edit;
    QLineEdit *author_edit, *email_edit, *version_edit;
    QComboBox *license_combo;
    QMultiLineEdit *filetemplate_edit;

    AppWizardPart *m_part;
};

#endif
