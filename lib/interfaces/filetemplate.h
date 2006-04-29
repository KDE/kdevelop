/* This file is part of the KDE project
   Copyright (C) 2002 Bernd Gehrmann <bernd@kdevelop.org>

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

#ifndef _FILETEMPLATE_H_
#define _FILETEMPLATE_H_

#include <QString>
#include <QHash>
#include "kdevexport.h"

class KDevPlugin;
class QDomDocument;

/**
@file filetemplate.h
FileTemplate class with utility methods to work with file templates.
*/

/**
Utilities to work with file templates in the current project.
*/
class KDEVINTERFACES_EXPORT FileTemplate
{
public:

    /**Policy of finding file templates.*/ 
    typedef enum { 
        Default    /**<Checks for templates in project and also for global filecreate templates.*/,
        Custom     /**<Checks for templates in custom directories. This usually means that 
                       full paths are given for FileTemplate methods.*/
    } Policy;

    /**
     * @return Whether a template with the given name
     * exists in the current project. File templates
     * are stored in the "templates" subdirectory of a project or in application shared dirs.
     */
    static bool exists(KDevPlugin *part, const QString &name, Policy p = Default);
    
    /**
     * Reads a template with the given name (e.g. "cpp")
     * and makes variable substitutions (like $AUTHOR$ etc.)
     * in it. The resulting string is returned.
     */
    static QString read(KDevPlugin *part, const QString &name, Policy p = Default);

    /**
     * Reads a template with the given URL
     * and makes variable substitutions (like $AUTHOR$ etc.)
     * in it. The resulting string is returned.
     */
    static QString readFile(KDevPlugin *part, const QString &fileName);

    /**
     * Makes variable substitutions on a text, based on a specified QDomDocument 
     * describing a KDevelop project file. The resulting string is returned.
     */
    static QString makeSubstitutions(QDomDocument &dom, const QString &text);

    /**
     * Copies a file template with the given name to the
     * file with the name dest and - while copying -
     * performs variable substitutions.
     */
    static bool copy(KDevPlugin *part, const QString &name,
                     const QString &dest, Policy p = Default);
    /**
     * Translates a template name into a full path, or suggests a full path
     * for the template in the project directory if it doesn't exist.
     */
    static QString fullPathForName(KDevPlugin *part, const QString &name, Policy p = Default);

    /**
     * Escape a substitution map for usage on a XML file.
     */
    static QHash<QString,QString> normalSubstMapToXML( const QHash<QString,QString>& src );
};

#endif
