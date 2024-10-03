/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mesonrewriterjob.h"
#include "mesonconfig.h"

#include "interfaces/iproject.h"
#include "util/path.h"

#include <KLocalizedString>
#include <KProcess>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTemporaryFile>
#include <QtConcurrentRun>

#include "debug.h"

using namespace KDevelop;

MesonRewriterJob::MesonRewriterJob(IProject* project, const QVector<MesonRewriterActionPtr>& actions, QObject* parent)
    : KJob(parent)
    , m_project(project)
    , m_actions(actions)
{
    connect(&m_futureWatcher, &QFutureWatcher<QString>::finished, this, &MesonRewriterJob::finished);
}

QString MesonRewriterJob::execute()
{
    QJsonArray command;
    for (auto& i : m_actions) {
        command.append(i->command());
    }

    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    if (!tempFile.open()) {
        return i18n("Failed to create a temporary file.");
    }

    QJsonDocument doc(command);
    tempFile.write(doc.toJson());
    tempFile.flush();

    Meson::BuildDir buildDir = Meson::currentBuildDir(m_project);

    KProcess proc;
    proc.setWorkingDirectory(m_project->path().toLocalFile());
    proc.setOutputChannelMode(KProcess::SeparateChannels);
    proc.setProgram(buildDir.mesonExecutable.toLocalFile());
    proc << QStringLiteral("rewrite") << QStringLiteral("command") << tempFile.fileName();

    int ret = proc.execute();
    if (ret != 0) {
        return i18n("%1 returned %2", proc.program().join(QLatin1Char(' ')), ret);
    }

    auto rawData = proc.readAllStandardError();
    if (rawData.isEmpty()) {
        return QString();
    }

    QJsonParseError error;
    QJsonDocument result = QJsonDocument::fromJson(rawData, &error);
    if (error.error) {
        return i18n("JSON parser error: %1", error.errorString());
    }

    if (!result.isObject()) {
        return i18n("The rewriter output of '%1' is not an object", proc.program().join(QLatin1Char(' ')));
    }

    for (auto& i : m_actions) {
        i->parseResult(result.object());
    }

    return QString();
}

void MesonRewriterJob::finished()
{
    QString result = m_futureWatcher.result();
    if (!result.isEmpty()) {
        qCWarning(KDEV_Meson) << "REWRITER " << result;
        setError(true);
        setErrorText(result);
        emitResult();
        return;
    }

    qCDebug(KDEV_Meson) << "REWRITER: Meson rewriter job finished";
    emitResult();
}

bool MesonRewriterJob::doKill()
{
    if (m_futureWatcher.isRunning()) {
        m_futureWatcher.cancel();
    }
    return true;
}

void MesonRewriterJob::start()
{
    auto future = QtConcurrent::run([this] {
        return execute();
    });
    m_futureWatcher.setFuture(future);
}

#include "moc_mesonrewriterjob.cpp"
