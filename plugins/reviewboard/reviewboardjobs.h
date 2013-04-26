/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#ifndef KDEVPLATFORM_PLUGIN_REVIEWBOARDJOBS_H
#define KDEVPLATFORM_PLUGIN_REVIEWBOARDJOBS_H

#include <KJob>
#include <KUrl>
#include <QList>
#include <QPair>
#include <QNetworkAccessManager>

class QNetworkReply;

namespace ReviewBoard
{
    /**
     * Post call to the specified service.
     * Converts returned json data to a QVariant to be used from actual API calls
     * 
     * @note It is reviewboard-agnostic.
     */
    class HttpPostCall : public KJob
    {
        Q_OBJECT
        public:
            HttpPostCall(const KUrl& s, const QString& apiPath, const QList<QPair<QString,QString> >& queryParameters, const QByteArray& post, bool multipart, QObject* parent);
            
            virtual void start();
            
            QVariant result() const;
            
        private slots:
            void finished();
            
        private:
            QVariant m_result;
            QNetworkReply* m_reply;
            KUrl m_requrl;
            QByteArray m_post;
            
            QNetworkAccessManager m_manager;
            bool m_multipart;
    };
    
    class NewRequest : public KJob
    {
        Q_OBJECT
        public:
            NewRequest(const KUrl& server, const KUrl& patch, const QString& project, const QString& basedir, QObject* parent = 0);
            
            virtual void start();
            QString requestId() const;
            
        private slots:
            void submitPatch();
            void done();
            
        private:
            KUrl m_server;
            HttpPostCall* m_newreq;
            
            KUrl m_patch;
            QString m_basedir;
            QString m_id;
            HttpPostCall* m_uploadpatch;
            QString m_project;
    };
    
    class ProjectsListRequest : public KJob
    {
        Q_OBJECT
        public:
            ProjectsListRequest(const KUrl& server, QObject* parent);
            virtual void start();
            QVariantList repositories() const;
    
        private slots:
            void requestRepositoryList(int startingIndex);
            void done(KJob*);
            
        private:
            KUrl m_server;
            QVariantList m_repositories;
    };
    
    QByteArray urlToData(const KUrl&);
}

#endif
