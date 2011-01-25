#include "qmakebuilddirchooser.h"
#include <KStandardDirs>

QMakeBuildDirChooser::QMakeBuildDirChooser(KDevelop::IProject* project, QWidget* parent)
    : KDialog(parent), Ui::QMakeBuildDirChooser(), m_project(project)
{
    setButtons( KDialog::Ok | KDialog::Cancel );
    setCaption( i18n("Configure QMake build settings") );
    setDefaultButton( KDialog::Ok );
    
    setupUi(mainWidget());
    
    KConfigGroup cg(project->projectConfiguration(), "QMake Builder");
    
    cmakeBin->setUrl( cg.readEntry<KUrl>("QMake Binary", KUrl(KStandardDirs::findExe("qmake"))) );
    KUrl proposedBuildUrl( project->folder().toLocalFile() + "/build" );
    proposedBuildUrl.cleanPath();
    buildFolder->setUrl( cg.readEntry<KUrl>("Build Directory", proposedBuildUrl) );
    installPrefix->setUrl( cg.readEntry<KUrl>("Install Prefix", KUrl("")) );
    extraArguments->setText( cg.readEntry("Extra Arguments") );
}

QMakeBuildDirChooser::~QMakeBuildDirChooser()
{ }


