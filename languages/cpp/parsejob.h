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

#include "kdevparsejob.h"

#include <QStringList>

class CodeModel;
class TranslationUnitAST;
class CppLanguageSupport;
class ParseSession;
class ParseJob;

class CPPParseJob : public KDevParseJob
{
    Q_OBJECT
public:
    CPPParseJob( const KUrl &url,
              CppLanguageSupport* parent );

    CPPParseJob( KDevDocument* document,
              CppLanguageSupport* parent );

    virtual ~CPPParseJob();

    CppLanguageSupport* cpp() const;

    ParseSession* parseSession() const;

    void setAST(TranslationUnitAST* ast);
    virtual KDevAST *AST() const;

    void setCodeModel(CodeModel* model);
    virtual KDevCodeModel *codeModel() const;

    void setDUChain(TopDUContext* duChain);
    virtual TopDUContext* duChain() const;

    void setReadFromDisk(bool readFromDisk);
    bool wasReadFromDisk() const;

    void addIncludedFile(const QString& filename);
    const QStringList& includedFiles() const;
    void requestDependancies();

    ParseJob* parseJob() const;

private:
    ParseSession* m_session;
    TranslationUnitAST *m_AST;
    CodeModel *m_model;
    QStringList m_includedFiles;
    TopDUContext* m_duContext;
    bool m_readFromDisk;
    ParseJob* m_parseJob;
};

class ParseJob : public ThreadWeaver::Job
{
    Q_OBJECT
public:
    ParseJob(CPPParseJob* parent);

    CPPParseJob* parentJob() const;

protected:
    virtual void run();
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
