/*
* This file is part of KDevelop
*
* Copyright 2006 Adam Treat <treat@kde.org>
* Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
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
#ifndef CPPPARSEJOB_H
#define CPPPARSEJOB_H

#include <language/backgroundparser/parsejob.h>
#include <util/path.h>

#include <QStringList>
#include <QWaitCondition>

#include <KTextEditor/Range>

#include "parser/parsesession.h"
#include "cppduchain/contextbuilder.h"

class PreprocessJob;
class CppLanguageSupport;
class CppPreprocessEnvironment;
class CPPInternalParseJob;

namespace Cpp {
    class EnvironmentFile;
}

namespace KDevelop {
    class DocumentCursor;
    class ILanguageSupport;
    class IndexedString;
    class TopDUContext;
}

class IncludePathComputer;

class CPPParseJob: public KDevelop::ParseJob
{
    Q_OBJECT
public:
  
    typedef QPair<CPPParseJob*, int> LineJobPair;
  
    /**
     * @param parentPreprocessor May be the preprocess-job that created this parse-job.
     * Defined macros will be imported from that preprocess-job.
     * If parentPreprocessor is set, no jobs will be automatically created, since everything should be parsed in foreground.
     * Instead the preprocessor should call parseForeground();
     * */
    CPPParseJob( const KDevelop::IndexedString &url, KDevelop::ILanguageSupport* languageSupport, PreprocessJob* parentPreprocessor = 0  );

//  CPPParseJob( KDevelop::Document* document, CppLanguageSupport* parent );

    virtual ~CPPParseJob();

    ///When the parse-job was initialized with a parent-preprocessor, the parent-preprocessor should call this to do the parsing
    void parseForeground();

    CppLanguageSupport* cpp() const;

    ParseSession::Ptr parseSession() const;

    void setReadFromDisk(bool readFromDisk);
    bool wasReadFromDisk() const;

    void addIncludedFile(KDevelop::ReferencedTopDUContext duChain, int sourceLine);
    const IncludeFileList& includedFiles() const;

    ///When this parse-job is ready, the resulting top-context will be imported into the given one.
    ///This can be used to delay the importing in case of recursion
    void addDelayedImporter(LineContextPair duChain);
    
    ///When this parse-job is ready, import the result of the given job into the current top-context
    void addDelayedImport(LineJobPair job);

    ///Returns the preprocessor-job that is parent of this job, or 0
    PreprocessJob* parentPreprocessor() const;

    //Feedback from CppLanguageSupport, don't use for other purposes
    void gotIncludePaths(IncludePathComputer* comp);
    
    void requestDependancies();

    QSharedPointer<CPPInternalParseJob> parseJob() const;

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

    /**
     * Returns the minimum required features for slave contexts.
     * These features also must be satisfied on the main context,
     * as the requirements may be recursive.
     */
    TopDUContext::Features slaveMinimumFeatures() const;
    
    ///Set this to true if the existing du-chain of the parsed file should be left as it is, without updating.
    void setKeepDuchain(bool b);
    bool keepDuchain() const;

    ///Proxy-context that is being updated, or zero.
    void setUpdatingProxyContext( const KDevelop::ReferencedTopDUContext& context );
    KDevelop::ReferencedTopDUContext updatingProxyContext() const;

    void setUpdatingContentContext( const KDevelop::ReferencedTopDUContext& context );
    ///If this is set, the updatingContentContext should either be used without modification, or updated if it is outdated.
    KDevelop::ReferencedTopDUContext updatingContentContext() const;

    ///If this file was included from another, this contains the path within the search-paths that this file was found through
    KDevelop::Path includedFromPath() const;
    void setIncludedFromPath( const KDevelop::Path& path );

    KDevelop::Path localPath() const;

    //Returns the master parse-job, which means the one that was not issued as an include-file
    const CPPParseJob* masterJob() const;
    CPPParseJob* masterJob();

    void addPreprocessorProblem(const ProblemPointer problem);
    QList<ProblemPointer> preprocessorProblems() const;

    QList<ProblemPointer>* preprocessorProblemsPointer();
    
    ///Whether every single context encountered needs an update
    bool needUpdateEverything() const;
    ///Set whether every single context encountered needs an update
    void setNeedUpdateEverything(bool);

    bool wasUpdated(const KDevelop::DUContext* context) const;
    void setWasUpdated(const KDevelop::DUContext* context);

    const QSet<const KDevelop::DUContext*>& updated() const;

    //Can be called to indicate that an included file was parsed
    void includedFileParsed();

    //Sets local progress for this exact file
    void setLocalProgress(float progress, QString text);
    
    void setNeedsUpdate(bool needs);
    bool needsUpdate() const;
    
    //Called when the internal parse-job is complete, to apply the delayed imports/importers
    void processDelayedImports();
    
    KDevelop::ModificationRevisionSet includePathDependencies() const ;
    
    virtual ControlFlowGraph* controlFlowGraph() override;
    virtual DataAccessRepository* dataAccessInformation() override;
