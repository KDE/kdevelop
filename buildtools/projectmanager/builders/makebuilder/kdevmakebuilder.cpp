
#include <config.h>
#include "kdevmakebuilder.h"
#include <kdevprojectmodel.h>

#include <kdevproject.h>
#include <kdevcore.h>
#include <kdevmakefrontend.h>
#include <domutil.h>

#if 0 // ### port me
#include <makeoptionswidget.h>
#endif

#include <kiconloader.h>
#include <kgenericfactory.h>
#include <kprocess.h>
#include <kdialogbase.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <q3vbox.h>

K_EXPORT_COMPONENT_FACTORY(libkdevmakebuilder, KGenericFactory<KDevMakeBuilder>("kdevmakebuilder"))


const QString &KDevMakeBuilder::builder =
    KGlobal::staticQString("/kdevprojectmanager/builder");

const QString &KDevMakeBuilder::makeTool =
    KGlobal::staticQString("/kdevprojectmanager/builder/make/makebin");

const QString &KDevMakeBuilder::priority =
    KGlobal::staticQString("/kdevprojectmanager/builder/make/prio");

const QString &KDevMakeBuilder::abortOnError =
    KGlobal::staticQString("/kdevprojectmanager/builder/make/abortonerror");

const QString &KDevMakeBuilder::numberOfJobs =
    KGlobal::staticQString("/kdevprojectmanager/builder/make/numberofjobsfile:///usr/share/doc/HTML/index.html");

const QString &KDevMakeBuilder::dontAct =
    KGlobal::staticQString("/kdevprojectmanager/builder/make/dontact");

const QString &KDevMakeBuilder::environment =
    KGlobal::staticQString("/kdevprojectmanager/builder/make/envvars");

KDevMakeBuilder::KDevMakeBuilder(QObject *parent, const char *name, const QStringList &)
    : KDevProjectBuilder(parent)
{
    setObjectName(QString::fromUtf8(name));

    m_project = qobject_cast<KDevProject*>(parent);
    Q_ASSERT(m_project);

    connect(project()->core(), SIGNAL(projectConfigWidget(KDialogBase*)),
        this, SLOT(projectConfigWidget(KDialogBase*)));

    if (KDevMakeFrontend *make = project()->extension<KDevMakeFrontend>("KDevelop/MakeFrontend")) {
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

bool KDevMakeBuilder::isExecutable(KDevProjectItem *dom) const
{
    Q_UNUSED(dom);
    return false;
}

KDevProjectItem *KDevMakeBuilder::defaultExecutable() const
{
    return m_defaultExecutable;
}

void KDevMakeBuilder::setDefaultExecutable(KDevProjectItem *dom)
{
    m_defaultExecutable = dom;
}

bool KDevMakeBuilder::configure(KDevProjectItem *dom)
{
    Q_UNUSED(dom);
    return false;
}

bool KDevMakeBuilder::build(KDevProjectItem *dom)
{
    if (KDevMakeFrontend *make = project()->extension<KDevMakeFrontend>("KDevelop/MakeFrontend")) {
        if (KDevProjectFolderItem *folder = dom->folder()) {
            // ### compile the folder
            QString command = buildCommand(dom);
            make->queueCommand(folder->name(), command);
            m_commands.append(qMakePair(command, dom));
            return true;
        } else if (KDevProjectTargetItem *target = dom->target()) {
            // ### compile the target
            Q_UNUSED(target);
        } else if (KDevProjectFileItem *file = dom->file()) {
            // ### compile the file
            Q_UNUSED(file);
        }
    }

    return false;
}

bool KDevMakeBuilder::clean(KDevProjectItem *dom)
{
    Q_UNUSED(dom);
    return false;
}

bool KDevMakeBuilder::execute(KDevProjectItem *dom)
{
    Q_UNUSED(dom);
    return false;
}

void KDevMakeBuilder::projectConfigWidget(KDialogBase *dlg)
{
    Q_ASSERT(project());

#warning "port me"

#if 0
    Q3VBox *vbox = dlg->addVBoxPage(i18n("Make Options"), i18n("Make Options"), BarIcon( "make", KIcon::SizeMedium ));
    MakeOptionsWidget *widget = new MakeOptionsWidget(*project()->projectDom(), builder, vbox);
    connect(dlg, SIGNAL(okClicked()), widget, SLOT(accept()));
#endif
}

void KDevMakeBuilder::commandFinished(const QString &command)
{
    if (!m_commands.isEmpty()) {
        QPair<QString, KDevProjectItem *> item = m_commands.first();
        if (item.first == command) {
            m_commands.pop_front();
            emit builded(item.second);
        }
    }
}

void KDevMakeBuilder::commandFailed(const QString &command)
{
    Q_UNUSED(command);

    if (!m_commands.isEmpty()) {
        m_commands.clear();

        emit failed();
    }
}

QString KDevMakeBuilder::buildCommand(KDevProjectItem *item, const QString &target)
{
    QDomDocument &dom = *project()->projectDom();

    QString cmdline = DomUtil::readEntry(dom, makeTool);
    int prio = DomUtil::readIntEntry(dom, priority);
    QString nice;
    if (prio != 0) {
        nice = QString("nice -n%1 ").arg(prio);
    }

    if (cmdline.isEmpty())
        cmdline = MAKE_COMMAND;
    if (!DomUtil::readBoolEntry(dom, abortOnError))
        cmdline += " -k";
    int jobs = DomUtil::readIntEntry(dom, numberOfJobs);
    if (jobs != 0) {
        cmdline += " -j";
        cmdline += QString::number(jobs);
    }
    if (DomUtil::readBoolEntry(dom, dontAct))
        cmdline += " -n";

    cmdline += " ";
    cmdline += target;

    cmdline.prepend(nice);
    cmdline.prepend(makeEnvironment());

    Q_ASSERT(item->folder());

    QString dircmd = "cd ";
    QString dir = item->folder()->name();
    dircmd += KProcess::quote(dir);
    dircmd += " && ";

    return dircmd + cmdline;
}

QString KDevMakeBuilder::makeEnvironment() const
{
    DomUtil::PairList envvars =
        DomUtil::readPairListEntry(*project()->projectDom(), environment, "envvar", "name", "value");

    QString env;
    DomUtil::PairList::ConstIterator it;
    for (it = envvars.begin(); it != envvars.end(); ++it) {
        env += (*it).first;
        env += "=";
        env += KProcess::quote((*it).second);
        env += " ";
    }
    return env;
}

