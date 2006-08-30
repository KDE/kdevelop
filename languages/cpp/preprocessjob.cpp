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

//#include <valgrind/memcheck.h>

#include <QFile>
#include <QByteArray>
#include <QMutexLocker>

#include <kdebug.h>
#include <klocale.h>

#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevpersistenthash.h>
#include <kdevbackgroundparser.h>

#include "Thread.h"

#include "cpplanguagesupport.h"
#include "parser/parsesession.h"
#include "parsejob.h"
#include "parser/ast.h"
#include "parser/parsesession.h"

PreprocessJob::PreprocessJob(CPPParseJob * parent)
    : ThreadWeaver::Job(parent)
    , m_success(true)
{
}

CPPParseJob * PreprocessJob::parentJob() const
{
    return static_cast<CPPParseJob*>(const_cast<QObject*>(parent()));
}

void PreprocessJob::run()
{
    //kDebug() << k_funcinfo << "Started pp job " << this << " parse " << parentJob()->parseJob() << " parent " << parentJob() << endl;

    if (checkAbort())
        return;

    QMutexLocker lock(parentJob()->cpp()->parseMutex(thread()));

    bool readFromDisk = !parentJob()->contentsAvailableFromEditor();
    parentJob()->setReadFromDisk(readFromDisk);

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

    kDebug( 9007 ) << "===-- PREPROCESSING --===> "
    << parentJob()->document().fileName()
    << " <== readFromDisk: " << readFromDisk
    << " size: " << contents.length()
    << endl;

    if (checkAbort())
        return;

    parentJob()->parseSession()->setContents( processString( contents ).toUtf8() );
    parentJob()->parseSession()->macros = macros();
}

Stream* PreprocessJob::sourceNeeded(QString& fileName, IncludeType type)
{
    Q_UNUSED(type);

    // FIXME change to interruptable preprocessor

    if (checkAbort())
        return 0;

    bool dependencyAdded = false;
    bool dependencyAllowed = true;

    KUrl includedFile;

/*    if (CPPParseJob* parent = parentJob()) {
        VALGRIND_CHECK_MEM_IS_DEFINED(parent, sizeof(CPPParseJob));

        if (CppLanguageSupport* lang = parent->cpp()) {
            VALGRIND_CHECK_MEM_IS_DEFINED(lang, sizeof(CppLanguageSupport)); */


    includedFile = parentJob()->cpp()->findInclude(fileName);
    if (includedFile.isValid()) {
        if (KDevParseJob* job = parentJob()->backgroundParser()->parseJobForDocument(includedFile)) {
            /*if (job == parentJob())
                // Trying to include self
                goto done;

            if (!job->isFinished()) {
                dependencyAllowed = parentJob()->addDependency(job, parentJob()->parseJob());

                kDebug() << k_funcinfo << "Added dependency on job " << job << " to " << parentJob() << " pp " << this << " parse " << parentJob()->parseJob() << " success " << dependencyAllowed << endl;
            }

            dependencyAdded = true;*/
        }
    }

        /*} else {
            kWarning() << k_funcinfo << "Language support disappeared!!" << endl;
        }
    } else {
        kWarning() << k_funcinfo << "Parent job disappeared!!" << endl;
    }*/

    if (KDevCore::activeProject()) {
        KDevAST* ast = 0;
        if (includedFile.isValid())
            ast = KDevCore::activeProject()->persistentHash()->retrieveAST(includedFile);
        if (!ast)
            ast = KDevCore::activeProject()->persistentHash()->retrieveAST(fileName);

        if (ast) {
            TranslationUnitAST* t = static_cast<TranslationUnitAST*>(ast);
            addMacros(t->session->macros);

        } else if (!dependencyAdded && dependencyAllowed && includedFile.isValid()) {
            parentJob()->backgroundParser()->addDocument(includedFile);
            KDevParseJob* job = parentJob()->backgroundParser()->parseJobForDocument(includedFile);
            /*if (job == parentJob())
                // Trying to include self
                goto done;

            if (job && !job->isFinished()) {
                dependencyAllowed = parentJob()->addDependency(job, parentJob()->parseJob());
                kDebug() << k_funcinfo << "Added dependency on job " << job << " to " << parentJob() << " pp " << this << " parse " << parentJob()->parseJob() << " success " << dependencyAllowed << endl;
            }*/
        }
    }

    done:

    parentJob()->addIncludedFile(fileName);

    return 0;
}

bool PreprocessJob::checkAbort()
{
    if (CPPParseJob* parent = parentJob()) {
        if (parent->abortRequested()) {
            parent->abortJob();
            m_success = false;
            setFinished(true);
            return true;
        }

    } else {
        // What... the parent job got deleted??
        kWarning() << k_funcinfo << "Parent job disappeared!!" << endl;
        m_success = false;
        setFinished(true);
        return true;
    }

    return false;
}

bool PreprocessJob::success() const
{
    return m_success;
}

#include "preprocessjob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
