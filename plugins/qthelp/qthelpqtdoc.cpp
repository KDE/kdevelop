/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>
    SPDX-FileCopyrightText: 2016 Andreas Cord-Landwehr <cordlandwehr@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qthelpqtdoc.h"
#include "debug.h"

#include <KLocalizedString>

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QIcon>
#include <QProcess>
#include <QSet>
#include <QStandardPaths>

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
        QStringLiteral("qmake6"),    QStringLiteral("qmake-qt6"), QStringLiteral("qmake"),
        QStringLiteral("qmake-qt5"), QStringLiteral("qmake-qt4"),
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

    QSet<QString> qchFiles;
    visitQchFiles([&qchFiles](const QFileInfo& fileInfo) {
        qchFiles.insert(fileInfo.absoluteFilePath());
        return false; // continue iteration to collect all help file paths
    });
    if (qchFiles.empty()) {
        qCWarning(QTHELP) << "could not find QCH file in directory" << m_path;
        return;
    }

    cleanUpRegisteredDocumentations([&qchFiles, this](const QString& namespaceName) {
        const auto filePath = m_engine.documentationFileName(namespaceName);
        const auto it = qchFiles.constFind(filePath);
        if (it == qchFiles.cend()) {
            return true; // unregister this namespace associated with an unneeded .qch file
        }
        if (QHelpEngineCore::namespaceName(*it) != namespaceName) {
            // Unregister this namespace, because it does not match
            // the namespace name stored in the associated .qch file.
            return true;
        }

        // The .qch file *it is already registered and up-to-date.
        qchFiles.erase(it); // do not reregister it
        return false; // keep its namespace registered with the engine
    });

    for (const auto& fileName : std::as_const(qchFiles)) {
        registerDocumentation(fileName);
    }
}

void QtHelpQtDoc::unloadDocumentation()
{
    Q_ASSERT(m_isInitialized);
    cleanUpRegisteredDocumentations([](const QString&) {
        return true; // unregister all namespaces
    });
}

bool QtHelpQtDoc::isQtHelpAvailable() const
{
    return visitQchFiles([](const QFileInfo&) {
        return true; // abort iteration and return true (i.e. "available") once the first help file is found
    });
}

template<typename ProcessQchFileInfo>
bool QtHelpQtDoc::visitQchFiles(ProcessQchFileInfo processQchFileInfo) const
{
    const QVector<QString> paths{ // test directories
        m_path,
        m_path + QLatin1String("/qch/"),
    };

    for (const auto& path : paths) {
        if (path.isEmpty()) {
            continue;
        }

        QDirIterator it(path, {QStringLiteral("*.qch")}, QDir::Files);
        while (it.hasNext()) {
            if (processQchFileInfo(it.nextFileInfo())) {
                return true;
            }
        }
    }

    return false;
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
