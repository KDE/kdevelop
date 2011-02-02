#include "qmakebuilddirchooser.h"
#include "qmakeconfig.h"
#include <KStandardDirs>
#include <KDebug>

QMakeBuildDirChooser::QMakeBuildDirChooser(KDevelop::IProject* project, QWidget* parent)
    : KDialog(parent), Ui::QMakeBuildDirChooser(), m_project(project)
{
    setButtons( KDialog::Ok | KDialog::Cancel );
    setCaption( i18n("Configure QMake build settings") );
    setDefaultButton( KDialog::Ok );
    
    setupUi(mainWidget());
    
    KConfigGroup cg(project->projectConfiguration(), QMakeConfig::CONFIG_GROUP);
    
    qmakeBin->setUrl( cg.readEntry<KUrl>(QMakeConfig::QMAKE_BINARY, KUrl(KStandardDirs::findExe("qmake"))) );
    KUrl proposedBuildUrl( project->folder().toLocalFile() + "/build" );
    proposedBuildUrl.cleanPath();
    buildFolder->setUrl( cg.readEntry<KUrl>(QMakeConfig::BUILD_FOLDER, proposedBuildUrl) );
    installPrefix->setUrl( cg.readEntry<KUrl>(QMakeConfig::INSTALL_PREFIX, KUrl("")) );
    extraArguments->setText( cg.readEntry(QMakeConfig::EXTRA_ARGUMENTS) );
    buildType->setCurrentIndex( cg.readEntry<int>(QMakeConfig::BUILD_TYPE, 0) );
    status->setText("");
    
    //save; like this, we can be sure to have a qmake binary and build path set
    //(even if user clicks Cancel)
    saveConfig();
    
    connect(qmakeBin, SIGNAL(textChanged(QString)), this, SLOT(validate()));
    connect(buildFolder, SIGNAL(textChanged(QString)), this, SLOT(validate()));
    connect(installPrefix, SIGNAL(textChanged(QString)), this, SLOT(validate()));
    //connect(extraArguments, SIGNAL(textChanged(QString)), this, SLOT(validate()));
}

QMakeBuildDirChooser::~QMakeBuildDirChooser()
{
    //don't save in destructor; instead, on click of OK-button
    //saveConfig();
}


void QMakeBuildDirChooser::saveConfig()
{
    kDebug() << "Writing config";
    KConfigGroup cg(m_project->projectConfiguration(), QMakeConfig::CONFIG_GROUP);
    cg.writeEntry<KUrl>(QMakeConfig::QMAKE_BINARY, qmakeBin->url());
    cg.writeEntry<KUrl>(QMakeConfig::BUILD_FOLDER, buildFolder->url());
    cg.writeEntry<KUrl>(QMakeConfig::INSTALL_PREFIX, installPrefix->url());
    cg.writeEntry(QMakeConfig::EXTRA_ARGUMENTS, extraArguments->text());
    cg.writeEntry<int>(QMakeConfig::BUILD_TYPE, buildType->currentIndex());
    cg.sync();
}


void QMakeBuildDirChooser::slotButtonClicked(int button)
{
    if(button == KDialog::Ok) 
    {
        if(isValid())
        {
            saveConfig();
            accept();
        }
        else
        {
            kDebug() << "OK-button not accepted, input invalid";
        }
    }
    else
    {
        kDebug() << "button != OK";
        KDialog::slotButtonClicked(button);
    }
}



bool QMakeBuildDirChooser::isValid()
{
    bool valid = true;
    if(qmakeBin->url().isEmpty())
    {
        status->setText(i18n("Please specify path to QMake binary"));
        valid = false;
    }
    else if(!qmakeBin->url().isValid())
    {
        status->setText(i18n("QMake binary path is invalid"));
        valid = false;
    }
    else if(!qmakeBin->url().isLocalFile())
    {
        status->setText(i18n("QMake binary must be a local path"));
        valid = false;        
    }
    
    if(buildFolder->url().isEmpty()) 
    {
        status->setText(i18n("Please specify a build folder"));
        valid = false;
    }
    else if(!buildFolder->url().isValid())
    {
        status->setText(i18n("Build folder is invalid"));
        valid = false;
    }
    else if(!buildFolder->url().isLocalFile())
    {
        status->setText(i18n("Build folder must be a local path"));
        valid = false;
    }

    if(!installPrefix->url().isEmpty() && !installPrefix->url().isValid())
    {
        status->setText(i18n("Install prefix is invalid (may also be left empty)"));
        valid = false;
    }
    if(!installPrefix->url().isEmpty() && !installPrefix->url().isLocalFile())
    {
        status->setText(i18n("Install prefix must be a local path (may also be left empty)"));
        valid = false;        
    }
    
    if(valid)
        status->setText("");
    kDebug() << "VALID == " << valid;
    button(KDialog::Ok)->setEnabled(valid);
    return valid;
}

void QMakeBuildDirChooser::validate()
{
    isValid();
}
