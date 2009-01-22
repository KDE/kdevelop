/* KDevelop CMake Support
 *
 * Copyright 2007 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakebuilddircreator.h"
#include <QScrollBar>
#include <QDir>
#include <KDebug>
#include <KMessageBox>
#include "ui_cmakebuilddircreator.h"

CMakeBuildDirCreator::CMakeBuildDirCreator(const KUrl& srcDir, QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f), m_srcFolder(srcDir)
{
    m_creatorUi = new Ui::CMakeBuildDirCreator;
    m_creatorUi->setupUi( this );
    m_creatorUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    m_creatorUi->buttonBox->button(QDialogButtonBox::Ok)->setIcon(KIcon("dialog-ok"));
    m_creatorUi->buttonBox->button(QDialogButtonBox::Cancel)->setIcon(KIcon("dialog-cancel"));

    connect(m_creatorUi->run, SIGNAL(clicked()), this, SLOT(runBegin()));
    connect(m_creatorUi->builddirChooser, SIGNAL(updated(CMakeBuildDirChooser::StatusTypes, const QString&)), 
            SLOT(chooserUpdated(CMakeBuildDirChooser::StatusTypes,const QString&)));
    connect(&m_proc, SIGNAL(readyReadStandardError()), this, SLOT(addOutput()));
    connect(&m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(addOutput()));
    connect(&m_proc, SIGNAL(finished ( int , QProcess::ExitStatus )), this, SLOT(cmakeCommandDone ( int , QProcess::ExitStatus )));
    m_creatorUi->builddirChooser->setSourceFolder( m_srcFolder );
}

void CMakeBuildDirCreator::runBegin()
{
    if(m_proc.state()==QProcess::NotRunning)
    {
        m_creatorUi->cmakeOutput->clear();
        QStringList args;
        kDebug(9042) << "Type of build: " << m_creatorUi->builddirChooser->buildType();
        kDebug(9042) << "Installing to: " << m_creatorUi->builddirChooser->installPrefix();
        kDebug(9042) << "Build directory: " << m_creatorUi->builddirChooser->buildFolder();

        QDir d( m_creatorUi->builddirChooser->buildFolder().toLocalFile() );
        if( !d.exists() )
        {
            int ret=KMessageBox::warningContinueCancel(0,
                        i18n("The %1 directory does not exist, but is needed before the build directory is created.\n"
                            "Do you want KDevelop to create it for you?", m_creatorUi->builddirChooser->buildFolder().toLocalFile()));
            if(ret==KMessageBox::Continue)
            {
                bool res=QDir::root().mkpath(d.absolutePath() );
                if(!res)
                    return;
            }
            else
                return;
        }

        args += m_srcFolder.toLocalFile();
        args += "-DCMAKE_INSTALL_PREFIX="+m_creatorUi->builddirChooser->installPrefix().toLocalFile();
        args += "-DCMAKE_BUILD_TYPE="+m_creatorUi->builddirChooser->buildType();

        m_proc.setWorkingDirectory(m_creatorUi->builddirChooser->buildFolder().toLocalFile());
        m_proc.setProgram(m_creatorUi->builddirChooser->cmakeBinary().toLocalFile(), args);
        m_proc.setOutputChannelMode(KProcess::MergedChannels);
        m_proc.start();
        m_creatorUi->cmakeOutput->setEnabled(true);
        m_creatorUi->status->setText(i18n("Running"));
        m_creatorUi->run->setText(i18n("&Cancel"));
    }
    else
    {
        m_creatorUi->run->setEnabled(false);
        m_creatorUi->run->setText(i18n("&Run"));
        m_proc.kill();
        m_creatorUi->status->setText(i18n("CMake process killed"));
    }
}

void CMakeBuildDirCreator::runEnd()
{
}

void CMakeBuildDirCreator::addOutput()
{
    QByteArray output=m_proc.readAllStandardOutput();
    QString s;
    s.append(output.trimmed());
    m_creatorUi->cmakeOutput->appendPlainText(s);
    if( m_creatorUi->cmakeOutput->verticalScrollBar()->value() == m_creatorUi->cmakeOutput->verticalScrollBar()->maximum() )
        m_creatorUi->cmakeOutput->verticalScrollBar()->setValue(m_creatorUi->cmakeOutput->verticalScrollBar()->maximum());
}

void CMakeBuildDirCreator::cmakeCommandDone(int exitCode, QProcess::ExitStatus exitStatus)
{
    /*QByteArray b = m_proc.readAllStandardOutput();
    QString t;
    t.prepend(b.trimmed());
    m_creatorUi->cmakeOutput->setPlainText(t);*/

    bool successful=exitCode==0 && exitStatus==QProcess::NormalExit;
    if(successful)
    {
        m_creatorUi->status->setText(i18n("Created successfully"));
    }
    else
    {
        m_creatorUi->status->setText(i18n("An error occurred"));
    }
    m_creatorUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(successful);
}

void CMakeBuildDirCreator::chooserStatus(const QString& status)
{
    m_creatorUi->status->setText( status );
}

void CMakeBuildDirCreator::chooserUpdated( CMakeBuildDirChooser::StatusTypes st, const QString& srcDirFromCache )
{
    m_creatorUi->cmakeOutput->setEnabled(false);
    m_creatorUi->run->setEnabled( (st & CMakeBuildDirChooser::BuildFolderEmpty ) != 0 
                               && ( st & CMakeBuildDirChooser::HaveCMake ) != 0 );
    if(m_alreadyUsed.contains(srcDirFromCache))
    {
        m_creatorUi->run->setEnabled(false);
        m_creatorUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        m_creatorUi->status->setText(i18n("Already used build directory"));
    }
    else if( ( st & CMakeBuildDirChooser::BuildDirCreated ) != 0 
          && ( st & CMakeBuildDirChooser::CorrectProject ) != 0 )
    {
        m_creatorUi->run->setEnabled(false);
        m_creatorUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
    else
    {
        m_creatorUi->run->setEnabled( ( st & CMakeBuildDirChooser::CorrectBuildDir ) != 0 );
        m_creatorUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

void CMakeBuildDirCreator::setCMakeBinary(const KUrl& url) { m_creatorUi->builddirChooser->setCMakeBinary(url); }

void CMakeBuildDirCreator::setInstallPrefix(const KUrl& url) { m_creatorUi->builddirChooser->setInstallPrefix(url); }

void CMakeBuildDirCreator::setBuildFolder(const KUrl& url) { m_creatorUi->builddirChooser->setBuildFolder(url); }

void CMakeBuildDirCreator::setBuildType(const QString& s) { m_creatorUi->builddirChooser->setBuildType(s); }

KUrl CMakeBuildDirCreator::cmakeBinary() const { return m_creatorUi->builddirChooser->cmakeBinary(); }

KUrl CMakeBuildDirCreator::installPrefix() const { return m_creatorUi->builddirChooser->installPrefix(); }

KUrl CMakeBuildDirCreator::buildFolder() const { return m_creatorUi->builddirChooser->buildFolder(); }

QString CMakeBuildDirCreator::buildType() const { return m_creatorUi->builddirChooser->buildType(); }

void CMakeBuildDirCreator::setAlreadyUsed(const QStringList &used)
{
    m_alreadyUsed=used;
}

#include "cmakebuilddircreator.moc"

