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
#include <kdebug.h>
#include "ui_cmakebuilddircreator.h"

CMakeBuildDirCreator::CMakeBuildDirCreator(const KUrl& srcDir, QWidget* parent, Qt::WindowFlags f)
	: QDialog(parent, f), m_srcFolder(srcDir)
{
	m_creatorUi = new Ui::CMakeBuildDirCreator;
	m_creatorUi->setupUi( this );
	m_creatorUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	
	QString cmakeBin=executeProcess("which", QStringList("cmake"));
	setCMakeBinary(KUrl(cmakeBin));
	
	connect(m_creatorUi->run, SIGNAL(clicked()), this, SLOT(runBegin()));
	connect(m_creatorUi->cmakeBin, SIGNAL(textChanged(const QString &)), this, SLOT(updated()));
	connect(m_creatorUi->buildFolder, SIGNAL(textChanged(const QString &)), this, SLOT(updated()));
	connect(m_creatorUi->buildType, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(updated()));
	connect(&m_proc, SIGNAL(readyReadStandardError()), this, SLOT(addError()));
	connect(&m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(addOutput()));
	connect(&m_proc, SIGNAL(finished ( int , QProcess::ExitStatus )), this, SLOT(cmakeCommandDone ( int , QProcess::ExitStatus )));
	updated();
}

void CMakeBuildDirCreator::runBegin()
{
    if(m_proc.state()==QProcess::NotRunning) {
        m_creatorUi->cmakeOutput->clear();
        QStringList args;
        kDebug(9042) << "Type of build: " << buildType();
        kDebug(9042) << "Installing to: " << installPrefix();
        kDebug(9042) << "Build directory: " << buildFolder();
        args += m_srcFolder.toLocalFile();
        args += "-DCMAKE_INSTALL_PREFIX="+installPrefix().toLocalFile();
        args += "-DCMAKE_BUILD_TYPE="+buildType();
        
        m_proc.setWorkingDirectory(buildFolder().toLocalFile());
        m_proc.setProgram(cmakeBinary().toLocalFile(), args);
        m_proc.setOutputChannelMode(KProcess::MergedChannels);
        m_proc.start();
        updated();
        m_creatorUi->cmakeOutput->setEnabled(true);
        m_creatorUi->status->setText(i18n("Running"));
        m_creatorUi->run->setText(i18n("&Cancel"));
    }
    else
    {
        m_proc.kill();
        m_creatorUi->status->setText(i18n("CMake process killed"));
    }
}

void CMakeBuildDirCreator::runEnd()
{
}

void CMakeBuildDirCreator::addError()
{
	QByteArray output=m_proc.readAllStandardError();
	QString s;
	s.append(output.trimmed());
    m_creatorUi->cmakeOutput->setPlainText(m_creatorUi->cmakeOutput->toPlainText()+s+'\n');
    m_creatorUi->cmakeOutput->verticalScrollBar()->setValue(m_creatorUi->cmakeOutput->verticalScrollBar()->maximum());
}

void CMakeBuildDirCreator::addOutput()
{
	QByteArray output=m_proc.readAllStandardOutput();
	QString s;
	s.append(output.trimmed());
	m_creatorUi->cmakeOutput->setPlainText(m_creatorUi->cmakeOutput->toPlainText()+s+'\n');
    m_creatorUi->cmakeOutput->verticalScrollBar()->setValue(m_creatorUi->cmakeOutput->verticalScrollBar()->maximum());
}

void CMakeBuildDirCreator::cmakeCommandDone(int exitCode, QProcess::ExitStatus exitStatus)
{
    /*QByteArray b = m_proc.readAllStandardOutput();
    QString t;
    t.prepend(b.trimmed());
    m_creatorUi->cmakeOutput->setPlainText(t);*/
    
    bool successful=exitCode==0 & exitStatus==QProcess::NormalExit;
    if(successful) {
        m_creatorUi->status->setText(i18n("Created successfully"));
    }
    else
    {
        m_creatorUi->status->setText(i18n("An error occurred"));
    }
    m_creatorUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(successful);
}

bool CMakeBuildDirCreator::isBuildDirProject(const KUrl& buildDir)
{
    QString srcfold=m_srcFolder.toLocalFile();
    if(srcfold.endsWith("/"))
        srcfold=srcfold.left(srcfold.size()-1);
    QFile file(buildDir.toLocalFile()+"/CMakeFiles/CMakeDirectoryInformation.cmake");
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        kWarning(9032) << "Something really strange happened at" << buildDir.toLocalFile()+"/CMakeDirectoryInformation.cmake";
        return false;
    }

    bool correct=false;
    const QString pLine=QString("SET(CMAKE_RELATIVE_PATH_TOP_SOURCE \"%1").arg(srcfold);
    while (!correct && !file.atEnd()) {
        QString line = file.readLine().trimmed();
        if(line.startsWith("#") || line.isEmpty())
            continue;
        
        if(line.startsWith(pLine))
            correct=true;
    }
    return correct;
}

