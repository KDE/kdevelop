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

#include <QtCore/QObject>

#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QMutex>
#include <QtCore/QHash>
#include <QtCore/QPointer>
#include <kurl.h>

#include <languageexport.h>

namespace ThreadWeaver
{
class Weaver;
class Job;
}

namespace KTextEditor
{
class Document;
}

class QMutex;

namespace KDevelop
{
class ILanguageSupport;
class ParseJob;
class ParserDependencyPolicy;

using namespace ThreadWeaver;

class KDEVPLATFORMLANGUAGE_EXPORT BackgroundParser : public QObject
{
    Q_OBJECT
public:
    explicit BackgroundParser(ILanguageSupport *languageSupport, QObject* parent = 0 );
    virtual ~BackgroundParser();

    /**
     * Abort or dequeue all current running jobs with the specified \a parent.
     */
    void clear(QObject* parent);

    /**
     * Queries the background parser as to whether there is currently
     * a parse job for \a document, and if so, returns it.
     *
     * This may not contain all of the parse jobs that are intended
     * unless you call in from your job's ThreadWeaver::Job::aboutToBeQueued()
     * function.
     */
    ParseJob* parseJobForDocument(const KUrl& document) const;

    /**
     * The dependency policy which applies to all jobs (it is
     * applied automatically).
     */
    ParserDependencyPolicy* dependencyPolicy() const;

public Q_SLOTS:
    void suspend();
    void resume();
    void setDelay( int msecs );
    void setThreads( int threads );

    /**When addDocument is called with a document that is currently
     * being processed in a parse-job, the document will be parsed
     * again once the job has finished. */
    void addDocument( const KUrl &url );
//     void addDocument( Document *document );
    void addDocumentList( const KUrl::List &urls );
    void removeDocument( const KUrl &url );
//     void removeDocument( Document *document );

    void parseDocuments();

private Q_SLOTS:
    void parseComplete( Job *job );
//     void documentChanged( KTextEditor::Document *document );

protected:
    virtual void loadSettings( bool projectIsLoaded );
    virtual void saveSettings( bool projectIsLoaded );
    virtual void initialize();
    virtual void cleanup();

Q_SIGNALS:
    void requestAddDocument(const QUrl& url);



private:
    Q_PRIVATE_SLOT( d, void acceptAddDocument(const QUrl& url))
    class BackgroundParserPrivate* const d;
};

}
#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
