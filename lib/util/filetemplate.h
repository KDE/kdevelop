/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FILETEMPLATE_H_
#define _FILETEMPLATE_H_

#include <qstring.h>

class KDevPlugin;


class FileTemplate
{
public:
    
    typedef enum { Default, Custom } Policy;
    /**
     * Returns whether a template with the given name
     * exists in the current project. File templates
     * are stored in the "templates" subdirectory of a project.
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
     * Copies a file template with the given name to the
     * file with the name dest and - while copying -
     * performs variable substitutions.
     */
    static bool copy(KDevPlugin *part, const QString &name,
                     const QString &dest, Policy p = Default);
};

#endif
