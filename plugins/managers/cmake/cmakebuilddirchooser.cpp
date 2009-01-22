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

#include "cmakebuilddirchooser.h"
#include <QDir>
#include <KDebug>
#include <KProcess>
#include <KMessageBox>
#include "ui_cmakebuilddirchooser.h"

CMakeBuildDirChooser::CMakeBuildDirChooser(QWidget* parent)
    : QWidget(parent)
{
    m_chooserUi = new Ui::CMakeBuildDirChooser;
    m_chooserUi->setupUi( this );
    m_chooserUi->buildFolder->setMode(KFile::Directory|KFile::ExistingOnly);

    QString cmakeBin=executeProcess("which", QStringList("cmake"));
    setCMakeBinary(KUrl(cmakeBin));
    
    connect(m_chooserUi->cmakeBin, SIGNAL(textChanged(const QString &)), this, SLOT(updated()));
    connect(m_chooserUi->buildFolder, SIGNAL(textChanged(const QString &)), this, SLOT(updated()));
    connect(m_chooserUi->buildType, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(updated()));
    updated();
}

void CMakeBuildDirChooser::setSourceFolder( const KUrl& srcFolder )
{
    m_srcFolder = srcFolder;
    m_chooserUi->buildFolder->setUrl(KUrl( srcFolder.toLocalFile() + "/build" ) );
    update();
}

QString CMakeBuildDirChooser::buildDirProject(const KUrl& buildDir)
{
    QFile file(buildDir.toLocalFile()+"/CMakeCache.txt");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        kWarning(9032) << "Something really strange happened reading CMakeCache.txt";
        return "";
    }

    QString ret;
    bool correct=false;
    const QString pLine="CMAKE_HOME_DIRECTORY:INTERNAL=";
    while (!correct && !file.atEnd())
    {
        QString line = file.readLine().trimmed();
        if(line.startsWith('#') || line.isEmpty())
            continue;

        if(line.startsWith(pLine))
        {
            correct=true;
            ret=line.mid(pLine.count());
        }
    }
    kDebug(9042) << "The source directory for " << buildDir << "is" << ret;
    return ret;
}

void CMakeBuildDirChooser::updated()
{
    bool haveCMake=QFile::exists(m_chooserUi->cmakeBin->url().toLocalFile());
    StatusTypes st;
    if( haveCMake ) st |= HaveCMake;

    m_chooserUi->buildFolder->setEnabled(haveCMake);
    m_chooserUi->installPrefix->setEnabled(haveCMake);
    m_chooserUi->buildType->setEnabled(haveCMake);
//  m_chooserUi->generator->setEnabled(haveCMake);
    if(!haveCMake)
    {
        emit status(i18n("You need to select a cmake binary"));
        emit updated( st, "" );
        return;
    }

    bool emptyUrl=m_chooserUi->buildFolder->url().isEmpty();
    if( emptyUrl ) st |= BuildFolderEmpty;

    bool alreadyCreated=false, correctProject=false, dirEmpty = false, dirExists=false;
    QString srcDir;
    if(!emptyUrl)
    {
        QDir d(m_chooserUi->buildFolder->url().toLocalFile());
        dirExists = d.exists();
        dirEmpty=dirExists && d.count()<=2;
        if(!dirEmpty && dirExists)
        {
            alreadyCreated=QFile::exists(m_chooserUi->buildFolder->url().toLocalFile()+"/CMakeCache.txt");
            if(alreadyCreated)
            {
                QString srcfold=m_srcFolder.toLocalFile(KUrl::RemoveTrailingSlash);

                srcDir=buildDirProject(m_chooserUi->buildFolder->url());
                if(!srcDir.isEmpty())
                    correctProject= (QDir(srcDir).canonicalPath()==QDir(srcfold).canonicalPath());
                else
                    kWarning(9042) << "maybe you are trying a damaged CMakeCache.txt file";
            }
        }
    }else
    {
        emit status(i18n("You need to specify a build directory"));
        emit updated( st, "" );
        return;
    }

    if( alreadyCreated ) st |= BuildDirCreated;
    if( correctProject ) st |= CorrectProject;

    if(alreadyCreated && correctProject)
    {
        m_chooserUi->installPrefix->setEnabled(false);
        m_chooserUi->buildType->setEnabled(false);
        emit status(i18n("Using an already created build directory"));
    }
    else
    {
        bool correct=dirEmpty || !dirExists;
        if(correct)
        {
            st |= CorrectBuildDir;
            emit status(QString());
        }
        else
        {
            //Useful to prevent disasters
            if(alreadyCreated)
                emit status(i18n("The selected build directory is not empty"));
            else if ( alreadyCreated && !correctProject )
                emit status(i18n("This build directory is for %1, "
                        "but the project directory is %2", srcDir, m_srcFolder.toLocalFile()));
        }

        m_chooserUi->installPrefix->setEnabled(correct);
        m_chooserUi->buildType->setEnabled(correct);
    }
    emit updated( st, srcDir );
}

void CMakeBuildDirChooser::setCMakeBinary(const KUrl& url) 
{ 
    m_chooserUi->cmakeBin->setUrl(url); 
    update();
}

void CMakeBuildDirChooser::setInstallPrefix(const KUrl& url) 
{ 
    m_chooserUi->installPrefix->setUrl(url); 
    update();
}

void CMakeBuildDirChooser::setBuildFolder(const KUrl& url) 
{ 
    m_chooserUi->buildFolder->setUrl(url); 
    update();
}

void CMakeBuildDirChooser::setBuildType(const QString& s) 
{ 
    m_chooserUi->buildType->addItem(s); 
    update();
}

KUrl CMakeBuildDirChooser::cmakeBinary() const { return m_chooserUi->cmakeBin->url(); }

KUrl CMakeBuildDirChooser::installPrefix() const { return m_chooserUi->installPrefix->url(); }

KUrl CMakeBuildDirChooser::buildFolder() const { return m_chooserUi->buildFolder->url(); }

QString CMakeBuildDirChooser::buildType() const { return m_chooserUi->buildType->currentText(); }

QString CMakeBuildDirChooser::executeProcess(const QString& execName, const QStringList& args)
{
    kDebug(9042) << "Executing:" << execName << "::" << args /*<< "into" << *m_vars*/;

    KProcess p;
    p.setOutputChannelMode(KProcess::MergedChannels);
    p.setProgram(execName, args);
    p.start();

    if(!p.waitForFinished())
    {
        kDebug(9042) << "failed to execute:" << execName;
    }

    QByteArray b = p.readAllStandardOutput();
    QString t;
    t.prepend(b.trimmed());
    kDebug(9042) << "executed" << execName << "<" << t;

    return t;
}

#include "cmakebuilddirchooser.moc"

