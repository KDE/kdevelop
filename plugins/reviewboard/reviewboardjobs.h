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

#ifndef REVIEWBOARDJOBS_H
#define REVIEWBOARDJOBS_H

#include <KJob>
#include <KUrl>

namespace KIO {
    class TransferJob;
    class Job;
}

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
            HttpPostCall(const KUrl& server, const QString& apiPath, const QByteArray& post, QObject* parent = 0);
            
            virtual void start();
            
            QVariant result() const;
            
        private slots:
            void data(KIO::Job*, const QByteArray& data);
            void processData(KJob* );
            
        private:
            KIO::TransferJob* requestJob;
            QByteArray receivedData;
            QVariant m_result;
    };
    
    class NewRequest : public KJob
    {
        Q_OBJECT
        public:
            NewRequest(const KUrl& server, QObject* parent = 0);
            
            virtual void start();
            
        private slots:
            void second();
            void third();
            
        private:
            KUrl m_server;
            HttpPostCall* m_repositories;
            HttpPostCall* m_newreq;
    };
}

#endif
