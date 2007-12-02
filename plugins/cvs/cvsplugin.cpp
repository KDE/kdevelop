/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvsplugin.h"

#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <KParts/PartManager>
#include <KParts/Part>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KActionCollection>
#include <KMessageBox>
#include <KUrl>
#include <KAction>


#include <projectmodel.h>
#include <iuicontroller.h>
#include <icore.h>
#include <context.h>

#include "cvsmainview.h"
#include "cvsproxy.h"
#include "cvsjob.h"
#include "diffoptionsdialog.h"
#include "editorsview.h"
#include "logview.h"
#include "commitdialog.h"
#include "updateoptionsdialog.h"
#include "cvsgenericoutputview.h"
#include "importdialog.h"
#include "checkoutdialog.h"


K_PLUGIN_FACTORY(KDevCvsFactory, registerPlugin<CvsPlugin>(); )
K_EXPORT_PLUGIN(KDevCvsFactory("kdevcvs"))

class KDevCvsViewFactory: public KDevelop::IToolViewFactory{
public:
    KDevCvsViewFactory(CvsPlugin *plugin): m_plugin(plugin) {}
    virtual QWidget* create(QWidget *parent = 0)
    {
        return new CvsMainView(m_plugin, parent);
    }
    virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
    {
        return Qt::BottomDockWidgetArea;
    }
private:
    CvsPlugin *m_plugin;
};

class CvsPluginPrivate {
public:
    KDevCvsViewFactory* m_factory;
    QPointer<CvsProxy> m_proxy;
    KUrl::List m_ctxUrlList;
};

CvsPlugin::CvsPlugin( QObject *parent, const QVariantList & )
    : KDevelop::IPlugin(KDevCvsFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBasicVersionControl )

    d = new CvsPluginPrivate();

    d->m_factory = new KDevCvsViewFactory(this);
    core()->uiController()->addToolView(i18n("CVS"), d->m_factory);

    setXMLFile("kdevcvs.rc");
    setupActions();

    d->m_proxy = new CvsProxy(this);
}

CvsPlugin::~CvsPlugin()
{
    delete d;
}


CvsProxy* CvsPlugin::proxy()
{
    return d->m_proxy;
}

