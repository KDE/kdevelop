/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
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

#include "importdlgbase.h"
#include <qstringlist.h>


class AppWizardPart;


class ImportDialog : public ImportDialogBase
{
    Q_OBJECT

public:
    ImportDialog( AppWizardPart *part, QWidget *parent=0, const char *name=0 );
    ~ImportDialog();

protected:
    virtual void accept();
    virtual void dirChanged();

private slots:
    void slotFinishedCheckout( QString destinationDir );
    void slotFetchModulesFromRepository();
    void slotProjectNameChanged( const QString &_text );
private:
    void scanAvailableVCS();
    void scanLegacyKDevelopProject(const QString &fileName);
    void scanLegacyStudioProject(const QString &fileName);
    void scanAutomakeProject(const QString &dirName);
    void setProjectType(const QString &type);

    QStringList importNames;
    AppWizardPart *m_part;
};

#endif
