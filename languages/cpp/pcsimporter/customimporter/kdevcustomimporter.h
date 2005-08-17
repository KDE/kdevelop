/***************************************************************************
*   Copyright (C) 2003 by Alexander Dymo                                  *
*   cloudtemple@mksat.net                                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#ifndef KDEVCUSTOMIMPORTER_H
#define KDEVCUSTOMIMPORTER_H

#include "kdevpcsimporter.h"

#include <qpointer.h>

class SettingsDialog;

class KDevCustomImporter : public KDevPCSImporter
{
Q_OBJECT
public:
    KDevCustomImporter(QObject* parent = 0, const char* name = 0, const QStringList &args = QStringList());

    ~KDevCustomImporter();

    virtual QString dbName() const;
    virtual QStringList fileList();
    virtual QStringList includePaths();
    virtual QWidget* createSettingsPage(QWidget* parent, const char* name);

protected:
    QStringList fileList( const QString& path );
    void processDir(const QString path, QStringList &files );

private:
    QPointer<SettingsDialog> m_settings;
};

#endif