void CvsPlugin::setupActions()
{
    KAction *action;

    action = actionCollection()->addAction("cvs_import");
    action->setText(i18n("Import Directory..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotImport()));

    action = actionCollection()->addAction("cvs_checkout");
    action->setText(i18n("Checkout..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotCheckout()));

    action = actionCollection()->addAction("cvs_status");
    action->setText(i18n("Status..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotStatus()));
}

const KUrl CvsPlugin::urlFocusedDocument() const
{
    KParts::ReadOnlyPart *plugin =
            dynamic_cast<KParts::ReadOnlyPart*>( core()->partManager()->activePart() );
    if ( plugin ) {
        if (plugin->url().isLocalFile() ) {
            return plugin->url();
        }
    }
    return KUrl();
}


void CvsPlugin::slotImport()
{
    KUrl url = urlFocusedDocument();

    /// @todo just for testing; remove me...
    // In order to be able the test the ImportDialog and the
    // import job I need to get to a directory somehow.
    // So this is a bit of a hack right now. I take the directory
    // from the currently opened file are directory to import.
    // This whole slot will be removed later when createNewProject()
    // gets called via the IVersionControl interface.
    QFileInfo info(url.toLocalFile());

    ///@todo implement me
}

void CvsPlugin::slotCheckout()
{
    ///@todo don't use proxy directly; use interface instead

    CheckoutDialog dlg(this);

    dlg.exec();
}

void CvsPlugin::slotStatus()
{
    KUrl url = urlFocusedDocument();
    KUrl::List urls;
    urls << url;

    KDevelop::VcsJob* j = status(url, KDevelop::IBasicVersionControl::Recursive);
    CvsJob* job = dynamic_cast<CvsJob*>(j);
    if (job) {
        CvsGenericOutputView* view = new CvsGenericOutputView(this, job);
        emit addNewTabToMainView( view, i18n("Status") );
        job->start();
    }
}



QPair<QString,QList<QAction*> > CvsPlugin::requestContextMenuActions(KDevelop::Context* context)
{
    // First we need to convert the data from the context* into something we can work with
    // This means we have to pull the KUrls out of the given list of items.
    KUrl::List ctxUrlList;

    // For now we only support the contextmenu from the ProjectView plugin.
    // If, in future, further types should be supported, just add an else-if for that type here
    if( context->type() == KDevelop::Context::ProjectItemContext ) {
        KDevelop::ProjectItemContext *itemCtx = dynamic_cast<KDevelop::ProjectItemContext*>(context);
        QList<KDevelop::ProjectBaseItem *> baseItemList = itemCtx->items();

        foreach( KDevelop::ProjectBaseItem* _item, baseItemList ) {
            if( _item->folder() ){
                KDevelop::ProjectFolderItem *folderItem = dynamic_cast<KDevelop::ProjectFolderItem*>(_item);
                ctxUrlList << folderItem->url();
            }
            else if( _item->file() ){
                KDevelop::ProjectFileItem *fileItem = dynamic_cast<KDevelop::ProjectFileItem*>(_item);
                ctxUrlList << fileItem->url();
            }
        }
    } else {
        // Unsupported context type
        return KDevelop::IPlugin::requestContextMenuActions( context );
    }

    if( ctxUrlList.isEmpty() )
        return KDevelop::IPlugin::requestContextMenuActions( context );

    // OK. If we made it until here, the requested context type is supported and at least one item
    // was given. Now let's store the items in d->m_ctxUrlList and create the menu.

    d->m_ctxUrlList = ctxUrlList;

    QList<QAction*> actions;
    KAction *action;

    action = new KAction(i18n("Commit..."), this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxCommit()) );
    actions << action;

    action = new KAction(i18n("Add"), this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxAdd()) );
    actions << action;

    action = new KAction(i18n("Remove"), this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxRemove()) );
    actions << action;

    action = new KAction(i18n("Edit"), this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxEdit()) );
    actions << action;

    action = new KAction(i18n("Unedit"), this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxUnEdit()) );
    actions << action;

    action = new KAction(i18n("Show Editors"), this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxEditors()) );
    actions << action;

    action = new KAction(i18n("Update.."), this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxUpdate()) );
    actions << action;

    action = new KAction(i18n("Log View"), this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxLog()) );
    actions << action;

    action = new KAction(i18n("Annotate"), this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxAnnotate()) );
    actions << action;

    action = new KAction(i18n("Revert"), this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxRevert()) );
    actions << action;

    action = new KAction("Diff...", this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxDiff()) );
    actions << action;

    return qMakePair( QString("CVS"), actions );
}


