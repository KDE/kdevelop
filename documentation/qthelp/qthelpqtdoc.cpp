/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>
    Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "qthelpqtdoc.h"

#include <QDir>

#include <KLocale>
#include <KStandardDirs>
#include <KProcess>
#include <KIcon>
#include <KDebug>

QtHelpQtDoc::QtHelpQtDoc(QObject *parent, const QVariantList &args)
    : QtHelpProviderAbstract(parent, "qthelpcollection.qhc", args)
{
    Q_UNUSED(args);
    registerDocumentations();
}

void QtHelpQtDoc::registerDocumentations()
{
    QStringList qmakes;
    qmakes << KStandardDirs::findExe("qmake")
           << KStandardDirs::findExe("qmake-qt4");
    if(!qmakes.isEmpty()) {
        KProcess *p = new KProcess;
        p->setOutputChannelMode(KProcess::MergedChannels);
        p->setProgram(qmakes.first(), QStringList("-query") << "QT_INSTALL_DOCS");
        p->start();
        connect(p, SIGNAL(finished(int)), SLOT(lookupDone(int)));
    }
}

void QtHelpQtDoc::lookupDone(int code)
{
    if(code==0) {
        KProcess* p = qobject_cast<KProcess*>(sender());
        
        QString path = QDir::fromNativeSeparators(QString::fromLatin1(p->readAllStandardOutput()));
        loadDirectory(path+"/qch/");
    }
    sender()->deleteLater();
}

void QtHelpQtDoc::loadDirectory(const QString& path)
{
    if(path.isEmpty()) {
        kDebug() << "no QtHelp found at all";
        return;
    }
    
    QDir d(path);
    foreach(const QString& file, d.entryList(QDir::Files)) {
        QString fileName=path+'/'+file;
        QString fileNamespace = QHelpEngineCore::namespaceName(fileName);
        
        if (!fileNamespace.isEmpty() && !m_engine.registeredDocumentations().contains(fileNamespace)) {
            kDebug() << "loading doc" << fileName << fileNamespace;
            if(!m_engine.registerDocumentation(fileName))
                kDebug() << "error >> " << fileName << m_engine.error();
        }
    }
    kDebug() << "registered" << m_engine.error() << m_engine.registeredDocumentations();
}

QIcon QtHelpQtDoc::icon() const
{
    return KIcon("qtlogo");
}

QString QtHelpQtDoc::name() const
{
    return i18n("QtHelp");
}
