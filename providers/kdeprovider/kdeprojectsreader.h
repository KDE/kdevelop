/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEPROJECTSREADER_H
#define KDEPROJECTSREADER_H
#include "kdeprojectsmodel.h"
#include <QNetworkReply>
#include <QStack>

class KDEProjectsModel;
class KDEProjectsReader : public QObject
{
    Q_OBJECT
    public:
        KDEProjectsReader(KDEProjectsModel* m, QObject* parent);
        QStringList errors() const { return m_errors; }
        bool hasErrors() { return !m_errors.isEmpty(); }
        
    public slots:
        void downloadFinished(QNetworkReply* reply);
        void downloadError(QNetworkReply::NetworkError);
        
    signals:
        void downloadDone();
        
    private:
        QStack<Source> m_current;
        QStringList m_errors;
        KDEProjectsModel* m_m;
};

#endif // KDEPROJECTSREADER_H
