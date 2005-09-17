/* This file is part of the KDE project
   Copyright (C) 2004 Sascha Cunz <sascha@sacu.de>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KDEVLICENSE_H
#define KDEVLICENSE_H

/**
@file kdevfile.h
The interface to installed licenses.
*/

#include <qstringlist.h>

#include "kdevfile.h"

/*The interface to installed licenses.*/
class KDevLicense
{
public:
    /**Constructs a KDevLicense and loads the given license template file.
    @param name The name of this license.
    @param fileName The file name where license template text is kept.*/
    KDevLicense(const QString& name, const QString& fileName);

public:
    /**Assembles the license to a string suitable for replacement of a %{LICENSE} .*/
    QString assemble(KDevFile::CommentingStyle commentingStyle, const QString& author, const QString& email, int leadingSpaces );

    /**
    * returns the name of this license.
    */
    QString name() const { return m_name; }

    /**
    * returns list of files tat must be present in the project's root directory
    * if any source files uses this license.
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
};

#endif
