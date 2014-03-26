/*
* This file is part of KDevelop
*
* Copyright 2006 Adam Treat <treat@kde.org>
* Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>
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

#ifndef PREPROCESSJOB_H
#define PREPROCESSJOB_H

#include <ksharedptr.h>
#include <threadweaver/job.h>

#include "parser/rpp/preprocessor.h"

namespace Cpp {
    class EnvironmentFile;
    typedef KSharedPtr<EnvironmentFile> EnvironmentFilePointer;
}

namespace KDevelop {
    class ParsingEnvironment;
}

namespace rpp {
    class pp;
}

class CPPParseJob;
class CppPreprocessEnvironment;

/**
 * The preprocess-job preprocesses the actual content and registers it at the parent-job parse-session.
 * Additional it sets CPPParseJob::contentEnvironmentFile, CPPParseJob::proxyEnvironmentFile,
 * CPPParseJob::updatingProxyContext, CPPParseJob::updatingContentContext, and CPPParseJob::keepDuchain.
 *
 * When simplified matching is enabled, all those members will be filled accordingly. Else, only contentEnvironmentFile
 * and updatingContentContext will be filled.
 * */

class PreprocessJob : public ThreadWeaver::Job, public rpp::Preprocessor
{
public:
    PreprocessJob(CPPParseJob* parent);
    ~PreprocessJob();

    CPPParseJob* parentJob() const;

    virtual rpp::Stream* sourceNeeded(QString& fileName, IncludeType type, int sourceLine, bool skipCurrentPath);

    virtual bool success() const;

    //Must only be called for direct parsing when the job is not queued
    virtual void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread);

    virtual void headerSectionEnded(rpp::Stream& stream);
    
    virtual void foundHeaderGuard(rpp::Stream& stream, KDevelop::IndexedString guardName);
    
    /**
     * Returns the standard-environment used for parsing c++ files(all other environments are based on that one)
     * The returned environment can be dynamic_casted to rpp::Environment.
     *
     * The returned environment is created exclusively for you, and you must destroy it.
     *
     * It can be used for matching, but it cannot be used for parsing.
     * */
    static KDevelop::ParsingEnvironment* createStandardEnvironment();

    static const KDevelop::ParsingEnvironment* standardEnvironment();
private:
    void headerSectionEndedInternal(rpp::Stream* stream);
    bool checkAbort();
    bool readContents();

    CPPParseJob* m_parentJob;
    CppPreprocessEnvironment* m_currentEnvironment;
    KSharedPtr<Cpp::EnvironmentFile> m_firstEnvironmentFile; //First environment-file. If simplified matching is used, this is the proxy.
    //If simplified matching is used, a separate EnvironmentFile is used for the content, as opposed to the #include statements.
    KSharedPtr<Cpp::EnvironmentFile> m_secondEnvironmentFile;
    //This is the environment-file that is currently updated. May be the proxy in the beginning, and context later after header-section ended.
    KSharedPtr<Cpp::EnvironmentFile> m_updatingEnvironmentFile;
    bool m_success;
    bool m_headerSectionEnded;
    rpp::pp* m_pp;
    QByteArray m_contents;

    static KDevelop::ParsingEnvironment* m_standardEnvironment;
};

KDevelop::ParsingEnvironment* CreateStandardEnvironment();
#endif

