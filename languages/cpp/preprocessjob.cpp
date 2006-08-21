/*
* This file is part of KDevelop
*
* Copyright (c) 2006 Adam Treat <treat@kde.org>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "preprocessjob.h"

#include <QFile>
#include <QByteArray>

#include <kdebug.h>
#include <klocale.h>

#include "kdevdocument.h"

#include "parser/parsesession.h"
#include "parsejob.h"

PreprocessJob::PreprocessJob(CPPParseJob * parent)
    : ThreadWeaver::Job(parent)
{
}

CPPParseJob * PreprocessJob::parentJob() const
{
    return static_cast<CPPParseJob*>(const_cast<QObject*>(parent()));
}

void PreprocessJob::run()
{
    bool readFromDisk = !parentJob()->openDocument();

    QString contents;

    if ( readFromDisk )
    {
        QFile file( parentJob()->document().path() );
        if ( !file.open( QIODevice::ReadOnly ) )
        {
            parentJob()->setErrorMessage(i18n( "Could not open file '%1'", parentJob()->document().path() ));
            kWarning( 9007 ) << k_funcinfo << "Could not open file " << parentJob()->document() << " (path " << parentJob()->document().path() << ")" << endl;
            return ;
        }

        QByteArray fileData = file.readAll();
        contents = QString::fromUtf8( fileData.constData() );
        Q_ASSERT( !contents.isEmpty() );
        file.close();
    }
    else
    {
        contents = parentJob()->contentsFromEditor();
    }

    kDebug( 9007 ) << "===-- PARSING --===> "
    << parentJob()->document().fileName()
    << " <== readFromDisk: " << readFromDisk
    << " size: " << contents.length()
    << endl;

    Preprocessor preprocessor;
    QStringList includes;
    //     if ( true )
    //     {
    //         includes.append ("/usr/include");
    //         includes.append ("/usr/lib/gcc/i586-suse-linux/4.0.2/include");
    //         includes.append ("/usr/include/c++/4.0.2");
    //         includes.append ("/usr/include/c++/4.0.2/i586-suse-linux");
    //     }
    includes.append ( "." );
    preprocessor.addIncludePaths( includes );

    parentJob()->parseSession()->setContents( preprocessor.processString( contents ).toUtf8() );
    parentJob()->parseSession()->macros = preprocessor.macros();
}

Stream* PreprocessJob::sourceNeeded(QString& fileName, IncludeType type)
{
    Q_UNUSED(type);

    // need build system support now.

    return 0;
}

#include "preprocessjob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
