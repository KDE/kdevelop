/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qfile.h>
#include <qstringlist.h>

#include <kurl.h>
#include <kdebug.h>

#include "urlutil.h"

#include "cvsentry.h"
#include "cvsutils.h"

///////////////////////////////////////////////////////////////////////////////

bool CvsUtils::isRegisteredInRepository( const QString &projectDirectory, const KURL &url )
{
    kdDebug(9000) << "===> CvsPart::isRegisteredInRepository() here! " << endl;

    if ( url.path() == projectDirectory )
    {
        kdDebug(9000) << "===> Operation requested for projectDir(): true. " << endl;
        return true;
    }

    QString dirName = url.directory();
    QString entriesFilePath = dirName + "/CVS/Entries";

    kdDebug(9000) << "===> pathUrl.path()      = " << url.path() << endl;
    kdDebug(9000) << "===> dirName             = " << dirName << endl;
    kdDebug(9000) << "===> entriesFilePath = " << entriesFilePath << endl;

    bool found = false;
    QFile f( entriesFilePath );
    if (f.open( IO_ReadOnly ))
    {
        QTextStream t( &f );
        CvsEntry cvsEntry;
        while (cvsEntry.read( t ) && !found)
        {
            if (cvsEntry.fileName == url.fileName())
            {
                kdDebug(9000) << "===> Wow!! *** Found it!!! *** " << endl;
                found = true;
            }
        }
    }
    else
    {
        kdDebug(9000) << "===> Error: could not open CVS/Entries!! " << endl;
    }
    f.close();

    return found;
}

///////////////////////////////////////////////////////////////////////////////

void CvsUtils::validateURLs( const QString &projectDirectory, KURL::List &urls, CvsPart::CvsOperation op )
{
    kdDebug(9000) << "CvsPart::validateURLs() here!" << endl;

    // If files are to be added, we can avoid to check them to see if they are registered in the
    // repository ;)
    if (op == CvsPart::opAdd)
    {
        kdDebug(9000) << "This is a Cvs Add operation and will not be checked against repository ;-)" << endl;
        return;
    }
    QValueList<KURL>::iterator it = urls.begin();
    while (it != urls.end())
    {
        if (!CvsUtils::isRegisteredInRepository( projectDirectory, (*it) ))
        {
            kdDebug(9000) << "Warning: file " << (*it).path() << " does NOT belong to repository and will not be used" << endl;

            it = urls.erase( it );
        }
        else
        {
            kdDebug(9000) << "Warning: file " << (*it).path() << " is in repository and will be accepted" << endl;

            ++it;
        }
    }

}

///////////////////////////////////////////////////////////////////////////////

void CvsUtils::addToIgnoreList( const QString &projectDirectory, const KURL &url )
{
    kdDebug(9000) << "===> CvsPart::addToIgnoreList() here! " << endl;

    if ( url.path() == projectDirectory )
    {
        kdDebug(9000) << "Can't add to ignore list current project directory " << endl;
        return;
    }

    QString dirName = url.directory();
    QString ignoreFilePath = dirName + "/.cvsignore";
    QString relPathName = url.fileName();

    kdDebug(9000) << "Adding file to ignore " << relPathName << " to " << ignoreFilePath << endl;

    QFile f( ignoreFilePath );
    if (f.open( IO_ReadOnly | IO_WriteOnly))
    {
        QTextStream t( &f );
        QString fileName;
        bool found = false;
        while (!t.eof() && !found)
        {
            fileName = t.readLine();
            kdDebug(9000) << "** Examining line: " << fileName << endl;
            found = fileName == relPathName;
        }
        if (!found) {
            t << relPathName << "\n";
            kdDebug(9000) << "** File is not present. So it can be added to " << ignoreFilePath << endl;
        }
    }
    f.close();
}

void CvsUtils::addToIgnoreList( const QString &projectDirectory, const KURL::List &urls )
{
    for (size_t i=0; i<urls.count(); ++i)
    {
        addToIgnoreList( projectDirectory, urls[i] );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsUtils::removeFromIgnoreList( const QString &/*projectDirectory*/, const KURL &url )
{
    kdDebug(9000) << "===> CvsPart::removeFromIgnoreList() here! " << endl;

    QStringList ignoreLines;

    QString dirName = url.directory();
    QString ignoreFilePath = dirName + "/.cvsignore";
    QString relPathName = url.fileName();

    // 1. Read all .ignore file in memory
    QFile f( ignoreFilePath );
    if (!f.open( IO_ReadOnly ))
        return; // No .cvsignore file? Nothing to do then!

    {
        QTextStream t( &f );
        while (!t.eof()) {
            QString line = t.readLine();
            kdDebug(9000) << "** Readin line: " << line << endl;
            ignoreLines << line;
        }
    }
    f.close();

    // 2. Look up for filename
    if (ignoreLines.remove( relPathName ) == 0) // Ok, file is not in list, so nothing to do!
    {
        kdDebug(9000) << "** File is not present in " << ignoreFilePath << "! So no point in removing it ..." << endl;
        return;
    }
    // 3. If present, remove it and write the lines in memory back to file!
    if (!f.open( IO_WriteOnly )) // Who nows
    {
        kdDebug(9000) << "Argh!! Could not open " << ignoreFilePath << " for appending the filename!!!" << endl;
        return; //
    }

    {
        QTextStream t( &f );
        for (size_t i=0; i<ignoreLines.count(); ++i)
        {
            t << ignoreLines[i] << "\n";
        }
    }
    f.close();
}

void CvsUtils::removeFromIgnoreList( const QString &projectDirectory, const KURL::List &urls )
{
    for (size_t i=0; i<urls.count(); ++i)
    {
        removeFromIgnoreList( projectDirectory, urls[i] );
    }
}

