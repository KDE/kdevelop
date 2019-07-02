/***************************************************************************
*   This file is part of KDevelop Perforce plugin, KDE project            *
*                                                                         *
*   Copyright 2018  Morten Danielsen Volden                               *
*                                                                         *
*   This program is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation, either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
***************************************************************************/

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

    m_ui->executableLoc->setText("/usr/bin/p4");
    m_ui->p4portEdit->setText("perforce:1666");

    QProcessEnvironment curEnv = QProcessEnvironment::systemEnvironment();
    m_ui->p4configEdit->setText(curEnv.contains("P4CONFIG") ? curEnv.value("P4CONFIG") : "");
    m_ui->p4portEdit->setText(curEnv.contains("P4PORT") ? curEnv.value("P4PORT") : "");
    m_ui->p4userEdit->setText(curEnv.contains("P4USER") ? curEnv.value("P4USER") : "");
    curEnv.contains("P4CONFIG") ? m_ui->radioButtonConfig->setChecked(true) : m_ui->radioButtonVariables->setChecked(true);
    curEnv.contains("P4CONFIG") ? m_ui->p4configEdit->setEnabled(true) : m_ui->p4configEdit->setEnabled(false);

    m_ui->sourceLoc->setEnabled(false);
    m_ui->sourceLoc->setMode(KFile::Directory);

    m_ui->errorMsg->setTextColor(QColor(255, 0, 0));
    m_ui->errorMsg->setReadOnly(true);

    m_ui->p4clientEdit->setEditable(true);

    connect(m_ui->p4clientEdit, QOverload<>::of(&KComboBox::returnPressed),
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
        m_ui->errorMsg->setText("Unable to find perforce executable. Is it installed on the system? Is it in your PATH?");
        return false;
    }
    return true;
}

bool PerforceImportMetadataWidget::validateP4user(const QString&  projectDir) const
{
    QProcess exec;
    QProcessEnvironment p4execEnvironment;
    p4execEnvironment.insert(QString("P4PORT"), m_ui->p4portEdit->displayText());
    exec.setWorkingDirectory(projectDir);
    exec.setProcessEnvironment(p4execEnvironment);
    exec.start(m_ui->executableLoc->url().toLocalFile(), QStringList{QStringLiteral("workspaces"),
        QStringLiteral("-u"), m_ui->p4userEdit->text()}
    );
    exec.waitForFinished();

    QString processStdout(exec.readAllStandardOutput());
    QString processStderr(exec.readAllStandardError());

//     std::cout << "Exited with code: " << exec.exitCode() << std::endl;
//     std::cout << "Exited with stdout" << processStdout.toStdString() << std::endl;
//     std::cout << "Exited with stderr" << processStderr.toStdString() << std::endl;
    if (exec.exitCode() != 0) {
        if(!processStderr.isEmpty()) {
            m_ui->errorMsg->setText(processStderr);
        } else {
            QString msg("P4 Client failed with exit code: ");
            msg += QString::number(exec.exitCode());
            m_ui->errorMsg->setText(msg);
        }
        return false;
    }
    if(!processStdout.isEmpty()) {
        QStringList clientCmdOutput = processStdout.split(QLatin1Char('\n'),QString::SkipEmptyParts);
        QStringList clientItems;
        clientItems.reserve(clientCmdOutput.size());
        for(QString const& clientLine : clientCmdOutput) {
            QStringList wordsInLine = clientLine.split(QLatin1Char(' '));
            // Client mvo_testkdevinteg 2017/05/22 root C:\P4repo 'Created by mvo. ' -- Line would be expected to look like so
            clientItems.append(wordsInLine.at(1));
        }
        m_ui->p4clientEdit->addItems(clientItems);
    }
    return true;
}

bool PerforceImportMetadataWidget::validateP4port(const QString&  projectDir) const
{
    QProcess exec;
    QProcessEnvironment p4execEnvironment;
    p4execEnvironment.insert(QString("P4PORT"), m_ui->p4portEdit->displayText());
    QTextStream out(stdout);
    const auto& env = p4execEnvironment.toStringList();
    for (const QString& x : env) {
        out << x << endl;
    }

    exec.setWorkingDirectory(projectDir);
    exec.setProcessEnvironment(p4execEnvironment);
    exec.start(m_ui->executableLoc->url().toLocalFile(), QStringList() << QStringLiteral("info"));
    exec.waitForFinished();
    //QString processStdout(exec.readAllStandardOutput());
    QString processStderr(exec.readAllStandardError());

    //std::cout << "Exited with code: " << exec.exitCode() << std::endl;
    //std::cout << "Exited with stdout" << processStdout.toStdString() << std::endl;
    //std::cout << "Exited with stderr" << processStderr.toStdString() << std::endl;
    if (exec.exitCode() != 0) {
        if(!processStderr.isEmpty()) {
            m_ui->errorMsg->setText(processStderr);
        } else {
            QString msg("P4 Client failed with error code: ");
            msg += QString::number(exec.exitCode());
            m_ui->errorMsg->setText(msg);
        }
        return false;
    }
    return true;
}
