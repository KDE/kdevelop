/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2007 by Hamish Rodda                                    *
 *   rodda@kde.org                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IMPORTPROJECT_H
#define IMPORTPROJECT_H

#include "ui_importproject.h"
#include <kdialog.h>
#include <QFile>

class AppWizardPart;

struct InfrastructureCmd{
    bool isOn;
    QString comment;
    QString command;
    QString existingPattern;
};

class ImportProject : public QDialog, public Ui::ImportProject
{
    Q_OBJECT

public:
    ImportProject( AppWizardPart *part, QWidget *parent=0 );
    ~ImportProject();

protected:
    virtual void accept();
    bool eventFilter ( QObject * watched, QEvent * event );

protected slots:
    virtual void dirChanged();

private slots:
//    void slotFinishedCheckout( QString destinationDir );
//    void slotFetchModulesFromRepository();

    private:
//    void scanAvailableVCS();

    void importProject(QFile& file);
    QString manager() const;
    QString primaryLanguage() const;

    AppWizardPart *m_part;
};

#endif // IMPORTPROJECT_H
