/*
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2017 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ninjajob.h"

#include "ninjabuilder.h"

#include <outputview/outputfilteringstrategies.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

#include <KLocalizedString>
#include <KConfigGroup>

#include <QFile>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>

using namespace KDevelop;

class NinjaJobCompilerFilterStrategy
    : public CompilerFilterStrategy
{
public:
    using CompilerFilterStrategy::CompilerFilterStrategy;

    IFilterStrategy::Progress progressInLine(const QString& line) override;
};

IFilterStrategy::Progress NinjaJobCompilerFilterStrategy::progressInLine(const QString& line)
{
    // example string: [87/88] Building CXX object projectbuilders/ninjabuilder/CMakeFiles/kdevninja.dir/ninjajob.cpp.o
    static const QRegularExpression re(QStringLiteral("^\\[([0-9]+)\\/([0-9]+)\\] (.*)"));

    QRegularExpressionMatch match = re.match(line);
    if (match.hasMatch()) {
        const int current = match.capturedView(1).toInt();
        const int total = match.capturedView(2).toInt();
        if (current && total) {
            // this is output from ninja
            const QString action = match.captured(3);
            const int percent = qRound(( float )current / total * 100);
            return {
                       action, percent
            };
        }
    }

    return {};
}

NinjaJob::NinjaJob(KDevelop::ProjectBaseItem* item, CommandType commandType,
                   const QStringList& arguments, const QByteArray& signal, NinjaBuilder* parent)
    : OutputExecuteJob(parent)
    , m_isInstalling(false)
    , m_idx(item->index())
    , m_commandType(commandType)
    , m_signal(signal)
    , m_plugin(parent)
{
    auto bsm = item->project()->buildSystemManager();
    auto buildDir = bsm->buildDirectory(item);

    setToolTitle(i18n("Ninja"));
    setCapabilities(Killable);
    setStandardToolView(KDevelop::IOutputView::BuildView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);
    setFilteringStrategy(new NinjaJobCompilerFilterStrategy(buildDir.toUrl()));
    setProperties(NeedWorkingDirectory | PortableMessages | DisplayStderr | IsBuilderHint | PostProcessOutput);

    // hardcode the ninja output format so we can parse it reliably
    addEnvironmentOverride(QStringLiteral("NINJA_STATUS"), QStringLiteral("[%s/%t] "));

    *this << ninjaExecutable();
    *this << arguments;

    QStringList targets;
    for (const QString& arg : arguments) {
        if (!arg.startsWith(QLatin1Char('-'))) {
            targets << arg;
        }
    }

    QString title;
    if (!targets.isEmpty()) {
        title = i18n("Ninja (%1): %2", item->text(), targets.join(QLatin1Char(' ')));
    } else {
        title = i18n("Ninja (%1)", item->text());
    }
    setJobName(title);

    connect(this, &NinjaJob::finished, this, &NinjaJob::emitProjectBuilderSignal);
}

NinjaJob::~NinjaJob()
{
    // prevent crash when emitting KJob::finished from ~KJob
    // (=> at this point NinjaJob is already destructed...)
    disconnect(this, &NinjaJob::finished, this, &NinjaJob::emitProjectBuilderSignal);
}

void NinjaJob::setIsInstalling(bool isInstalling)
{
    m_isInstalling = isInstalling;
}

QString NinjaJob::ninjaExecutable()
{
    QString path = QStandardPaths::findExecutable(QStringLiteral("ninja-build"));
    if (path.isEmpty()) {
        path = QStandardPaths::findExecutable(QStringLiteral("ninja"));
    }
    return path;
}

QUrl NinjaJob::workingDirectory() const
{
    KDevelop::ProjectBaseItem* it = item();
    if (!it) {
        return QUrl();
    }
    KDevelop::IBuildSystemManager* bsm = it->project()->buildSystemManager();
    KDevelop::Path workingDir = bsm->buildDirectory(it);
    while (!QFile::exists(workingDir.toLocalFile() + QLatin1String("build.ninja"))) {
        KDevelop::Path upWorkingDir = workingDir.parent();
        if (!upWorkingDir.isValid() || upWorkingDir == workingDir) {
            return bsm->buildDirectory(it->project()->projectItem()).toUrl();
        }
        workingDir = upWorkingDir;
    }
    return workingDir.toUrl();
}

QStringList NinjaJob::privilegedExecutionCommand() const
{
    KDevelop::ProjectBaseItem* it = item();
    if (!it) {
        return QStringList();
    }
    KSharedConfigPtr configPtr = it->project()->projectConfiguration();
    KConfigGroup builderGroup(configPtr, QStringLiteral("NinjaBuilder"));

    bool runAsRoot = builderGroup.readEntry("Install As Root", false);
    if (runAsRoot && m_isInstalling) {
        int suCommand = builderGroup.readEntry("Su Command", 0);
        QStringList arguments;
        switch (suCommand) {
        case 1:
            return QStringList{QStringLiteral("kdesudo"), QStringLiteral("-t")};

        case 2:
            return QStringList{QStringLiteral("sudo")};

        default:
            return QStringList{QStringLiteral("kdesu"), QStringLiteral("-t")};
        }
    }
    return QStringList();
}

void NinjaJob::emitProjectBuilderSignal(KJob* job)
{
    if (!m_plugin) {
        return;
    }

    KDevelop::ProjectBaseItem* it = item();
    if (!it) {
        return;
    }

    if (job->error() == 0) {
        Q_ASSERT(!m_signal.isEmpty());
        QMetaObject::invokeMethod(m_plugin, m_signal.constData(), Q_ARG(KDevelop::ProjectBaseItem*, it));
    } else {
        QMetaObject::invokeMethod(m_plugin, "failed", Q_ARG(KDevelop::ProjectBaseItem*, it));
    }
}

void NinjaJob::postProcessStderr(const QStringList& lines)
{
    appendLines(lines);
}

void NinjaJob::postProcessStdout(const QStringList& lines)
{
    appendLines(lines);
}

void NinjaJob::appendLines(const QStringList& lines)
{
    if (lines.isEmpty()) {
        return;
    }

    QStringList ret(lines);
    bool prev = false;
    for (QStringList::iterator it = ret.end(); it != ret.begin(); ) {
        --it;
        bool curr = it->startsWith(QLatin1Char('['));
        if ((prev && curr) || it->endsWith(QLatin1String("] "))) {
            it = ret.erase(it);
        }
        prev = curr;
    }

    model()->appendLines(ret);
}

KDevelop::ProjectBaseItem* NinjaJob::item() const
{
    return KDevelop::ICore::self()->projectController()->projectModel()->itemFromIndex(m_idx);
}

NinjaJob::CommandType NinjaJob::commandType() const
{
    return m_commandType;
}

#include "moc_ninjajob.cpp"
