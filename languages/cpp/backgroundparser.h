/*
 * KDevelop C++ Background Parser
 *
 * Copyright (c) 2005 Adam Treat <treat@kde.org>
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
class Preprocessor;
class Control;
class Parser;
class pool;
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
    void addDocument( const KURL &url );
    void removeDocument( const KURL &url );
    void parseDocuments();
    void parseComplete( Job* );
    void documentChanged( KTextEditor::Document* document );

private:
    CppLanguageSupport *m_cppSupport;
    Weaver *m_weaver;

    // A list of known documents, and whether they are due to be parsed or not
    QMap<KURL, bool> m_documents;
    QTimer *m_timer;

    Preprocessor *m_preprocessor;
    Control *m_control;
    Parser *m_parser;
    pool *m_memoryPool;
    QMap<KURL, TranslationUnitAST*> m_url2unit;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
