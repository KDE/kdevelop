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


#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/context.h>
#include <vcs/vcsmapping.h>

#include<interfaces/contextmenuextension.h>

#include "cvsmainview.h"
#include "cvsproxy.h"
#include "cvsjob.h"
#include "diffoptionsdialog.h"
#include "editorsview.h"
#include "logview.h"
#include "commitdialog.h"
#include "updateoptionsdialog.h"
#include "cvsgenericoutputview.h"
#include "checkoutdialog.h"
#include "importdialog.h"
#include "importmetadatawidget.h"
#include <language/interfaces/editorcontext.h>

K_PLUGIN_FACTORY(KDevCvsFactory, registerPlugin<CvsPlugin>(); )
K_EXPORT_PLUGIN(KDevCvsFactory("kdevcvs"))

class KDevCvsViewFactory: public KDevelop::IToolViewFactory{
public:
    KDevCvsViewFactory(CvsPlugin *plugin): m_plugin(plugin) {}
    virtual QWidget* create(QWidget *parent = 0)
    {
        return new CvsMainView(m_plugin, parent);
    }
    virtual Qt::DockWidgetArea defaultPosition()
    {
        return Qt::BottomDockWidgetArea;
    }
    virtual QString id() const
    {
        return "org.kdevelop.CVSView";
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
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::ICentralizedVersionControl )

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
            dynamic_cast<KParts::ReadOnlyPart*>( core()->partController()->activePart() );
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

    ImportDialog dlg(this, url);
    dlg.exec();
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
        KDevelop::ICore::self()->runController()->registerJob(job);
    }
}



KDevelop::ContextMenuExtension CvsPlugin::contextMenuExtension(KDevelop::Context* context)
{
    KUrl::List ctxUrlList;
    if( context->type() == KDevelop::Context::ProjectItemContext )
    {
        KDevelop::ProjectItemContext *itemCtx = dynamic_cast<KDevelop::ProjectItemContext*>(context);
        if( itemCtx )
        {
            QList<KDevelop::ProjectBaseItem *> baseItemList = itemCtx->items();

            // now general case
            foreach( KDevelop::ProjectBaseItem* _item, baseItemList )
            {
                if( _item->folder() ){
                    KDevelop::ProjectFolderItem *folderItem = dynamic_cast<KDevelop::ProjectFolderItem*>(_item);
                    ctxUrlList << folderItem->url();
                }
                else if( _item->file() ){
                    KDevelop::ProjectFileItem *fileItem = dynamic_cast<KDevelop::ProjectFileItem*>(_item);
                    ctxUrlList << fileItem->url();
                }
            }
        }
    }else if( context->type() == KDevelop::Context::EditorContext )
    {
        KDevelop::EditorContext *itemCtx = dynamic_cast<KDevelop::EditorContext*>(context);
        ctxUrlList << itemCtx->url();
    }else if( context->type() == KDevelop::Context::FileContext )
    {
        KDevelop::FileContext *itemCtx = dynamic_cast<KDevelop::FileContext*>(context);
        ctxUrlList += itemCtx->urls();
    }


    KDevelop::ContextMenuExtension menuExt;

    bool hasVersionControlledEntries = false;
    foreach( KUrl url, ctxUrlList )
    {
        if( isVersionControlled( url ) )
        {
            hasVersionControlledEntries = true;
            break;
        }
    }
    if( ctxUrlList.isEmpty() )
        return IPlugin::contextMenuExtension( context );


    d->m_ctxUrlList = ctxUrlList;
    QList<QAction*> actions;
    KAction *action;
    kDebug() << "version controlled?" << hasVersionControlledEntries;
    QMenu* menu = new QMenu("CVS");
    if( hasVersionControlledEntries )
    {
        // Just add actions which are not covered by the cvscommon plugin
        action = new KAction(i18n("Edit"), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxEdit()) );
        menu->addAction( action );

        action = new KAction(i18n("Unedit"), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxUnEdit()) );
        menu->addAction( action );

        action = new KAction(i18n("Show Editors"), this);
        connect( action, SIGNAL(triggered()), this, SLOT(ctxEditors()) );
        menu->addAction( action );

        menuExt.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, menu->menuAction() );
    }

    return menuExt;
}

void CvsPlugin::ctxEdit()
{
    KUrl url = d->m_ctxUrlList.first();

    KDevelop::VcsJob* j = edit(url);
    CvsJob* job = dynamic_cast<CvsJob*>(j);
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        KDevelop::ICore::self()->runController()->registerJob(job);
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
        KDevelop::ICore::self()->runController()->registerJob(job);
    }
}

void CvsPlugin::ctxEditors()
{
    ///@todo find a common base directory for the files
    QFileInfo info( d->m_ctxUrlList[0].toLocalFile() );

    CvsJob* job = d->m_proxy->editors( info.absolutePath(),
                                       d->m_ctxUrlList);
    if (job) {
        KDevelop::ICore::self()->runController()->registerJob(job);
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

KDevelop::VcsJob * CvsPlugin::import(const KDevelop::VcsMapping& localLocation, const QString& commitMessage)
{
    QList<KDevelop::VcsLocation> list = localLocation.sourceLocations();
    if (list.size() < 1) {
        return NULL;
    }

    KDevelop::VcsLocation srcLocation = list[0];
    KDevelop::VcsLocation destLocation = localLocation.destinationLocation(list[0]);

    if (srcLocation.type() != KDevelop::VcsLocation::LocalLocation) {
        return NULL;
    }
    if (destLocation.type() != KDevelop::VcsLocation::RepositoryLocation) {
        return NULL;
    }

    kDebug(9500) << "CVS Import requested "
                 << "src:"<<srcLocation.localUrl().path()
                 << "srv:"<<destLocation.repositoryServer()
                 << "module:"<<destLocation.repositoryModule() << endl;

    CvsJob* job = d->m_proxy->import( srcLocation.localUrl(),
				destLocation.repositoryServer(),
				destLocation.repositoryModule(),
				destLocation.userData().toString(),
				destLocation.repositoryTag(),
				commitMessage);
    return job;
}

KDevelop::VcsJob * CvsPlugin::checkout(const KDevelop::VcsMapping & mapping)
{
    QList<KDevelop::VcsLocation> list = mapping.sourceLocations();
    if (list.size() < 1) {
        return NULL;
    }

    KDevelop::VcsLocation srcLocation = list[0];
    KDevelop::VcsLocation destLocation = mapping.destinationLocation(list[0]);

    if (srcLocation.type() != KDevelop::VcsLocation::RepositoryLocation) {
        return NULL;
    }
    if (destLocation.type() != KDevelop::VcsLocation::LocalLocation) {
        return NULL;
    }

    kDebug(9500) << "CVS Checkout requested "
                 << "dest:"<<destLocation.localUrl().path()
                 << "srv:"<<srcLocation.repositoryServer()
                 << "module:"<<srcLocation.repositoryModule()
                 << "branch:"<<srcLocation.repositoryBranch() << endl;

    CvsJob* job = d->m_proxy->checkout(destLocation.localUrl(),
                                       srcLocation.repositoryServer(),
                                       srcLocation.repositoryModule(),
                                       "",
                                       srcLocation.repositoryBranch(),
                                       true, true);
    return job;
}


QString CvsPlugin::name() const
{
    return i18n("CVS");
}
KDevelop::VcsImportMetadataWidget* CvsPlugin::createImportMetadataWidget( QWidget* parent )
{
    return new ImportMetadataWidget(parent);
}

// End:  KDevelop::IBasicVersionControl

#include "cvsplugin.moc"
