/***************************************************************************
 *   Copyright 2007 Rober Gruber <rgruber@users.sourceforge.net>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvspart.h"

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


K_PLUGIN_FACTORY(KDevCvsFactory, registerPlugin<CvsPart>(); )
K_EXPORT_PLUGIN(KDevCvsFactory("kdevcvs"))

class KDevCvsViewFactory: public KDevelop::IToolViewFactory{
public:
    KDevCvsViewFactory(CvsPart *part): m_part(part) {}
    virtual QWidget* create(QWidget *parent = 0)
    {
        return new CvsMainView(m_part, parent);
    }
    virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
    {
        return Qt::BottomDockWidgetArea;
    }
private:
    CvsPart *m_part;
};

class CvsPartPrivate {
public:
    KDevCvsViewFactory* m_factory;
    QPointer<CvsProxy> m_proxy;
    KUrl::List m_ctxUrlList;
};

CvsPart::CvsPart( QObject *parent, const QVariantList & )
    : KDevelop::IPlugin(KDevCvsFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBasicVersionControl )

    d = new CvsPartPrivate();

    d->m_factory = new KDevCvsViewFactory(this);
    core()->uiController()->addToolView("CVS", d->m_factory);

    setXMLFile("kdevcvs.rc");
    setupActions();

    d->m_proxy = new CvsProxy(this);
}

CvsPart::~CvsPart()
{
    delete d;
}


CvsProxy* CvsPart::proxy()
{
    return d->m_proxy;
}

void CvsPart::setupActions()
{
    KAction *action;

    action = actionCollection()->addAction("cvs_edit");
    action->setText(i18n("Edit..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotEdit()));

    action = actionCollection()->addAction("cvs_unedit");
    action->setText(i18n("Unedit..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotUnEdit()));

    action = actionCollection()->addAction("cvs_editors");
    action->setText(i18n("Show Editors..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotEditors()));

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

const KUrl CvsPart::urlFocusedDocument() const
{
    KParts::ReadOnlyPart *part =
            dynamic_cast<KParts::ReadOnlyPart*>( core()->partManager()->activePart() );
    if ( part ) {
        if (part->url().isLocalFile() ) {
            return part->url();
        }
    }
    return KUrl();
}


void CvsPart::slotEdit()
{
    KUrl url = urlFocusedDocument();

    KDevelop::VcsJob* j = edit(url);
    CvsJob* job = dynamic_cast<CvsJob*>(j);
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

void CvsPart::slotUnEdit()
{
    KUrl url = urlFocusedDocument();

    KDevelop::VcsJob* j = unedit(url);
    CvsJob* job = dynamic_cast<CvsJob*>(j);
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

void CvsPart::slotEditors()
{
    KUrl url = urlFocusedDocument();
    KUrl::List urls;
    urls << url;

    ///@todo implement me
}

void CvsPart::slotImport()
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

void CvsPart::slotCheckout()
{
    ///@todo don't use proxy directly; use interface instead

    CheckoutDialog dlg(this);

    dlg.exec();
}

void CvsPart::slotStatus()
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



QPair<QString,QList<QAction*> > CvsPart::requestContextMenuActions(KDevelop::Context* context)
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


void CvsPart::ctxCommit()
{
    KDevelop::VcsJob* j = commit("", d->m_ctxUrlList, KDevelop::IBasicVersionControl::Recursive);
    CvsJob* job = dynamic_cast<CvsJob*>(j);
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

void CvsPart::ctxAdd()
{
    KDevelop::VcsJob* j = add(d->m_ctxUrlList, KDevelop::IBasicVersionControl::Recursive);
    CvsJob* job = dynamic_cast<CvsJob*>(j);
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

void CvsPart::ctxRemove()
{
    KDevelop::VcsJob* j = remove(d->m_ctxUrlList);
    CvsJob* job = dynamic_cast<CvsJob*>(j);
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

void CvsPart::ctxUpdate()
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

void CvsPart::ctxLog()
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

void CvsPart::ctxAnnotate()
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

void CvsPart::ctxRevert()
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

void CvsPart::ctxDiff()
{
    KUrl url = d->m_ctxUrlList.first();
    DiffOptionsDialog dlg(0, url);

    if (dlg.exec() == QDialog::Accepted) {
        KDevelop::VcsRevision revA;
        revA.setRevisionValue(dlg.revA(), KDevelop::VcsRevision::FileNumber);

        KDevelop::VcsRevision revB;
        revB.setRevisionValue(dlg.revB(), KDevelop::VcsRevision::FileNumber);

        KDevelop::VcsJob* j = diff(url, QString(""), revA, revB, KDevelop::VcsDiff::DiffUnified);
        CvsJob* job = dynamic_cast<CvsJob*>(j);
        if (job) {
            job->start();
            CvsGenericOutputView* view = new CvsGenericOutputView(this, job);
            emit addNewTabToMainView( view, i18n("Diff") );
        }
    }
}



// Begin:  KDevelop::IBasicVersionControl

bool CvsPart::isVersionControlled(const KUrl & localLocation)
{
    return d->m_proxy->isValidDirectory(localLocation);
}

KDevelop::VcsJob * CvsPart::repositoryLocation(const KUrl & localLocation)
{
    return NULL;
}

KDevelop::VcsJob * CvsPart::add(const KUrl::List & localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    ///@todo find a common base directory for the files
    QFileInfo info( localLocations[0].toLocalFile() );

    CvsJob* job = d->m_proxy->add( info.absolutePath(),
                                   localLocations,
                                   (recursion == KDevelop::IBasicVersionControl::Recursive) ? true:false);
    return job;
}

KDevelop::VcsJob * CvsPart::remove(const KUrl::List & localLocations)
{
    ///@todo find a common base directory for the files
    QFileInfo info( localLocations[0].toLocalFile() );

    CvsJob* job = d->m_proxy->remove( info.absolutePath(),
                                      localLocations);
    return job;
}

KDevelop::VcsJob * CvsPart::localRevision(const KUrl & localLocation, KDevelop::VcsRevision::RevisionType )
{
    return NULL;
}

KDevelop::VcsJob * CvsPart::status(const KUrl::List & localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    ///@todo find a common base directory for the files
    QFileInfo info( localLocations[0].toLocalFile() );

    CvsJob* job = d->m_proxy->status( info.absolutePath(),
                                      localLocations,
                                      (recursion == KDevelop::IBasicVersionControl::Recursive) ? true:false);
    return job;
}

KDevelop::VcsJob * CvsPart::unedit(const KUrl & localLocation)
{
    ///@todo find a common base directory for the files
    QFileInfo info( localLocation.toLocalFile() );

    CvsJob* job = d->m_proxy->unedit( info.absolutePath(),
                                      localLocation);
    return job;
}

KDevelop::VcsJob * CvsPart::edit(const KUrl & localLocation)
{
    QFileInfo info( localLocation.toLocalFile() );

    CvsJob* job = d->m_proxy->edit( info.absolutePath(),
                                    localLocation);
    return job;
}

KDevelop::VcsJob * CvsPart::copy(const KUrl & localLocationSrc, const KUrl & localLocationDstn)
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

KDevelop::VcsJob * CvsPart::move(const KUrl & localLocationSrc, const KUrl & localLocationDst)
{
    return NULL;
}

KDevelop::VcsJob * CvsPart::revert(const KUrl::List & localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion)
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

KDevelop::VcsJob * CvsPart::update(const KUrl::List & localLocations, const KDevelop::VcsRevision & rev, KDevelop::IBasicVersionControl::RecursionMode recursion)
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

KDevelop::VcsJob * CvsPart::commit(const QString & message, const KUrl::List & localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion)
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

KDevelop::VcsJob * CvsPart::showDiff(const KDevelop::VcsLocation& localOrRepoLocationSrc, const KDevelop::VcsLocation & localOrRepoLocationDst, const KDevelop::VcsRevision & srcRevision, const KDevelop::VcsRevision & dstRevision, KDevelop::VcsDiff::Type, KDevelop::IBasicVersionControl::RecursionMode  )
{
    return NULL;
}

KDevelop::VcsJob * CvsPart::diff(const KDevelop::VcsLocation & localOrRepoLocationSrc, const KDevelop::VcsLocation & localOrRepoLocationDst, const KDevelop::VcsRevision & srcRevision, const KDevelop::VcsRevision & dstRevision, KDevelop::VcsDiff::Type, KDevelop::IBasicVersionControl::RecursionMode )
{
    CvsJob* job = d->m_proxy->diff (localOrRepoLocationSrc.localUrl(), srcRevision, dstRevision, "");
    return job;
}

KDevelop::VcsJob * CvsPart::log(const KUrl & localLocation, const KDevelop::VcsRevision & rev, unsigned long limit)
{
    Q_UNUSED(limit)

    CvsJob* job = d->m_proxy->log( localLocation, rev );
    return job;
}

KDevelop::VcsJob * CvsPart::log(const KUrl & localLocation, const KDevelop::VcsRevision & rev, const KDevelop::VcsRevision & limit)
{
    Q_UNUSED(limit)
    return log(localLocation, rev, 0);
}

KDevelop::VcsJob * CvsPart::showLog(const KUrl & localLocation, const KDevelop::VcsRevision & rev)
{
    return NULL;
}

KDevelop::VcsJob * CvsPart::annotate(const KUrl & localLocation, const KDevelop::VcsRevision & rev)
{
    CvsJob* job = d->m_proxy->annotate( localLocation, rev );
    return job;
}

KDevelop::VcsJob * CvsPart::showAnnotate(const KUrl & localLocation, const KDevelop::VcsRevision & rev)
{
    return NULL;
}

KDevelop::VcsJob * CvsPart::merge(const KDevelop::VcsLocation & localOrRepoLocationSrc, const KDevelop::VcsLocation & localOrRepoLocationDst, const KDevelop::VcsRevision & srcRevision, const KDevelop::VcsRevision & dstRevision, const KUrl & localLocation)
{
    return NULL;
}

KDevelop::VcsJob * CvsPart::resolve(const KUrl::List & localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    return NULL;
}

KDevelop::VcsJob * CvsPart::import(const KUrl & localLocation, const QString & repositoryLocation, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    return NULL;
}

KDevelop::VcsJob * CvsPart::checkout(const KDevelop::VcsMapping & mapping)
{
    return NULL;
}

// End:  KDevelop::IBasicVersionControl

#include "cvspart.moc"
