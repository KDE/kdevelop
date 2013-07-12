/***************************************************************************
 *   Copyright 2004 Alexander Dymo <adymo@kdevelop.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef KDEVPLATFORM_SHELLEXTENSION_H
#define KDEVPLATFORM_SHELLEXTENSION_H

#include <QtCore/QString>
#include "shellexport.h"

namespace KDevelop
{

/**Default area parameters collection.*/
struct AreaParams {
    /**Unique name for the area.*/
    QString name;
    /**User-visible area title.*/
    QString title;
};

/**
Shell extension.
Provides application-dependent and shell-independent functionality.
Shell uses extensions to perform application dependent actions.
*/
class KDEVPLATFORMSHELL_EXPORT ShellExtension {
public:
    virtual ~ShellExtension() {}

    /**Returns an instance of a shell. Subclasses must create an instance of a shell
    by themselves. For example they could provide static init() method like:
    @code
    static void init()
    {
        s_instance = new MyExtension();
    }
    @endcode*/
    static ShellExtension *getInstance();

    /**Reimplement to return the path to the binary that needs to be executed for new sessions.*/
    virtual QString binaryPath() = 0;

    /**Reimplement to return the name of KXMLGUI resource file for an application.*/
    virtual QString xmlFile() = 0;

    /**Reimplement to return the name of the default ui area.*/
    virtual AreaParams defaultArea() = 0;

    /**Reimplement to return the filename extension for project files.*/
    virtual QString projectFileExtension() = 0;

    /**Reimplement to return the description for project files.*/
    virtual QString projectFileDescription() = 0;

    /**
     * Reimplement to return the list of plugins that should
     * automatically be loaded
     */
    virtual QStringList defaultPlugins() = 0;

protected:
    ShellExtension();
    static ShellExtension *s_instance;
};

}
#endif

