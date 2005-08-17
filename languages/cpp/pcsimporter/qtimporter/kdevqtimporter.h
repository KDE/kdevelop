/***************************************************************************
 *   Copyright (C) 2003 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVQTIMPORTER_H
#define KDEVQTIMPORTER_H

#include <kdevpcsimporter.h>
#include <qpointer.h>

class SettingsDialog;

class KDevQtImporter : public KDevPCSImporter
{
    Q_OBJECT
public:
    KDevQtImporter( QObject* parent=0, const char* name=0, const QStringList& args=QStringList() );
    virtual ~KDevQtImporter();

    virtual QString dbName() const { return QString::fromLatin1("Qt"); }
    virtual QStringList fileList();
    virtual QStringList includePaths();

    virtual QWidget* createSettingsPage( QWidget* parent, const char* name=0 );

private:
    QPointer<SettingsDialog> m_settings;
};

#endif
