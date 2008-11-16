/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef VERITAS_CTESTFILEPARSER_H
#define VERITAS_CTESTFILEPARSER_H

#include "veritasexport.h"
#include "testexecutableinfo.h"
#include <kurl.h>

namespace Veritas
{

/*! Extra layer of abstraction to allow mocking. */
class VERITAS_EXPORT FilesystemAccess
{
public:
    FilesystemAccess();
    virtual ~FilesystemAccess();
    virtual bool changeDir(const KUrl& dir);
    virtual QStringList listing();
    virtual QIODevice* file(const QString& name);
    virtual KUrl currentDirectory() const;
    
private:
    KUrl m_currentDir;
};

/*! Retrieves test location information through cmake files in the builddirectory. */
class VERITAS_EXPORT CTestfileParser
{
public:
    CTestfileParser(FilesystemAccess*);
    virtual ~CTestfileParser();

    /*! Process @p dir and it's subdirectories recursively for CTestCTestfile.cmake's.
     *  @n blocking. */
    void parse(const KUrl& dir);
    QList<Veritas::TestExecutableInfo> testExecutables() const;

private:
    
     /*! Read a single CTestCTestfile.cmake.
      *  @p subDirs: parsed subdirectories output parameter
      *  @p tests: parsed tests. output parameter */
     void readCTestfile(KUrl::List& subDirs, QList<Veritas::TestExecutableInfo>& tests);
     
     KUrl processSubDirLine(const QString& line);
     Veritas::TestExecutableInfo processAddTestLine(const QString& line);

private:
    FilesystemAccess* m_dirAccess;
    QList<Veritas::TestExecutableInfo> m_testExes;
};

}

#endif // VERITAS_CTESTFILEPARSER_H
