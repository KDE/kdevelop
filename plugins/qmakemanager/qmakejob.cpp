/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qmakejob.h"
#include <QDir>
#include <QProcess>
#include <KLocalizedString>
#include <KShell>
#include <util/processlinemaker.h>
#include <outputview/outputmodel.h>
#include <outputview/ioutputview.h>
#include <debug.h>

using namespace KDevelop;

QMakeJob::QMakeJob(QString srcDir, QString buildDir, QObject* parent)
    : OutputJob(parent)
    , m_srcDir(std::move(srcDir))
    , m_buildDir(std::move(buildDir))
    , m_qmakePath(QStringLiteral("qmake"))
    , m_buildType(0)
    , m_process(nullptr)
    , m_model(nullptr)

{
    setCapabilities(Killable);
    setStandardToolView(IOutputView::BuildView);
    setBehaviours(IOutputView::AllowUserClose | IOutputView::AutoScroll);

    setObjectName(i18n("Run QMake in %1", m_buildDir));
}

QMakeJob::~QMakeJob()
{
}

void QMakeJob::setQMakePath(const QString& path)
{
    m_qmakePath = path;
}

void QMakeJob::setInstallPrefix(const QString& prefix)
{
    m_installPrefix = prefix;
}

void QMakeJob::setBuildType(int comboboxSelectedIndex)
{
    m_buildType = comboboxSelectedIndex;
}

void QMakeJob::setExtraArguments(const QString& args)
{
    m_extraArguments = args;
}

void QMakeJob::start()
{
    static const char* const BUILD_TYPES[] = { "debug", "build", "(don't specify)" };

    m_model = new OutputModel;
    setModel(m_model);

    startOutput();

    QStringList args;
    if (m_buildType < 2)
        args << QLatin1String("CONFIG+=") + QLatin1String(BUILD_TYPES[m_buildType]);
    if (!m_installPrefix.isEmpty())
        args << QLatin1String("target.path=") + m_installPrefix;
    if (!m_extraArguments.isEmpty()) {
        KShell::Errors err;
        QStringList tmp = KShell::splitArgs(m_extraArguments, KShell::TildeExpand | KShell::AbortOnMeta, &err);
        if (err == KShell::NoError) {
            args += tmp;
        } else {
            qCWarning(KDEV_QMAKE) << "Ignoring qmake Extra arguments";
            if (err == KShell::BadQuoting) {
                qCWarning(KDEV_QMAKE) << "QMake arguments badly quoted:" << m_extraArguments;
            } else {
                qCWarning(KDEV_QMAKE) << "QMake arguments had meta character:" << m_extraArguments;
            }
        }
    }
    args << QStringLiteral("-r") << m_srcDir;

    m_model->appendLine(m_buildDir + QLatin1String(": ") + args.join(QLatin1Char(' ')));

    QDir build(m_buildDir);
    if (!build.exists()) {
        build.mkpath(build.absolutePath());
    }

    m_process = new QProcess(this);
    m_process->setWorkingDirectory(m_buildDir);
    m_process->setProgram(m_qmakePath);
    m_process->setArguments(args);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    auto lineMaker = new KDevelop::ProcessLineMaker(m_process, this);

    connect(lineMaker, &ProcessLineMaker::receivedStdoutLines, m_model, &OutputModel::appendLines);
    connect(lineMaker, &ProcessLineMaker::receivedStderrLines, m_model, &OutputModel::appendLines);
    connect(m_process, &QProcess::errorOccurred, this, &QMakeJob::processError);
    connect(m_process, &QProcess::finished, this, &QMakeJob::processFinished);

    m_process->start();
}

bool QMakeJob::doKill()
{
    if (!m_process) {
        return true;
    }

    m_process->kill();
    return m_process->state() == m_process->NotRunning;
}

QString QMakeJob::errorString() const
{
    return m_process->errorString();
}

void QMakeJob::processError(QProcess::ProcessError error)
{
    m_model->appendLine(errorString());
    setError(error);
    emitResult();
}

void QMakeJob::processFinished(int exitCode, QProcess::ExitStatus status)
{
    if (status == QProcess::NormalExit) {
        m_model->appendLine(i18n("*** Exited with return code: %1 ***", exitCode));
    } else if (error() == KJob::KilledJobError) {
        m_model->appendLine(i18n("*** Process aborted ***"));
    } else {
        m_model->appendLine(i18n("*** Crashed with return code: %1 ***", exitCode));
    }

    emitResult();
}

#include "moc_qmakejob.cpp"
