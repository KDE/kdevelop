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
#include <KStandardDirs>
#include "ui_cmakebuilddirchooser.h"

#include <KColorScheme>

static const int maxExtraArgumentsInHistory = 15;

CMakeBuildDirChooser::CMakeBuildDirChooser(QWidget* parent)
    : KDialog(parent)
{
    setCaption(i18n("Configure a build directory"));
    
//     QWidget* w= new QWidget(this);
    m_chooserUi = new Ui::CMakeBuildDirChooser;
    m_chooserUi->setupUi(mainWidget());
    m_chooserUi->buildFolder->setMode(KFile::Directory|KFile::ExistingOnly);
    m_chooserUi->installPrefix->setMode(KFile::Directory|KFile::ExistingOnly);
//     setMainWidget(w);

    QString cmakeBin=KStandardDirs::findExe( "cmake" );
    setCMakeBinary(KUrl(cmakeBin));

    KConfigGroup config = KGlobal::config()->group("CMakeBuildDirChooser");
    QStringList lastExtraArguments = config.readEntry("LastExtraArguments", QStringList());;
    m_chooserUi->extraArguments->addItem("");
    m_chooserUi->extraArguments->addItems(lastExtraArguments);
    m_chooserUi->extraArguments->setInsertPolicy(QComboBox::InsertAtTop);
    KCompletion *comp = m_chooserUi->extraArguments->completionObject();
    connect(m_chooserUi->extraArguments, SIGNAL(returnPressed(const QString&)), comp, SLOT(addItem(QString)));
    comp->insertItems(lastExtraArguments);

    connect(m_chooserUi->cmakeBin, SIGNAL(textChanged(QString)), this, SLOT(updated()));
    connect(m_chooserUi->buildFolder, SIGNAL(textChanged(QString)), this, SLOT(updated()));
    connect(m_chooserUi->buildType, SIGNAL(currentIndexChanged(QString)), this, SLOT(updated()));
    connect(m_chooserUi->extraArguments, SIGNAL(editTextChanged(QString)), this, SLOT(updated()));
    updated();
}

CMakeBuildDirChooser::~CMakeBuildDirChooser()
{
    KConfigGroup config = KGlobal::config()->group("CMakeBuildDirChooser");
    config.writeEntry("LastExtraArguments", extraArgumentsHistory());
    config.sync();

    delete m_chooserUi;
}

void CMakeBuildDirChooser::setSourceFolder( const KUrl& srcFolder )
{
    m_srcFolder = srcFolder;

    KUrl proposedBuildUrl;
    if (srcFolder.path().contains("/src/"))
    {
        const QString srcBuildPath = srcFolder.path().replace("/src/", "/build/");
        if (QDir(srcBuildPath).exists())
        {
            proposedBuildUrl = KUrl(srcBuildPath);
        }
    }
    if (proposedBuildUrl.isEmpty())
    {
        proposedBuildUrl = KUrl( srcFolder.toLocalFile() + "/build" );
    }

    proposedBuildUrl.cleanPath();
    m_chooserUi->buildFolder->setUrl(proposedBuildUrl);
    setCaption(i18n("Configure a build directory for %1", srcFolder.toLocalFile()));
    update();
}

