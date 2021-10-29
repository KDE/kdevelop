/*
    SPDX-FileCopyrightText: 2004 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SHELLEXTENSION_H
#define KDEVPLATFORM_SHELLEXTENSION_H

#include <QString>
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

    /**Reimplement to return the path to the executable that needs to be executed for new sessions.*/
    virtual QString executableFilePath() = 0;

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
     * loaded by default.
     * If an empty list is returned, instead the plugin metadata is fallen back to,
     * by reading the bool value KPlugin/EnabledByDefault (default: true).
     */
    virtual QStringList defaultPlugins() = 0;

protected:
    ShellExtension();
    static ShellExtension *s_instance;
};

}
#endif

