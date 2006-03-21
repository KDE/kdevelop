/***************************************************************************
 *   Copyright (C) 2004-2005 by Sascha Cunz                                *
 *   sascha@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVLICENSE_H
#define KDEVLICENSE_H

/**
 * @file kdevfile.h
 * The interface to installed licenses.
 */

#include <qstringlist.h>

#include "kdevfile.h"

/*The interface to installed licenses.*/
class KDevLicense
{
public:
    /**
     * Constructs a KDevLicense and loads the given license template file.
     * @param name The name of this license.
     * @param fileName The file name where license template text is kept.
     */
    KDevLicense(const QString& name, const QString& fileName);

public:
    /**
     * Assembles the license to a string suitable for replacement of a %{LICENSE}.
     */
    QString assemble(KDevFile::CommentingStyle commentingStyle, const QString& author, const QString& email, int leadingSpaces );

    /**
     * @return the name of this license.
     */
    QString name() const { return m_name; }

    /**
     * @return  The XXX in KAboutData::LicenseXXX to be used
     */
    QString aboutDataEnum() const{ return m_KAboutDataEnum; }

    /**
     * @return  list of files tat must be present in the project's root
     *          directory
     *          if any source files uses this license.
     */
    QStringList copyFiles(){ return m_copyFiles; }

private:
    void readFile( const QString& fileName );

private:
    /**
     * Name of this license
     */
    QString m_name;
    /**
     * Raw lines of the license. Should contain no spaces at
     * the start or end of each line.
     */
    QStringList m_rawLines;
    /**
     * List of files that must be added to the project, if any
     * file in the project uses this license.
     */
    QStringList m_copyFiles;
    /**
     * Contains the XXX in KAboutData::LicenseXXX
     */
    QString m_KAboutDataEnum;
};

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
