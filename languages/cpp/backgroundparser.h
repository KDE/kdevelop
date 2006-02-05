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

#ifndef KDEVBACKGROUNDPARSER_H
#define KDEVBACKGROUNDPARSER_H

#include <QObject>

#include <kurl.h>
#include <QMap>
#include <QMutex>

namespace ThreadWeaver
{
class Weaver;
class Job;
}

namespace KTextEditor
{
class Document;
}

class QTimer;

class pool;
class Parser;
class Preprocessor;
class TranslationUnitAST;
class CppLanguageSupport;

using namespace ThreadWeaver;

class BackgroundParser : public QObject
{
    Q_OBJECT
public:
    BackgroundParser( CppLanguageSupport* cppSupport );
    virtual ~BackgroundParser();

public slots:
    void suspend();
    void resume();
    void addDocument( const KUrl &url );
    void removeDocument( const KUrl &url );
    void parseDocuments();
    void parseComplete( Job* );
    void documentChanged( KTextEditor::Document* document );

private:
    CppLanguageSupport *m_cppSupport;
    pool *m_memoryPool;
    QTimer *m_timer;
    bool m_suspend;

    // A list of known documents, and whether they are due to be parsed or not
    QMap<KUrl, bool> m_documents;
    QMap<KUrl, TranslationUnitAST* > m_url2unit;
    mutable QMutex m_mutex;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
