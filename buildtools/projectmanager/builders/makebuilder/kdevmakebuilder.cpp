
#include "kdevmakebuilder.h"

#include <kdevproject.h>
#include <kdevcore.h>
#include <kdevmakefrontend.h>
#include <domutil.h>
#include <makeoptionswidget.h>

#include <kgenericfactory.h>
#include <kdialogbase.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <qvbox.h>

K_EXPORT_COMPONENT_FACTORY(libkdevmakebuilder, KGenericFactory<KDevMakeBuilder>("kdevmakebuilder"))


const QString &KDevMakeBuilder::builder =
    KGlobal::staticQString("/kdevprojectmanager/builder/make");
    
const QString &KDevMakeBuilder::makeTool =
    KGlobal::staticQString("/kdevprojectmanager/builder/make/makebin");
    
const QString &KDevMakeBuilder::priority =
    KGlobal::staticQString("/kdevprojectmanager/builder/make/prio");
    
const QString &KDevMakeBuilder::abortOnError =
    KGlobal::staticQString("/kdevprojectmanager/builder/make/abortonerror");
    
const QString &KDevMakeBuilder::numberOfJobs =
    KGlobal::staticQString("/kdevprojectmanager/builder/make/numberofjobs");
    
const QString &KDevMakeBuilder::dontAct = 
    KGlobal::staticQString("/kdevprojectmanager/builder/make/dontact");

KDevMakeBuilder::KDevMakeBuilder(QObject *parent, const char *name, const QStringList &)
    : KDevProjectBuilder(parent, name)
{
    m_project = ::qt_cast<KDevProject*>(parent);
    Q_ASSERT(m_project);
    
    connect(project()->core(), SIGNAL(projectConfigWidget(KDialogBase*)),
        this, SLOT(projectConfigWidget(KDialogBase*)));
        
    if (KDevMakeFrontend *make = project()->makeFrontend()) {
        connect(make, SIGNAL(commandFinished(const QString &)),
            this, SLOT(commandFinished(const QString &)));
            
        connect(make, SIGNAL(commandFailed(const QString &)),
            this, SLOT(commandFailed(const QString &)));
    }
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

bool KDevMakeBuilder::configure(ProjectItemDom dom)
{
    Q_UNUSED(dom);
    return false;
}
    
bool KDevMakeBuilder::build(ProjectItemDom dom)
{
    if (KDevMakeFrontend *make = project()->makeFrontend()) {
        if (ProjectFolderDom folder = dom->toFolder()) {
            // ### compile the folder
            QString command = "make";
            make->queueCommand(folder->name(), command); // ### use the make-tool variable
            m_commands.append(qMakePair(command, dom));
            return true;
        } else if (ProjectTargetDom target = dom->toTarget()) {
            // ### compile the target
        } else if (ProjectFileDom file = dom->toFile()) {
            // ### compile the file
        }
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
    MakeOptionsWidget *widget = new MakeOptionsWidget(*project()->projectDom(), builder, vbox);
    connect(dlg, SIGNAL(okClicked()), widget, SLOT(accept()));
}

void KDevMakeBuilder::commandFinished(const QString &command)
{
    if (!m_commands.isEmpty()) {
        QPair<QString, ProjectItemDom> item = m_commands.first();
        if (item.first == command) {
            m_commands.pop_front();
            emit builded(item.second);
        }
    }
}

void KDevMakeBuilder::commandFailed(const QString &command)
{
    if (!m_commands.isEmpty()) {
        QPair<QString, ProjectItemDom> item = m_commands.first();
        if (item.first == command) {
            m_commands.clear();
            emit failed(item.second);
        }
    }
}