QString CMakeBuildDirChooser::buildDirProject(const KUrl& srcDir)
{
    QFile file(srcDir.toLocalFile()+"/CMakeCache.txt");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        kWarning(9032) << "Something really strange happened reading CMakeCache.txt";
        return QString();
    }

    QString ret;
    bool correct=false;
    const QString pLine="CMAKE_HOME_DIRECTORY:INTERNAL=";
    while (!correct && !file.atEnd())
    {
        // note: CMakeCache.txt is UTF8-encoded, also see bug 329305
        QString line = QString::fromUtf8(file.readLine().trimmed());
        if(line.startsWith(pLine))
        {
            correct=true;
            ret=line.mid(pLine.count());
        }
    }
    kDebug(9042) << "The source directory for " << file.fileName() << "is" << ret;
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
        setStatus(i18n("You need to select a cmake binary."), false);
        return;
    }

    bool emptyUrl=m_chooserUi->buildFolder->url().isEmpty();
    if( emptyUrl ) st |= BuildFolderEmpty;

    bool dirEmpty = false, dirExists= false, dirRelative = false;
    QString srcDir;
    if(!emptyUrl)
    {
        QDir d(m_chooserUi->buildFolder->url().toLocalFile());
        dirExists = d.exists();
        dirEmpty = dirExists && d.count()<=2;
        dirRelative = d.isRelative();
        if(!dirEmpty && dirExists && !dirRelative)
        {
            bool hasCache=QFile::exists(m_chooserUi->buildFolder->url().toLocalFile()+"/CMakeCache.txt");
            if(hasCache)
            {
                QString proposed=m_srcFolder.toLocalFile(KUrl::RemoveTrailingSlash);

                srcDir=buildDirProject(m_chooserUi->buildFolder->url());
                if(!srcDir.isEmpty())
                {
                    if(QDir(srcDir).canonicalPath()==QDir(proposed).canonicalPath())
                    {
                            st |= CorrectBuildDir | BuildDirCreated;
                    }
                }
                else
                {
                    kWarning(9042) << "maybe you are trying a damaged CMakeCache.txt file. Proper: ";
                }
            }
        }
        
        if(m_alreadyUsed.contains(buildFolder().toLocalFile(KUrl::RemoveTrailingSlash))) {
            st=DirAlreadyCreated;
        }
    }
    else
    {
        setStatus(i18n("You need to specify a build directory."), false);
        return;
    }
    
    
    if(st & (BuildDirCreated | CorrectBuildDir))
    {
        setStatus(i18n("Using an already created build directory."), true);
        m_chooserUi->installPrefix->setEnabled(false);
        m_chooserUi->buildType->setEnabled(false);
    }
    else
    {
        bool correct = (dirEmpty || !dirExists) && !(st & DirAlreadyCreated) && !dirRelative;
        
        if(correct)
        {
            st |= CorrectBuildDir;
            setStatus(i18n("Creating a new build directory."), true);
        }
        else
        {
            //Useful to explain what's going wrong
            if(st & DirAlreadyCreated)
                setStatus(i18n("Build directory already configured."), false);
            else if (!srcDir.isEmpty())
                setStatus(i18n("This build directory is for %1, "
                               "but the project directory is %2.", srcDir, m_srcFolder.toLocalFile()), false);
            else if(dirRelative)
                setStatus(i18n("You may not select a relative build directory."), false);
            else if(!dirEmpty)
                setStatus(i18n("The selected build directory is not empty."), false);
        }

        m_chooserUi->installPrefix->setEnabled(correct);
        m_chooserUi->buildType->setEnabled(correct);
    }
}

void CMakeBuildDirChooser::setCMakeBinary(const KUrl& url) 
{ 
    m_chooserUi->cmakeBin->setUrl(url); 
    updated();
}

void CMakeBuildDirChooser::setInstallPrefix(const KUrl& url) 
{ 
    m_chooserUi->installPrefix->setUrl(url); 
    updated();
}

void CMakeBuildDirChooser::setBuildFolder(const KUrl& url) 
{ 
    m_chooserUi->buildFolder->setUrl(url); 
    updated();
}

void CMakeBuildDirChooser::setBuildType(const QString& s) 
{
    m_chooserUi->buildType->addItem(s);
    m_chooserUi->buildType->setCurrentIndex(m_chooserUi->buildType->findText(s));
    updated();
}

void CMakeBuildDirChooser::setAlreadyUsed (const QStringList & used)
{
    m_alreadyUsed = used;
    updated();
}

void CMakeBuildDirChooser::setExtraArguments(const QString& args)
{
    m_chooserUi->extraArguments->setEditText(args);
    updated();
}

void CMakeBuildDirChooser::setStatus(const QString& message, bool canApply)
{
    KColorScheme scheme(QPalette::Normal);
    KColorScheme::ForegroundRole role;
    if (canApply) {
        role = KColorScheme::PositiveText;
    } else {
        role = KColorScheme::NegativeText;
    }
    m_chooserUi->status->setText(QString("<i><font color='%1'>%2</font></i>").arg(scheme.foreground(role).color().name()).arg(message));
    button(Ok)->setEnabled(canApply);
}

KUrl CMakeBuildDirChooser::cmakeBinary() const { return m_chooserUi->cmakeBin->url(); }

KUrl CMakeBuildDirChooser::installPrefix() const { return m_chooserUi->installPrefix->url(); }

KUrl CMakeBuildDirChooser::buildFolder() const { return m_chooserUi->buildFolder->url(); }

QString CMakeBuildDirChooser::buildType() const { return m_chooserUi->buildType->currentText(); }

QString CMakeBuildDirChooser::extraArguments() const { return m_chooserUi->extraArguments->currentText(); }

QStringList CMakeBuildDirChooser::extraArgumentsHistory() const
{
    QStringList list;
    KComboBox* extraArguments = m_chooserUi->extraArguments;
    if (!extraArguments->currentText().isEmpty()) {
        list << extraArguments->currentText();
    }
    for (int i = 0; i < qMin(maxExtraArgumentsInHistory, extraArguments->count()); ++i) {
        if (!extraArguments->itemText(i).isEmpty() &&
            (extraArguments->currentText() != extraArguments->itemText(i))) {
            list << extraArguments->itemText(i);
        }
    }
    return list;
}

#include "cmakebuilddirchooser.moc"