private:

    QList<LineJobPair> m_delayedImports;
    QList<LineContextPair> m_delayedImporters;
    
    friend class PreprocessJob; //So it can access includePaths()
    //Only use from within the background thread, and make sure no mutexes are locked when calling it
    const QList<IndexedString>& indexedIncludePaths() const;
    //Only use from within the background thread, and make sure no mutexes are locked when calling it
    const KDevelop::Path::List& includePathUrls() const;

    //Merges the macro-definitions into the given environment
    void mergeDefines(CppPreprocessEnvironment& env) const;
  
    bool m_needUpdateEverything;
    QExplicitlySharedDataPointer<Cpp::EnvironmentFile> m_proxyEnvironmentFile;
    PreprocessJob* m_parentPreprocessor;
    ParseSession::Ptr m_session;
    ThreadWeaver::JobPointer m_preprocessJob;
    ThreadWeaver::JobPointer m_parseJob;
    KTextEditor::Range m_textRangeToParse;
    IncludeFileList m_includedFiles;

    KDevelop::ReferencedTopDUContext m_updatingProxyContext;
    
    //The following two members are used when simplified-matching is used, which means that one content-context and one specialized context will be used.
    KDevelop::ReferencedTopDUContext m_updatingContentContext;
    QExplicitlySharedDataPointer<Cpp::EnvironmentFile> m_contentEnvironmentFile;

    mutable QList<ProblemPointer> m_preprocessorProblems;
  
    KDevelop::Path m_includedFromPath;
    KDevelop::Path m_localPath;
    mutable IncludePathComputer* m_includePathsComputed;
    mutable QList<IndexedString> m_includePaths; //Only a master-job has this set
    mutable KDevelop::Path::List m_includePathUrls; //Only a master-job has this set
    bool m_keepDuchain;
    QSet<const KDevelop::DUContext*> m_updated;
    int m_parsedIncludes;
    mutable QMutex m_waitForIncludePathsMutex;
    mutable QWaitCondition m_waitForIncludePaths;
    bool m_needsUpdate;
};

class CPPInternalParseJob : public ThreadWeaver::Job
{
public:
    CPPInternalParseJob(CPPParseJob* parent);

    CPPParseJob* parentJob() const;

    virtual int   priority () const override;
    void setPriority(int priority);

    //Must only be called for direct parsing when the job is not queued
    virtual void run(ThreadWeaver::JobPointer pointer, ThreadWeaver::Thread* thread) override;

    //Called as soon as the first updated context has been set
    void initialize();

    ReferencedTopDUContext proxyContext;
    ReferencedTopDUContext contentContext;
private:
    //Duchain must not be locked
    void highlightIfNeeded();

    ReferencedTopDUContext updatingProxyContext;
    ReferencedTopDUContext updatingContentContext;

    CPPParseJob* m_parentJob;
    bool m_initialized;
    int m_priority;
};

#endif

