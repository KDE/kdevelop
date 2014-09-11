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
#include <QDebug>
#include <vcs/vcslocation.h>

static QUrl kdeProjectsUrl("https://projects.kde.org/kde_projects.xml");

KDEProjectsReader::KDEProjectsReader(KDEProjectsModel* m, QObject* parent)
    : QObject(parent)
    , m_m(m) //donuts
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QNetworkReply* reply = manager->get(QNetworkRequest(kdeProjectsUrl));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(downloadError(QNetworkReply::NetworkError)));
    connect(manager,SIGNAL(finished(QNetworkReply*)), SLOT(downloadFinished(QNetworkReply*)));
}

QString readText(QXmlStreamReader* xml)
{
    QString ret;
    QXmlStreamReader::TokenType token;
    
    for(int opened=1; opened>0 && !xml->atEnd(); ) {
        token=xml->readNext();
        
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
            
            if(name == "project" || name == "module")
            {
                m_current.push(Source());

                if (name == "project")
                    m_current.top().type = Source::Project;
                else if (name == "module")
                    m_current.top().type = Source::Module;
                m_current.top().identifier = xml.attributes().value("identifier").toString();
            }
            else if(!m_current.isEmpty())
            {
                if(name == "name")
                    m_current.top().name = readText(&xml);
                else if(name == "url") {
                    QString protocol = xml.attributes().value("protocol").toString();
                    m_current.top().m_urls[protocol] = readText(&xml);
                } else if(name == "icon")
                    m_current.top().icon = readText(&xml);
            }
        } else if(token==QXmlStreamReader::EndElement) {
            QStringRef name = xml.name();
            if(name == "project" || name == "module") {
                Source p = m_current.pop();

                if (name == "project")
                    Q_ASSERT(p.type == Source::Project);
                else if (name == "module")
                    Q_ASSERT(p.type == Source::Module);

                if(!p.m_urls.isEmpty()) {
                    SourceItem* item = new SourceItem(p);
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
    m_errors += QString("error: %1").arg(error);
}
