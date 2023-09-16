/*
    SPDX-FileCopyrightText: 2018 Morten Danielsen Volden

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "perforceimportmetadatawidget.h"

#include <vcslocation.h>

#include <QProcess>
#include <QTemporaryDir>
#include <QTextStream>
#include <QStandardPaths>

using namespace KDevelop;

PerforceImportMetadataWidget::PerforceImportMetadataWidget(QWidget* parent)
    : VcsImportMetadataWidget(parent)
    , m_ui(new Ui::PerforceImportMetadataWidget)
{
    m_ui->setupUi(this);

    m_ui->executableLoc->setText(QStringLiteral("/usr/bin/p4"));
    m_ui->p4portEdit->setText(QStringLiteral("perforce:1666"));

    QProcessEnvironment curEnv = QProcessEnvironment::systemEnvironment();
    m_ui->p4configEdit->setText(curEnv.value(QStringLiteral("P4CONFIG")));
    m_ui->p4portEdit->setText(curEnv.value(QStringLiteral("P4PORT")));
    m_ui->p4userEdit->setText(curEnv.value(QStringLiteral("P4USER")));

    const auto hasP4Config = curEnv.contains(QStringLiteral("P4CONFIG"));
    m_ui->radioButtonConfig->setChecked(hasP4Config);
    m_ui->radioButtonVariables->setChecked(!hasP4Config);
    m_ui->p4configEdit->setEnabled(hasP4Config);

    m_ui->sourceLoc->setEnabled(false);
    m_ui->sourceLoc->setMode(KFile::Directory);

    m_ui->errorMsg->setTextColor(QColor(255, 0, 0));
    m_ui->errorMsg->setReadOnly(true);

    m_ui->p4clientEdit->setEditable(true);

    connect(m_ui->p4clientEdit, QOverload<const QString&>::of(&KComboBox::returnPressed),
            this, &PerforceImportMetadataWidget::changed);

    connect(m_ui->radioButtonConfig, &QRadioButton::clicked, m_ui->p4configEdit, &QLineEdit::setEnabled);
    connect(m_ui->radioButtonVariables, &QRadioButton::clicked, m_ui->p4configEdit, &QLineEdit::setDisabled);

    connect(m_ui->testP4setupButton, &QPushButton::pressed, this, &PerforceImportMetadataWidget::testP4setup);
}

QUrl PerforceImportMetadataWidget::source() const
{
    return m_ui->sourceLoc->url();
}

VcsLocation PerforceImportMetadataWidget::destination() const
{
    VcsLocation dest;
    dest.setRepositoryServer(m_ui->p4portEdit->text());
    dest.setUserData(QVariant::fromValue(m_ui->p4userEdit->text()));
    dest.setRepositoryBranch(m_ui->p4clientEdit->itemText(0));
    return dest;
}

QString PerforceImportMetadataWidget::message() const
{
    return QString();
    //TODO: return m_ui->message->toPlainText();
}

void PerforceImportMetadataWidget::setSourceLocation(const VcsLocation& url)
{
    m_ui->sourceLoc->setUrl(url.localUrl());
}

void PerforceImportMetadataWidget::setSourceLocationEditable(bool enable)
{
    m_ui->sourceLoc->setEnabled(enable);
}

void PerforceImportMetadataWidget::setMessage(const QString& message)
{
    Q_UNUSED(message);

    //FIXME: correct ui field needs to be set
    //m_ui->message->setText(message);
}

bool PerforceImportMetadataWidget::hasValidData() const
{
    // FIXME: It has valid data if testP4setup has completed correctly. AND client name has been set to something
    return !m_ui->p4clientEdit->itemText(0).isEmpty();
}

void PerforceImportMetadataWidget::testP4setup()
{
    m_ui->errorMsg->clear();
    m_ui->p4clientEdit->clear();

    if (!validateP4executable())
        return;

    QDir execDir(m_ui->sourceLoc->url().toLocalFile());
    QTemporaryDir tmpDir;
    if (!execDir.exists())
        execDir.setPath(tmpDir.path());

    if(!validateP4port(execDir.path()))
        return;

    if(!validateP4user(execDir.path()))
        return;

    emit changed();
}

bool PerforceImportMetadataWidget::validateP4executable()
{
    if (QStandardPaths::findExecutable(m_ui->executableLoc->url().toLocalFile()).isEmpty()) {
        m_ui->errorMsg->setText(
            i18n("Unable to find perforce executable. Is it installed on the system? Is it in your PATH?"));
        return false;
    }
    return true;
}

bool PerforceImportMetadataWidget::validateP4user(const QString&  projectDir) const
{
    QProcess exec;
    QProcessEnvironment p4execEnvironment;
    p4execEnvironment.insert(QStringLiteral("P4PORT"), m_ui->p4portEdit->displayText());
    exec.setWorkingDirectory(projectDir);
    exec.setProcessEnvironment(p4execEnvironment);
    exec.start(m_ui->executableLoc->url().toLocalFile(), QStringList{QStringLiteral("workspaces"),
        QStringLiteral("-u"), m_ui->p4userEdit->text()}
    );
    exec.waitForFinished();

    const auto processStdout = QString::fromUtf8(exec.readAllStandardOutput());
    const auto processStderr = QString::fromUtf8(exec.readAllStandardError());

    //     std::cout << "Exited with code: " << exec.exitCode() << std::endl;
    //     std::cout << "Exited with stdout" << processStdout.toStdString() << std::endl;
    //     std::cout << "Exited with stderr" << processStderr.toStdString() << std::endl;
    if (exec.exitCode() != 0) {
        if(!processStderr.isEmpty()) {
            m_ui->errorMsg->setText(processStderr);
        } else {
            m_ui->errorMsg->setText(i18n("P4 Client failed with exit code: %1", exec.exitCode()));
        }
        return false;
    }
    if(!processStdout.isEmpty()) {
        const auto clientCmdOutput = QStringView(processStdout).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        QStringList clientItems;
        clientItems.reserve(clientCmdOutput.size());
        for (const auto& clientLine : clientCmdOutput) {
            const auto wordsInLine = clientLine.split(QLatin1Char(' '));
            // Client mvo_testkdevinteg 2017/05/22 root C:\P4repo 'Created by mvo. ' -- Line would be expected to look like so
            clientItems.append(wordsInLine.at(1).toString());
        }
        m_ui->p4clientEdit->addItems(clientItems);
    }
    return true;
}

bool PerforceImportMetadataWidget::validateP4port(const QString&  projectDir) const
{
    QProcess exec;
    QProcessEnvironment p4execEnvironment;
    p4execEnvironment.insert(QStringLiteral("P4PORT"), m_ui->p4portEdit->displayText());
    QTextStream out(stdout);
    const auto& env = p4execEnvironment.toStringList();
    for (const QString& x : env) {
        out << x << QLatin1Char('\n');
    }
    out.flush();

    exec.setWorkingDirectory(projectDir);
    exec.setProcessEnvironment(p4execEnvironment);
    exec.start(m_ui->executableLoc->url().toLocalFile(), QStringList() << QStringLiteral("info"));
    exec.waitForFinished();
    //QString processStdout(exec.readAllStandardOutput());
    const auto processStderr = QString::fromUtf8(exec.readAllStandardError());

    //std::cout << "Exited with code: " << exec.exitCode() << std::endl;
    //std::cout << "Exited with stdout" << processStdout.toStdString() << std::endl;
    //std::cout << "Exited with stderr" << processStderr.toStdString() << std::endl;
    if (exec.exitCode() != 0) {
        if(!processStderr.isEmpty()) {
            m_ui->errorMsg->setText(processStderr);
        } else {
            m_ui->errorMsg->setText(i18n("P4 Client failed with error code: %1", exec.exitCode()));
        }
        return false;
    }
    return true;
}

#include "moc_perforceimportmetadatawidget.cpp"
