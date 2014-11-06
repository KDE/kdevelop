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

#include <QUrl>
#include <KJob>
#include <QList>
#include <QPair>
#include <QNetworkAccessManager>

class QNetworkReply;

namespace ReviewBoard
{
    /**
     * Http call to the specified service.
     * Converts returned json data to a QVariant to be used from actual API calls
     *
     * @note It is reviewboard-agnostic.
     */
    class HttpCall : public KJob
    {
        Q_OBJECT
        public:
            HttpCall(const QUrl& s, const QString& apiPath, const QList<QPair<QString,QString> >& queryParameters, const QByteArray& post, bool multipart, QObject* parent);

            virtual void start();

            QVariant result() const;

        private slots:
            void onFinished();

        private:
            QVariant m_result;
            QNetworkReply* m_reply;
            QUrl m_requrl;
            QByteArray m_post;

            QNetworkAccessManager m_manager;
            bool m_multipart;
    };

    class ReviewRequest : public KJob
    {
        Q_OBJECT
        public:
            ReviewRequest(const QUrl& server, const QString& id, QObject* parent)
                          : KJob(parent), m_server(server), m_id(id) {}
            QString requestId() const { return m_id; }
            void setRequestId(QString id) { m_id = id; }
            QUrl server() const { return m_server; }

        private:
            QUrl m_server;
            QString m_id;
    };

    class NewRequest : public ReviewRequest
    {
        Q_OBJECT
        public:
            NewRequest(const QUrl& server, const QString& project, QObject* parent = 0);
            virtual void start();

        private slots:
            void done();

        private:
            HttpCall* m_newreq;
            QString m_project;
    };

    class SubmitPatchRequest : public ReviewRequest
    {
        Q_OBJECT
        public:
            SubmitPatchRequest(const QUrl &server, const QUrl& patch, const QString& basedir, const QString& id, QObject* parent = 0);
            virtual void start();

        private slots:
            void done();

        private:
            HttpCall* m_uploadpatch;
            QUrl m_patch;
            QString m_basedir;
    };

    class ProjectsListRequest : public KJob
    {
        Q_OBJECT
        public:
            ProjectsListRequest(const QUrl &server, QObject* parent = 0);
            virtual void start();
            QVariantList repositories() const;

        private slots:
            void requestRepositoryList(int startIndex);
            void done(KJob* done);

        private:
            QUrl m_server;
            QVariantList m_repositories;
    };

    class ReviewListRequest : public KJob
    {
        Q_OBJECT
        public:
            ReviewListRequest(const QUrl& server, const QString& user, const QString& reviewStatus, QObject* parent = 0);
            virtual void start();
            QVariantList reviews() const;

        private slots:
            void requestReviewList(int startIndex);
            void done(KJob* done);

        private:
            QUrl m_server;
            QString m_user;
            QString m_reviewStatus;
            QVariantList m_reviews;
    };

    QByteArray urlToData(const QUrl&);
}

#endif
