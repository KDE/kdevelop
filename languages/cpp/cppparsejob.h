/*
* This file is part of KDevelop
*
* Copyright 2006 Adam Treat <treat@kde.org>
* Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
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

#ifndef PARSEJOB_H
#define PARSEJOB_H

#include <parsejob.h>

#include <QStringList>

#include <ksharedptr.h>
#include <ktexteditor/range.h>

#include <duchainpointer.h>
#include <contextbuilder.h>

class PreprocessJob;
class CppLanguageSupport;
class ParseSession;
class CPPInternalParseJob;

namespace Cpp {
    class EnvironmentFile;
}

namespace KDevelop {
    class TopDUContext;
}

class CPPParseJob : public KDevelop::ParseJob
{
    Q_OBJECT
public:
    /**
     * @param parentPreprocessor May be the preprocess-job that created this parse-job.
     * Defined macros will be imported from that preprocess-job.
     * If parentPreprocessor is set, no jobs will be automatically created, since everything should be parsed in foreground.
     * Instead the preprocessor should call parseForeground();
     * */
    CPPParseJob( const KUrl &url, CppLanguageSupport* parent, PreprocessJob* parentPreprocessor = 0  );

//  CPPParseJob( KDevelop::Document* document, CppLanguageSupport* parent );

    virtual ~CPPParseJob();

    ///When the parse-job was initialized with a parent-preprocessor, the parent-preprocessor should call this to do the parsing
    void parseForeground();

    CppLanguageSupport* cpp() const;

    ParseSession* parseSession() const;

    void setReadFromDisk(bool readFromDisk);
    bool wasReadFromDisk() const;

    void addIncludedFile(KDevelop::TopDUContext* duChain, int sourceLine);
    const IncludeFileList& includedFiles() const;

    const QStack<DocumentCursor>& includeStack() const;
    void setIncludeStack(const QStack<DocumentCursor>& includeStack);

    ///Returns the preprocessor-job that is parent of this job, or 0
    PreprocessJob* parentPreprocessor() const;

    const QList<HashedString>& includePaths() const;
    const KUrl::List& includePathUrls() const;
    
    void requestDependancies();

    CPPInternalParseJob* parseJob() const;

    const KTextEditor::Range& textRangeToParse() const;

    /**
     * Get/set the environment-file of the proxy-context, if simpified matching is used.
     * When simplified-matching is used, two separate contexts will be created, with separate environment-descriptions.
     * */
    void setProxyEnvironmentFile( Cpp::EnvironmentFile* file );
    /**
     * Returns the environment-file to be used for the proxy context.
     * @return Pointer or zero if simpified matching is disabled.
     * */
    Cpp::EnvironmentFile* proxyEnvironmentFile();

    /**
     * Set the EnvironmentFile for the content-context. */
    void setContentEnvironmentFile( Cpp::EnvironmentFile* file );
    /**
     * Environment-file for the content. This is always valid.
     * */
    Cpp::EnvironmentFile* contentEnvironmentFile();

    ///Set this to true if the existing du-chain of the parsed file should be left as it is, without updating.
    void setKeepDuchain(bool b);
    bool keepDuchain() const;

    ///Proxy-context that is being updated, or zero.
    void setUpdatingProxyContext( const KDevelop::TopDUContextPointer& context );
    KDevelop::TopDUContextPointer updatingProxyContext() const;

    void setUpdatingContentContext( const KDevelop::TopDUContextPointer& context );
    ///If this is set, the updatingContentContext should either be used without modification, or updated if it is outdated.
    KDevelop::TopDUContextPointer updatingContentContext() const;

    ///If this file was included from another, this contains the path within the search-paths that this file was found through
    KUrl includedFromPath() const;
    void setIncludedFromPath( const KUrl& path );

    //Returns the master parse-job, which means the one that was not issued as an include-file
    const CPPParseJob* masterJob() const;
    CPPParseJob* masterJob();

    void addPreprocessorProblem(const ProblemPointer problem);
    QList<ProblemPointer> preprocessorProblems() const;

    ///Whether every single context encountered needs an update
    bool needUpdateEverything() const;
    ///Set whether every single context encountered needs an update
    void setNeedUpdateEverything(bool);

    bool wasUpdated(const KDevelop::DUContext* context) const;
    void setWasUpdated(const KDevelop::DUContext* context);

    const QSet<const KDevelop::DUContext*>& updated() const;
    
private:
    bool m_needUpdateEverything;
    KSharedPtr<Cpp::EnvironmentFile> m_proxyEnvironmentFile;
    PreprocessJob* m_parentPreprocessor;
    ParseSession* m_session;
    bool m_readFromDisk;
    PreprocessJob* m_preprocessJob;
    CPPInternalParseJob* m_parseJob;
    KTextEditor::Range m_textRangeToParse;
    IncludeFileList m_includedFiles;

    KDevelop::TopDUContextPointer m_updatingProxyContext;
    
    //The following two members are used when simplified-matching is used, which means that one content-context and one specialized context will be used.
    KDevelop::TopDUContextPointer m_updatingContentContext;
    KSharedPtr<Cpp::EnvironmentFile> m_contentEnvironmentFile;

    mutable QList<ProblemPointer> m_preprocessorProblems;
  
    KUrl m_includedFromPath;
    mutable bool m_includePathsComputed;
    mutable QList<HashedString> m_includePaths; //Only a master-job has this set
    mutable KUrl::List m_includePathUrls; //Only a master-job has this set
    bool m_keepDuchain;
    QStack<DocumentCursor> m_includeStack;
    QSet<const KDevelop::DUContext*> m_updated;
};

class CPPInternalParseJob : public ThreadWeaver::Job
{
    Q_OBJECT
public:
    CPPInternalParseJob(CPPParseJob* parent);

    CPPParseJob* parentJob() const;

    virtual int   priority () const;
    void setPriority(int priority);

    //Must only be called for direct parsing when the job is not queued
    virtual void run();

private:
    bool needUses() const;
    int m_priority;
};

#endif

