/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>
    SPDX-FileCopyrightText: 2016 Andreas Cord-Landwehr <cordlandwehr@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qthelpqtdoc.h"
#include "debug.h"

#include <QDir>
#include <QIcon>
#include <QStandardPaths>

#include <KLocalizedString>
#include <QProcess>

#include <algorithm>

QtHelpQtDoc::QtHelpQtDoc(QObject* parent, const QString& qmake, const QString& collectionFileName)
    : QtHelpProviderAbstract(parent, collectionFileName)
    , m_qmake(qmake)
{
    registerDocumentations();
}

QtHelpQtDoc::~QtHelpQtDoc() = default;

QStringList QtHelpQtDoc::qmakeCandidates()
{
    QStringList candidates{
        QStringLiteral("qmake"),  QStringLiteral("qmake-qt4"), QStringLiteral("qmake-qt5"),
        QStringLiteral("qmake6"), QStringLiteral("qmake-qt6"),
    };
    const auto it = std::remove_if(candidates.begin(), candidates.end(), [](const QString& candidate) {
        return QStandardPaths::findExecutable(candidate).isEmpty();
    });
    candidates.erase(it, candidates.end());
    return candidates;
}

void QtHelpQtDoc::registerDocumentations()
{
    Q_ASSERT(!m_isInitialized);
    if (!m_qmake.isEmpty()) {
        auto* p = new QProcess(this);
        p->setProcessChannelMode(QProcess::MergedChannels);
        p->setProgram(m_qmake);
        p->setArguments({QLatin1String("-query"), QLatin1String("QT_INSTALL_DOCS")});
        connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, p](int code) {
            if (code == QProcess::NormalExit) {
                m_path = QDir::fromNativeSeparators(QString::fromLatin1(p->readAllStandardOutput().trimmed()));
                qCDebug(QTHELP) << "Detected doc path:" << m_path;
            } else {
                qCCritical(QTHELP) << "qmake query returned error:" << QString::fromLatin1(p->readAllStandardError());
                qCDebug(QTHELP) << "last standard output was:" << QString::fromLatin1(p->readAllStandardOutput());
            }

            p->deleteLater();
            m_isInitialized = true;
            emit isInitializedChanged();
        });
        p->start();
    } else {
        m_isInitialized = true;
        emit isInitializedChanged();
    }
}

void QtHelpQtDoc::loadDocumentation()
{
    Q_ASSERT(m_isInitialized);

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
    Q_ASSERT(m_isInitialized);

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

#include "moc_qthelpqtdoc.cpp"
