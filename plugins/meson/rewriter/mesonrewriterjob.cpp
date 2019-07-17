/* This file is part of KDevelop
    Copyright 2019 Daniel Mensinger <daniel@mensinger-ka.de>

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

    KProcess proc(this);
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
    auto future = QtConcurrent::run(this, &MesonRewriterJob::execute);
    m_futureWatcher.setFuture(future);
}