void CMakeBuildDirCreator::updated()
{
    bool haveCMake=QFile::exists(m_creatorUi->cmakeBin->url().toLocalFile());
    m_creatorUi->buildFolder->setEnabled(haveCMake);
    m_creatorUi->installPrefix->setEnabled(haveCMake);
    m_creatorUi->buildType->setEnabled(haveCMake);
//  m_creatorUi->generator->setEnabled(haveCMake);
    m_creatorUi->run->setEnabled(haveCMake);
    m_creatorUi->cmakeOutput->setEnabled(false);
    if(!haveCMake)
    {
        m_creatorUi->status->setText(i18n("You need to select a cmake binary"));
        return;
    }

    bool dirEmpty=!m_creatorUi->buildFolder->url().isEmpty();
    bool alreadyCreated=false, correctProject=false;
    if(dirEmpty)
    {
        QDir d(m_creatorUi->buildFolder->url().toLocalFile());
        dirEmpty=d.exists() && d.count()<=2;
        if(!dirEmpty)
        {
            alreadyCreated=QFile::exists(m_creatorUi->buildFolder->url().toLocalFile()+"/CMakeCache.txt");
            if(alreadyCreated)
            {
                correctProject=isBuildDirProject(m_creatorUi->buildFolder->url());
            }
        }
    }

//     m_creatorUi->buildFolder->setEnabled(true);
    if(alreadyCreated && correctProject)
    {
        m_creatorUi->installPrefix->setEnabled(false);
        m_creatorUi->buildType->setEnabled(false);
//         m_creatorUi->generator->setEnabled(dirEmpty);
        m_creatorUi->run->setEnabled(false);
        m_creatorUi->status->setText(i18n("Using an already created build directory"));
        m_creatorUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
    else
    {
        m_creatorUi->installPrefix->setEnabled(dirEmpty);
        m_creatorUi->buildType->setEnabled(dirEmpty);
//         m_creatorUi->generator->setEnabled(dirEmpty);
        m_creatorUi->run->setEnabled(dirEmpty);
        m_creatorUi->run->setText(i18n("&Run"));
        m_creatorUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        if(dirEmpty)
            m_creatorUi->status->setText(i18n("Click run when you are ready"));
        else
        {
            //Useful to prevent disasters
            if(!alreadyCreated)
                m_creatorUi->status->setText(QString(i18n("The selected build directory does not exist or is not empty")));
            else
                m_creatorUi->status->setText(i18n("The selected build directory is for another kind of project"));
        }
    }
}

void CMakeBuildDirCreator::setCMakeBinary(const KUrl& url) { m_creatorUi->cmakeBin->setUrl(url); }

void CMakeBuildDirCreator::setInstallPrefix(const KUrl& url) { m_creatorUi->installPrefix->setUrl(url); }

void CMakeBuildDirCreator::setBuildFolder(const KUrl& url) { m_creatorUi->buildFolder->setUrl(url); }

void CMakeBuildDirCreator::setBuildType(const QString& s) { m_creatorUi->buildType->addItem(s); }

KUrl CMakeBuildDirCreator::cmakeBinary() const { return m_creatorUi->cmakeBin->url(); }

KUrl CMakeBuildDirCreator::installPrefix() const { return m_creatorUi->installPrefix->url(); }

KUrl CMakeBuildDirCreator::buildFolder() const { return m_creatorUi->buildFolder->url(); }

QString CMakeBuildDirCreator::buildType() const { return m_creatorUi->buildType->currentText(); }

QString CMakeBuildDirCreator::executeProcess(const QString& execName, const QStringList& args)
{
	kDebug(9032) << "Executing:" << execName << "::" << args /*<< "into" << *m_vars*/;

	KProcess p;
	p.setOutputChannelMode(KProcess::MergedChannels);
	p.setProgram(execName, args);
	p.start();

	if(!p.waitForFinished())
	{
		kDebug(9032) << "failed to execute:" << execName;
	}

	QByteArray b = p.readAllStandardOutput();
	QString t;
	t.prepend(b.trimmed());
	kDebug(9032) << "executed" << execName << "<" << t;

	return t;
}

#include "cmakebuilddircreator.h"
