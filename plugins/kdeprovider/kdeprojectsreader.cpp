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

#include "kdeprojectsreader.h"
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QXmlStreamReader>

static const QUrl kdeProjectsUrl(QStringLiteral("https://projects.kde.org/kde_projects.xml"));

KDEProjectsReader::KDEProjectsReader(KDEProjectsModel* m, QObject* parent)
    : QObject(parent)
    , m_m(m) //donuts
{
    auto *manager = new QNetworkAccessManager(this);

    QNetworkReply* reply = manager->get(QNetworkRequest(kdeProjectsUrl));
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &KDEProjectsReader::downloadError);
    connect(manager,&QNetworkAccessManager::finished, this, &KDEProjectsReader::downloadFinished);
}

static QString readText(QXmlStreamReader* xml)
{
    QString ret;
    for(int opened=1; opened>0 && !xml->atEnd(); ) {
        const auto token=xml->readNext();

        if(token==QXmlStreamReader::StartElement)
            opened++;
        else if(token==QXmlStreamReader::EndElement)
            opened--;
        else if(token==QXmlStreamReader::Characters)
            ret += xml->text();
    }
    return ret;
}

void KDEProjectsReader::downloadFinished(QNetworkReply* reply)
{
    if(reply->error()!=QNetworkReply::NoError) {
        m_errors += reply->errorString();
        emit downloadDone();
        return;
    }

    QByteArray data = reply->readAll();
    QXmlStreamReader xml(data);

    while (!xml.atEnd()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        if(token==QXmlStreamReader::StartElement) {
            QStringRef name = xml.name();

            if (name == QLatin1String("project") || name == QLatin1String("module")) {
                m_current.push(Source());

                if (name == QLatin1String("project"))
                    m_current.top().type = Source::Project;
                else if (name == QLatin1String("module"))
                    m_current.top().type = Source::Module;
                m_current.top().identifier = xml.attributes().value(QStringLiteral("identifier")).toString();
            }
            else if(!m_current.isEmpty())
            {
                if(name == QLatin1String("name"))
                    m_current.top().name = readText(&xml);
                else if(name == QLatin1String("url")) {
                    QString protocol = xml.attributes().value(QStringLiteral("protocol")).toString();
                    m_current.top().m_urls[protocol] = readText(&xml);
                } else if(name == QLatin1String("icon"))
                    m_current.top().icon = readText(&xml);
            }
        } else if(token==QXmlStreamReader::EndElement) {
            QStringRef name = xml.name();
            if (name == QLatin1String("project") || name == QLatin1String("module")) {
                Source p = m_current.pop();

                if (name == QLatin1String("project"))
                    Q_ASSERT(p.type == Source::Project);
                else if (name == QLatin1String("module"))
                    Q_ASSERT(p.type == Source::Module);

                if(!p.m_urls.isEmpty()) {
                    auto* item = new SourceItem(p);
                    m_m->appendRow(item);
                }
            }
        }
    }
    if (xml.hasError()) {
        m_errors += xml.errorString();
    }
    emit downloadDone();
}

void KDEProjectsReader::downloadError(QNetworkReply::NetworkError error)
{
    m_errors += QStringLiteral("error: %1").arg(error);
}