void CvsPlugin::ctxCommit()
{
    KDevelop::VcsJob* j = commit("", d->m_ctxUrlList, KDevelop::IBasicVersionControl::Recursive);
    CvsJob* job = dynamic_cast<CvsJob*>(j);
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

void CvsPlugin::ctxAdd()
{
    KDevelop::VcsJob* j = add(d->m_ctxUrlList, KDevelop::IBasicVersionControl::Recursive);
    CvsJob* job = dynamic_cast<CvsJob*>(j);
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

void CvsPlugin::ctxRemove()
{
    KDevelop::VcsJob* j = remove(d->m_ctxUrlList);
    CvsJob* job = dynamic_cast<CvsJob*>(j);
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

void CvsPlugin::ctxUpdate()
{
    UpdateOptionsDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
        KDevelop::VcsJob* j = update(d->m_ctxUrlList,
                                     dlg.revision(),
                                     KDevelop::IBasicVersionControl::Recursive);
        CvsJob* job = dynamic_cast<CvsJob*>(j);
        if (job) {
            connect(job, SIGNAL( result(KJob*) ),
                    this, SIGNAL( jobFinished(KJob*) ));
            job->start();
        }
    }
}

void CvsPlugin::ctxLog()
{
    KDevelop::VcsRevision rev;
    KDevelop::VcsJob* j = log( d->m_ctxUrlList.first(), rev, 0 );
    CvsJob* job = dynamic_cast<CvsJob*>(j);
    if (job) {
        job->start();
        LogView* view = new LogView(this, job);
        emit addNewTabToMainView( view, i18n("Log") );
    }
}

void CvsPlugin::ctxAnnotate()
{
    /// @todo let user pick annotate revision
    KDevelop::VcsRevision rev;

    KDevelop::VcsJob* j = annotate(d->m_ctxUrlList.first(), rev);
    CvsJob* job = dynamic_cast<CvsJob*>(j);
    if (job) {
        CvsGenericOutputView* view = new CvsGenericOutputView(this, job);
        emit addNewTabToMainView( view, i18n("Annotate") );
        job->start();
    }
}

void CvsPlugin::ctxRevert()
{
    KDevelop::VcsJob* j = revert(d->m_ctxUrlList,
                                    KDevelop::IBasicVersionControl::Recursive);
    CvsJob* job = dynamic_cast<CvsJob*>(j);
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

void CvsPlugin::ctxDiff()
{
    KUrl url = d->m_ctxUrlList.first();
    DiffOptionsDialog dlg(0, url);

    if (dlg.exec() == QDialog::Accepted) {
        KDevelop::VcsJob* j = diff(url, QString(""), dlg.revA(), dlg.revB(), KDevelop::VcsDiff::DiffUnified);
        CvsJob* job = dynamic_cast<CvsJob*>(j);
        if (job) {
            job->start();
            CvsGenericOutputView* view = new CvsGenericOutputView(this, job);
            emit addNewTabToMainView( view, i18n("Diff") );
        }
    }
}

void CvsPlugin::ctxEdit()
{
    KUrl url = d->m_ctxUrlList.first();

    KDevelop::VcsJob* j = edit(url);
    CvsJob* job = dynamic_cast<CvsJob*>(j);
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

void CvsPlugin::ctxUnEdit()
{
    KUrl url = d->m_ctxUrlList.first();

    KDevelop::VcsJob* j = unedit(url);
    CvsJob* job = dynamic_cast<CvsJob*>(j);
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

void CvsPlugin::ctxEditors()
{
    ///@todo find a common base directory for the files
    QFileInfo info( d->m_ctxUrlList[0].toLocalFile() );

    CvsJob* job = d->m_proxy->editors( info.absolutePath(),
                                       d->m_ctxUrlList);
    if (job) {
        job->start();
        EditorsView* view = new EditorsView(this, job);
        emit addNewTabToMainView( view, i18n("Editors") );
    }
}




// Begin:  KDevelop::IBasicVersionControl

bool CvsPlugin::isVersionControlled(const KUrl & localLocation)
{
    return d->m_proxy->isValidDirectory(localLocation);
}

KDevelop::VcsJob * CvsPlugin::repositoryLocation(const KUrl & localLocation)
{
    return NULL;
}

KDevelop::VcsJob * CvsPlugin::add(const KUrl::List & localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    ///@todo find a common base directory for the files
    QFileInfo info( localLocations[0].toLocalFile() );

    CvsJob* job = d->m_proxy->add( info.absolutePath(),
                                   localLocations,
                                   (recursion == KDevelop::IBasicVersionControl::Recursive) ? true:false);
    return job;
}

KDevelop::VcsJob * CvsPlugin::remove(const KUrl::List & localLocations)
{
    ///@todo find a common base directory for the files
    QFileInfo info( localLocations[0].toLocalFile() );

    CvsJob* job = d->m_proxy->remove( info.absolutePath(),
                                      localLocations);
    return job;
}

KDevelop::VcsJob * CvsPlugin::localRevision(const KUrl & localLocation, KDevelop::VcsRevision::RevisionType )
{
    return NULL;
}

KDevelop::VcsJob * CvsPlugin::status(const KUrl::List & localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    ///@todo find a common base directory for the files
    QFileInfo info( localLocations[0].toLocalFile() );

    CvsJob* job = d->m_proxy->status( info.absolutePath(),
                                      localLocations,
                                      (recursion == KDevelop::IBasicVersionControl::Recursive) ? true:false);
    return job;
}

KDevelop::VcsJob * CvsPlugin::unedit(const KUrl & localLocation)
{
    ///@todo find a common base directory for the files
    QFileInfo info( localLocation.toLocalFile() );

    CvsJob* job = d->m_proxy->unedit( info.absolutePath(),
                                      localLocation);
    return job;
}

KDevelop::VcsJob * CvsPlugin::edit(const KUrl & localLocation)
{
    QFileInfo info( localLocation.toLocalFile() );

    CvsJob* job = d->m_proxy->edit( info.absolutePath(),
                                    localLocation);
    return job;
}

KDevelop::VcsJob * CvsPlugin::copy(const KUrl & localLocationSrc, const KUrl & localLocationDstn)
{
    bool ok = QFile::copy(localLocationSrc.path(), localLocationDstn.path());
    if (!ok) {
        return NULL;
    }

    QFileInfo infoDstn( localLocationDstn.toLocalFile() );
    KUrl::List listDstn;
    listDstn << localLocationDstn;

    CvsJob* job = d->m_proxy->add( infoDstn.absolutePath(),
                                   listDstn, true);

    return job;
}

KDevelop::VcsJob * CvsPlugin::move(const KUrl & localLocationSrc, const KUrl & localLocationDst)
{
    return NULL;
}

KDevelop::VcsJob * CvsPlugin::revert(const KUrl::List & localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    ///@todo find a common base directory for the files
    QFileInfo info( localLocations[0].toLocalFile() );

    KDevelop::VcsRevision rev;
    CvsJob* job = d->m_proxy->update( info.absolutePath(),
                                      localLocations,
                                      rev,
                                      "-C",
                                      (recursion == KDevelop::IBasicVersionControl::Recursive) ? true:false,
                                      false, false);
    return job;
}

KDevelop::VcsJob * CvsPlugin::update(const KUrl::List & localLocations, const KDevelop::VcsRevision & rev, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    ///@todo find a common base directory for the files
    QFileInfo info( localLocations[0].toLocalFile() );

    CvsJob* job = d->m_proxy->update( info.absolutePath(),
                                      localLocations,
                                      rev,
                                      "",
                                      (recursion == KDevelop::IBasicVersionControl::Recursive) ? true:false,
                                      false, false);
    return job;
}

KDevelop::VcsJob * CvsPlugin::commit(const QString & message, const KUrl::List & localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    QString msg = message;
    if( msg.isEmpty() )
    {
        CommitDialog dlg;
        if( dlg.exec() == QDialog::Accepted )
        {
            msg = dlg.message();
        }
    }
    ///@todo find a common base directory for the files
    QFileInfo info( localLocations[0].toLocalFile() );

    CvsJob* job = d->m_proxy->commit( info.absolutePath(),
                                      localLocations,
                                      msg);
    return job;
}

KDevelop::VcsJob * CvsPlugin::diff(const KDevelop::VcsLocation & localOrRepoLocationSrc, const KDevelop::VcsLocation & localOrRepoLocationDst, const KDevelop::VcsRevision & srcRevision, const KDevelop::VcsRevision & dstRevision, KDevelop::VcsDiff::Type, KDevelop::IBasicVersionControl::RecursionMode )
{
    CvsJob* job = d->m_proxy->diff (localOrRepoLocationSrc.localUrl(), srcRevision, dstRevision, "");
    return job;
}

KDevelop::VcsJob * CvsPlugin::log(const KUrl & localLocation, const KDevelop::VcsRevision & rev, unsigned long limit)
{
    Q_UNUSED(limit)

    CvsJob* job = d->m_proxy->log( localLocation, rev );
    return job;
}

KDevelop::VcsJob * CvsPlugin::log(const KUrl & localLocation, const KDevelop::VcsRevision & rev, const KDevelop::VcsRevision & limit)
{
    Q_UNUSED(limit)
    return log(localLocation, rev, 0);
}

KDevelop::VcsJob * CvsPlugin::annotate(const KUrl & localLocation, const KDevelop::VcsRevision & rev)
{
    CvsJob* job = d->m_proxy->annotate( localLocation, rev );
    return job;
}

KDevelop::VcsJob * CvsPlugin::merge(const KDevelop::VcsLocation & localOrRepoLocationSrc, const KDevelop::VcsLocation & localOrRepoLocationDst, const KDevelop::VcsRevision & srcRevision, const KDevelop::VcsRevision & dstRevision, const KUrl & localLocation)
{
    return NULL;
}

KDevelop::VcsJob * CvsPlugin::resolve(const KUrl::List & localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    return NULL;
}

KDevelop::VcsJob * CvsPlugin::import(const KUrl & localLocation, const QString & repositoryLocation, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    return NULL;
}

KDevelop::VcsJob * CvsPlugin::checkout(const KDevelop::VcsMapping & mapping)
{
    return NULL;
}


QString CvsPlugin::name() const
{
    return i18n("CVS");
}
QWidget* CvsPlugin::importMetadataWidget( QWidget* parent )
{
    return 0;
}

// End:  KDevelop::IBasicVersionControl

#include "cvsplugin.moc"
