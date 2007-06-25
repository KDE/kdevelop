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

#ifndef PARSEJOB_H
#define PARSEJOB_H

#include <parsejob.h>

#include <QStringList>

#include <ksharedptr.h>
#include <ktexteditor/range.h>

class PreprocessJob;
class TranslationUnitAST;
class CppLanguageSupport;
class ParseSession;
class CPPInternalParseJob;

namespace Cpp {
    class LexedFile;
};

namespace KDevelop {
    class TopDUContext;
}

typedef QList<KDevelop::TopDUContext*> IncludeFileList;

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

    void setAST(TranslationUnitAST* ast);
    virtual TranslationUnitAST *AST() const;

    void setDUChain(KDevelop::TopDUContext* duChain);
    virtual KDevelop::TopDUContext* duChain() const;

    void setReadFromDisk(bool readFromDisk);
    bool wasReadFromDisk() const;

    void addIncludedFile(KDevelop::TopDUContext* duChain);
    const IncludeFileList& includedFiles() const;

    ///Returns the preprocessor-job that is parent of this job, or 0
    PreprocessJob* parentPreprocessor() const;

    void requestDependancies();

    CPPInternalParseJob* parseJob() const;

    const KTextEditor::Range& textRangeToParse() const;

    void setLexedFile( Cpp::LexedFile* file );

    Cpp::LexedFile* lexedFile();
    
private:
    KSharedPtr<Cpp::LexedFile> m_lexedFile;
    PreprocessJob* m_parentPreprocessor;
    ParseSession* m_session;
    TranslationUnitAST* m_AST;
    KDevelop::TopDUContext* m_duContext;
    bool m_readFromDisk;
    PreprocessJob* m_preprocessJob;
    CPPInternalParseJob* m_parseJob;
    KTextEditor::Range m_textRangeToParse;
    IncludeFileList m_includedFiles;
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
    int m_priority;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
