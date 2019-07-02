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
#include <QProcess>

#include <algorithm>

namespace {

QString qmakeCandidate()
{
    // return the first qmake executable we can find
    const QStringList candidates {
        QStringLiteral("qmake"),
        QStringLiteral("qmake-qt4"),
        QStringLiteral("qmake-qt5")
    };
    auto it = std::find_if(candidates.constBegin(), candidates.constEnd(), [](const QString& candidate) {
        return !QStandardPaths::findExecutable(candidate).isEmpty();
    });
    return it != candidates.constEnd() ? *it : QString();
}

}

QtHelpQtDoc::QtHelpQtDoc(QObject *parent, const QVariantList &args)
    : QtHelpProviderAbstract(parent, QStringLiteral("qthelpcollection.qhc"), args)
    , m_path(QString())
{
    Q_UNUSED(args);
    registerDocumentations();
}

void QtHelpQtDoc::registerDocumentations()
{
    const QString qmake = qmakeCandidate();
    if (!qmake.isEmpty()) {
        auto *p = new QProcess;
        p->setProcessChannelMode(QProcess::MergedChannels);
        p->setProgram(qmake);
        p->setArguments({QLatin1String("-query"), QLatin1String("QT_INSTALL_DOCS")});
        p->start();
        connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &QtHelpQtDoc::lookupDone);
    }
}

void QtHelpQtDoc::lookupDone(int code)
{
    auto *p = qobject_cast<QProcess*>(sender());
    if(code == QProcess::NormalExit) {
        m_path = QDir::fromNativeSeparators(QString::fromLatin1(p->readAllStandardOutput().trimmed()));
        qCDebug(QTHELP) << "Detected doc path:" << m_path;
    } else {
        qCCritical(QTHELP) << "qmake query returned error:" << QString::fromLatin1(p->readAllStandardError());
        qCDebug(QTHELP) << "last standard output was:" << QString::fromLatin1(p->readAllStandardOutput());
    }

    sender()->deleteLater();
}

void QtHelpQtDoc::loadDocumentation()
{
    if(m_path.isEmpty()) {
        return;
    }

    const QStringList files = qchFiles();
    if(files.isEmpty()) {
        qCWarning(QTHELP) << "could not find QCH file in directory" << m_path;
        return;
    }

    for (const QString& fileName : files) {
        QString fileNamespace = QHelpEngineCore::namespaceName(fileName);
        if (!fileNamespace.isEmpty() && !m_engine.registeredDocumentations().contains(fileNamespace)) {
            qCDebug(QTHELP) << "loading doc" << fileName << fileNamespace;
            if(!m_engine.registerDocumentation(fileName))
                qCCritical(QTHELP) << "error >> " << fileName << m_engine.error();
        }
    }
}

void QtHelpQtDoc::unloadDocumentation()
{
    const auto fileNames = qchFiles();
    for (const QString& fileName : fileNames) {
        QString fileNamespace = QHelpEngineCore::namespaceName(fileName);
        if(!fileName.isEmpty() && m_engine.registeredDocumentations().contains(fileNamespace)) {
            m_engine.unregisterDocumentation(fileName);
        }
    }
}

QStringList QtHelpQtDoc::qchFiles() const
{
    QStringList files;

    const QVector<QString> paths{ // test directories
        m_path,
        m_path + QLatin1String("/qch/"),
    };

    for (const auto& path : paths) {
        QDir d(path);
        if(path.isEmpty() || !d.exists()) {
            continue;
        }
        const auto fileInfos = d.entryInfoList(QDir::Files);
        for (const auto& file : fileInfos) {
            files << file.absoluteFilePath();
        }
    }
    if (files.isEmpty()) {
        qCDebug(QTHELP) << "no QCH file found at all";
    }
    return files;
}

QIcon QtHelpQtDoc::icon() const
{
    return QIcon::fromTheme(QStringLiteral("qtlogo"));
}

QString QtHelpQtDoc::name() const
{
    return i18n("QtHelp");
}
