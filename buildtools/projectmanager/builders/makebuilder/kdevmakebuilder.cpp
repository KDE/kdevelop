
#include "kdevmakebuilder.h"

#include <kdevproject.h>
#include <kdevcore.h>
#include <domutil.h>
#include <makeoptionswidget.h>

#include <kgenericfactory.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kdebug.h>

#include <qvbox.h>

K_EXPORT_COMPONENT_FACTORY(libkdevmakebuilder, KGenericFactory<KDevMakeBuilder>("kdevmakebuilder"))

KDevMakeBuilder::KDevMakeBuilder(QObject *parent, const char *name, const QStringList &)
    : KDevProjectBuilder(parent, name)
{
    m_project = ::qt_cast<KDevProject*>(parent);
    Q_ASSERT(m_project);
    
    connect(project()->core(), SIGNAL(projectConfigWidget(KDialogBase*)),
        this, SLOT(projectConfigWidget(KDialogBase*)));
}

KDevMakeBuilder::~KDevMakeBuilder()
{
}

KDevProject *KDevMakeBuilder::project() const
{
    return m_project;
}

bool KDevMakeBuilder::isExecutable(ProjectItemDom dom) const
{
    Q_UNUSED(dom);
    return false;
}

ProjectItemDom KDevMakeBuilder::defaultExecutable() const
{
    return m_defaultExecutable;
}

void KDevMakeBuilder::setDefaultExecutable(ProjectItemDom dom)
{
    m_defaultExecutable = dom;
}

bool KDevMakeBuilder::build(ProjectItemDom dom)
{
    // ### we need the KDevPartController here!!
    
    if (ProjectFolderDom folder = dom->toFolder()) {
        // ### compile the folder
    } else if (ProjectTargetDom target = dom->toTarget()) {
        // ### compile the target
    } else if (ProjectFileDom file = dom->toFile()) {
        // ### compile the file
    }
    
    return false;
}

bool KDevMakeBuilder::clean(ProjectItemDom dom)
{
    Q_UNUSED(dom);
    return false;
}

bool KDevMakeBuilder::execute(ProjectItemDom dom)
{
    Q_UNUSED(dom);
    return false;
}

void KDevMakeBuilder::projectConfigWidget(KDialogBase *dlg)
{
    Q_ASSERT(project());
    
    QVBox *vbox = dlg->addVBoxPage(i18n("Make Options"));
    MakeOptionsWidget *widget = new MakeOptionsWidget(*project()->projectDom(), "/kdevprojectmanager/builder/make", vbox);
    connect(dlg, SIGNAL(okClicked()), widget, SLOT(accept()));
}

