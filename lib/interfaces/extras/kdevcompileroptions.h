/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 Roberto Raggi <roberto@kdevelop.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/**
@file kdevcompileroptions.h
The interface to compiler options configuration.
*/

#ifndef _KDEVCOMPILEROPTIONS_H_
#define _KDEVCOMPILEROPTIONS_H_

#include <qobject.h>

/**
The interface to compiler options configuration.
Used by build systems to give users a compiler options configuration dialog.

Common use case:
@code
static KDevCompilerOptions *createCompilerOptions( const QString &name, QObject *parent )
{
    KService::Ptr service = KService::serviceByDesktopName( name );
    if ( !service )
        return 0;
    
    KLibFactory *factory = KLibLoader::self()->factory(QFile::encodeName(service->library()));
    if (!factory)
        return 0;

    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());

    QObject *obj = factory->create(parent, service->name().latin1(),
                                   "KDevCompilerOptions", args);

    if (!obj->inherits("KDevCompilerOptions"))
        return 0;
    
    KDevCompilerOptions *dlg = (KDevCompilerOptions*) obj;
    return dlg;
}

...
KDevCompilerOptions *plugin = createCompilerOptions(compilerName, parent);
QString flags = ""; //old compiler flags
if ( plugin )
{
    flags = plugin->exec( parent, flags ); //new compiler flags are returned
    delete plugin;
}
@endcode
*/
class KDevCompilerOptions : public QObject
{
    Q_OBJECT

public:
    KDevCompilerOptions( QObject *parent=0);

    /**
     * Opens a dialog which allows the user to configure the
     * compiler options. The initial settings in the dialog
     * will be set from the flags argument of this method.
     * After the dialog is accepted, the new settings will
     * be returned as a string. If the dialog was cancelled,
     * QString() is returned.
     */
    virtual QString exec(QWidget *parent, const QString &flags) = 0;
};

#endif
