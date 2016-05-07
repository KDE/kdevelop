/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>
    Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>
    Copyright 2016 Andreas Cord-Landwehr <cordlandwehr@kde.org>

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
#include "debug.h"

#include <QDir>
#include <QIcon>
#include <QStandardPaths>

#include <KLocalizedString>
#include <KProcess>

#include <algorithm>

namespace {

QString qmakeCandidate()
{
    // return the first qmake executable we can find
    const QStringList candidates = {"qmake", "qmake-qt4", "qmake-qt5"};
    auto it = std::find_if(candidates.constBegin(), candidates.constEnd(), [](const QString& candidate) {
        return !QStandardPaths::findExecutable(candidate).isEmpty();
    });
    return it != candidates.constEnd() ? *it : QString();
}

}

QtHelpQtDoc::QtHelpQtDoc(QObject *parent, const QVariantList &args)
    : QtHelpProviderAbstract(parent, "qthelpcollection.qhc", args)
    , m_path(QString())
{
    Q_UNUSED(args);
    registerDocumentations();
}

void QtHelpQtDoc::registerDocumentations()
{
    const QString qmake = qmakeCandidate();
    if (!qmake.isEmpty()) {
        KProcess *p = new KProcess;
        p->setOutputChannelMode(KProcess::MergedChannels);
        p->setProgram(qmake, QStringList("-query") << "QT_INSTALL_DOCS");
        p->start();
        connect(p, static_cast<void(KProcess::*)(int)>(&KProcess::finished), this, &QtHelpQtDoc::lookupDone);
    }
}

void QtHelpQtDoc::lookupDone(int code)
{
    if(code==0) {
        KProcess* p = qobject_cast<KProcess*>(sender());

        m_path = QDir::fromNativeSeparators(QString::fromLatin1(p->readAllStandardOutput().trimmed()));
        qCDebug(QTHELP) << "Detected doc path:" << m_path;
    }
    sender()->deleteLater();
}

void QtHelpQtDoc::loadDocumentation()
{
    if(m_path.isEmpty()) {
        return;
    }

    QString fileName = qchFile();
    if(fileName.isEmpty()) {
        qCWarning(QTHELP) << "could not find QCH file in directory" << m_path;
        return;
    }

    QString fileNamespace = QHelpEngineCore::namespaceName(fileName);
    if (!fileNamespace.isEmpty() && !m_engine.registeredDocumentations().contains(fileNamespace)) {
        qCDebug(QTHELP) << "loading doc" << fileName << fileNamespace;
        if(!m_engine.registerDocumentation(fileName))
            qCCritical(QTHELP) << "error >> " << fileName << m_engine.error();
    }
}

void QtHelpQtDoc::unloadDocumentation()
{
    QString fileName = qchFile();
    QString fileNamespace = QHelpEngineCore::namespaceName(fileName);
    if(!fileName.isEmpty() && !m_engine.registeredDocumentations().contains(fileNamespace)) {
        m_engine.unregisterDocumentation(fileName);
    }
}

QString QtHelpQtDoc::qchFile() const
{
    QVector<QString> paths;
    paths << m_path << m_path+"/qch/";
    foreach (const auto &path, paths) {
        QDir d(path);
        if(path.isEmpty() || !d.exists()) {
            continue;
        }

        foreach(const QString& file, d.entryList(QDir::Files)) {
            QString fileName=path+'/'+file;
            return fileName;
        }
    }
    qCDebug(QTHELP) << "no QCH file found at all";
    return QString();
}

QIcon QtHelpQtDoc::icon() const
{
    return QIcon::fromTheme("qtlogo");
}

QString QtHelpQtDoc::name() const
{
    return i18n("QtHelp");
}
