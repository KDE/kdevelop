/*
    SPDX-FileCopyrightText: 2007 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakebuilddirchooser.h"
#include "ui_cmakebuilddirchooser.h"
#include "cmakeutils.h"
#include "debug.h"

#include <project/helper.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>

#include <KLocalizedString>

#include <QDir>
#include <QDialogButtonBox>
#include <QVBoxLayout>

using namespace KDevelop;

CMakeBuildDirChooser::CMakeBuildDirChooser(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Configure a Build Directory - %1", ICore::self()->runtimeController()->currentRuntime()->name()));

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto mainWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);

    m_chooserUi = new Ui::CMakeBuildDirChooser;
    m_chooserUi->setupUi(mainWidget);
    setShowAvailableBuildDirs(false);
    mainLayout->addWidget(m_buttonBox);

    m_chooserUi->buildFolder->setMode(KFile::Directory|KFile::ExistingOnly);
    m_chooserUi->installPrefix->setMode(KFile::Directory|KFile::ExistingOnly);

    // configure the extraArguments widget to span the widget width but not
    // expand the dialog to the width of the longest element in the argument history.
    m_chooserUi->extraArguments->setMinimumWidth(m_chooserUi->extraArguments->minimumSizeHint().width());
    m_extraArgumentsHistory = new CMakeExtraArgumentsHistory(m_chooserUi->extraArguments);

    connect(m_chooserUi->buildFolder, &KUrlRequester::textChanged, this, &CMakeBuildDirChooser::updated);
    connect(m_chooserUi->buildType, &QComboBox::currentTextChanged,
            this, &CMakeBuildDirChooser::updated);
    connect(m_chooserUi->extraArguments, &KComboBox::editTextChanged, this, &CMakeBuildDirChooser::updated);
    connect(m_chooserUi->availableBuildDirs, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CMakeBuildDirChooser::adoptPreviousBuildDirectory);

    const auto defaultInstallPrefix = ICore::self()->runtimeController()->currentRuntime()->getenv("KDEV_DEFAULT_INSTALL_PREFIX");
    if (!defaultInstallPrefix.isEmpty()) {
        m_chooserUi->installPrefix->setUrl(QUrl::fromLocalFile(QFile::decodeName(defaultInstallPrefix)));
    }

    updated();
}

CMakeBuildDirChooser::~CMakeBuildDirChooser()
{
    delete m_extraArgumentsHistory;

    delete m_chooserUi;
}

void CMakeBuildDirChooser::setProject( IProject* project )
{
    m_project = project;

    KDevelop::Path folder = m_project->path();
    QString relative=CMake::projectRootRelative(m_project);
    folder.cd(relative);
    m_srcFolder = folder;

    m_chooserUi->buildFolder->setUrl(KDevelop::proposedBuildFolder(m_srcFolder).toUrl());
    setWindowTitle(i18nc("@title:window", "Configure a Build Directory for %1", project->name()));
    update();
}

void CMakeBuildDirChooser::buildDirSettings(
    const KDevelop::Path& buildDir,
    QString& srcDir,
    QString& installDir,
    QString& buildType)
{
    const QByteArray srcLine = QByteArrayLiteral("CMAKE_HOME_DIRECTORY:INTERNAL=");
    const QByteArray installLine = QByteArrayLiteral("CMAKE_INSTALL_PREFIX:PATH=");
    const QByteArray buildLine = QByteArrayLiteral("CMAKE_BUILD_TYPE:STRING=");

    const Path cachePath(buildDir, QStringLiteral("CMakeCache.txt"));
    QFile file(cachePath.toLocalFile());

    srcDir.clear();
    installDir.clear();
    buildType.clear();

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCWarning(CMAKE) << "Something really strange happened reading" << cachePath;
        return;
    }

    int cnt = 0;
    while (cnt != 3 && !file.atEnd())
    {
        auto rawLine = file.readLine();

        if (rawLine.endsWith('\n'))
            rawLine.chop(1);
        if (rawLine.endsWith('\r'))
            rawLine.chop(1);

        auto match = [&rawLine](const QByteArray& prefix, QString* target) -> bool
        {
            if (rawLine.startsWith(prefix)) {
                // note: CMakeCache.txt is UTF8-encoded, also see bug 329305
                *target = QString::fromUtf8(rawLine.constData() + prefix.size(), rawLine.size() - prefix.size());
                return true;
            }
            return false;
        };

        if (match(srcLine, &srcDir) || match(installLine, &installDir) || match(buildLine, &buildType)) {
            ++cnt;
        }
    }

    qCDebug(CMAKE) << "The source directory for " << file.fileName() << "is" << srcDir;
    qCDebug(CMAKE) << "The install directory for " << file.fileName() << "is" << installDir;
    qCDebug(CMAKE) << "The build type for " << file.fileName() << "is" << buildType;
}

void CMakeBuildDirChooser::updated()
{
    StatusTypes st;
    Path chosenBuildFolder(m_chooserUi->buildFolder->url());
    bool emptyUrl = chosenBuildFolder.isEmpty();
    if( emptyUrl ) st |= BuildFolderEmpty;

    bool dirEmpty = false, dirExists= false, dirRelative = false;
    QString srcDir;
    QString installDir;
    QString buildType;
    if(!emptyUrl)
    {
        QDir d(chosenBuildFolder.toLocalFile());
        dirExists = d.exists();
        dirEmpty = dirExists && d.count()<=2;
        dirRelative = d.isRelative();
        if(!dirEmpty && dirExists && !dirRelative)
        {
            bool hasCache=QFile::exists(Path(chosenBuildFolder, QStringLiteral("CMakeCache.txt")).toLocalFile());
            if(hasCache)
            {
                QString proposed=m_srcFolder.toLocalFile();

                buildDirSettings(chosenBuildFolder, srcDir, installDir, buildType);
                if(!srcDir.isEmpty())
                {
                    auto rt = ICore::self()->runtimeController()->currentRuntime();
                    if(QDir(rt->pathInHost(Path(srcDir)).toLocalFile()).canonicalPath() == QDir(proposed).canonicalPath())
                    {
                            st |= CorrectBuildDir | BuildDirCreated;
                    }
                }
                else
                {
                    qCWarning(CMAKE) << "maybe you are trying a damaged CMakeCache.txt file. Proper: ";
                }

                if(!installDir.isEmpty() && QDir(installDir).exists())
                {
                    m_chooserUi->installPrefix->setUrl(QUrl::fromLocalFile(installDir));
                }

                m_chooserUi->buildType->setCurrentText(buildType);
            }
        }

        if(m_alreadyUsed.contains(chosenBuildFolder.toLocalFile()) && !m_chooserUi->availableBuildDirs->isEnabled()) {
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

void CMakeBuildDirChooser::setCMakeExecutable(const Path& path)
{
    m_chooserUi->cmakeExecutable->setUrl(path.toUrl());
    updated();
}

void CMakeBuildDirChooser::setInstallPrefix(const Path& path)
{
    m_chooserUi->installPrefix->setUrl(path.toUrl());
    updated();
}

void CMakeBuildDirChooser::setBuildFolder(const Path& path)
{
    m_chooserUi->buildFolder->setUrl(path.toUrl());
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
    m_chooserUi->availableBuildDirs->addItems(used);
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
    m_chooserUi->status->setMessageType(canApply ? KMessageWidget::Positive : KMessageWidget::Warning);
    m_chooserUi->status->setText(message);

    auto okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(canApply);
    if (canApply) {
        auto cancelButton = m_buttonBox->button(QDialogButtonBox::Cancel);
        cancelButton->clearFocus();
    }
}

void CMakeBuildDirChooser::adoptPreviousBuildDirectory(int index)
{
    if (index > 0) {
        Q_ASSERT(m_project);
        m_chooserUi->cmakeExecutable->setUrl(CMake::currentCMakeExecutable(m_project, index -1).toUrl());
        m_chooserUi->buildFolder->setUrl(CMake::currentBuildDir(m_project, index -1).toUrl());
        m_chooserUi->installPrefix->setUrl(CMake::currentInstallDir(m_project, index -1).toUrl());
        m_chooserUi->buildType->setCurrentText(CMake::currentBuildType(m_project, index -1));
        m_chooserUi->extraArguments->setCurrentText(CMake::currentExtraArguments(m_project, index -1));
    }

    m_chooserUi->label_5->setEnabled(index == 0);
    m_chooserUi->cmakeExecutable->setEnabled(index == 0);
    m_chooserUi->label_3->setEnabled(index == 0);
    m_chooserUi->buildFolder->setEnabled(index == 0);
    m_chooserUi->label->setEnabled(index == 0);
    m_chooserUi->installPrefix->setEnabled(index == 0);
    m_chooserUi->label_2->setEnabled(index == 0);
    m_chooserUi->buildType->setEnabled(index == 0);
    m_chooserUi->status->setEnabled(index == 0);
    m_chooserUi->extraArguments->setEnabled(index == 0);
    m_chooserUi->label_4->setEnabled(index == 0);
}

bool CMakeBuildDirChooser::reuseBuilddir()
{
    return m_chooserUi->availableBuildDirs->currentIndex() > 0;
}

int CMakeBuildDirChooser::alreadyUsedIndex() const
{
    return m_chooserUi->availableBuildDirs->currentIndex() - 1;
}

void CMakeBuildDirChooser::setShowAvailableBuildDirs(bool show)
{
    m_chooserUi->availableLabel->setVisible(show);
    m_chooserUi->availableBuildDirs->setVisible(show);
}

Path CMakeBuildDirChooser::cmakeExecutable() const { return Path(m_chooserUi->cmakeExecutable->url()); }

Path CMakeBuildDirChooser::installPrefix() const { return Path(m_chooserUi->installPrefix->url()); }

Path CMakeBuildDirChooser::buildFolder() const { return Path(m_chooserUi->buildFolder->url()); }

QString CMakeBuildDirChooser::buildType() const { return m_chooserUi->buildType->currentText(); }

QString CMakeBuildDirChooser::extraArguments() const { return m_chooserUi->extraArguments->currentText(); }

#include "moc_cmakebuilddirchooser.cpp"
