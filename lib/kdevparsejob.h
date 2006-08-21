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

#ifndef KDEVPARSEJOB_H
#define KDEVPARSEJOB_H

#include <JobSequence.h>
#include <kurl.h>

class KDevDocument;

class QByteArray;

namespace KTextEditor
{
class SmartRange;
}

class KDevAST;
class KDevCodeModel;
class DUContext;

class KDevParseJob : public ThreadWeaver::JobSequence
{
    Q_OBJECT
public:
    KDevParseJob( const KUrl &url, QObject *parent );

    KDevParseJob( KDevDocument *document,
                  QObject* parent );

    virtual ~KDevParseJob();

    void setActiveDocument();
    virtual int priority() const;

    QString contentsFromEditor() const;

    KUrl document() const;
    KDevDocument* openDocument() const;
    bool wasSuccessful() const;

    void setErrorMessage(const QString& message);
    const QString& errorMessage() const;

    virtual KDevAST *AST() const = 0;
    virtual KDevCodeModel *codeModel() const = 0;
    virtual DUContext *duChain() const;

protected:
    KUrl m_document;
    KDevDocument* m_openDocument;
    QString m_errorMessage;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
