/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvspart.h"

#include <KParts/PartManager>
#include <KParts/Part>
#include <KGenericFactory>
#include <KActionCollection>
#include <KMessageBox>
#include <QAction>
#include <KUrl>
#include <QFileInfo>
#include <QDir>

#include <iuicontroller.h>
#include <icore.h>

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

typedef KGenericFactory<CvsPart> KDevCvsFactory;
K_EXPORT_COMPONENT_FACTORY( kdevcvs,
                            KDevCvsFactory( "kdevcvs" )  )

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
};

CvsPart::CvsPart( QObject *parent, const QStringList & )
    : KDevelop::IPlugin(KDevCvsFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IVersionControl )

    d = new CvsPartPrivate();

    d->m_factory = new KDevCvsViewFactory(this);
    core()->uiController()->addToolView("CVS", d->m_factory);

    setXMLFile("kdevcvs.rc");
    setupActions();

    d->m_proxy = new CvsProxy(this);
}

CvsPart::~CvsPart()
{
    kDebug()<<  k_funcinfo <<endl;

    delete d;
}


CvsProxy* CvsPart::proxy()
{
    return d->m_proxy;
}

void CvsPart::setupActions()
{
    QAction *action;

    action = actionCollection()->addAction("cvs_log");
    action->setText(i18n("Show log"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotLog()));

    action = actionCollection()->addAction("cvs_diff");
    action->setText(i18n("Show differences..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotDiff()));

    action = actionCollection()->addAction("cvs_edit");
    action->setText(i18n("Edit..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotEdit()));

    action = actionCollection()->addAction("cvs_unedit");
    action->setText(i18n("Unedit..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotUnEdit()));

    action = actionCollection()->addAction("cvs_editors");
    action->setText(i18n("Show Editors..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotEditors()));

    action = actionCollection()->addAction("cvs_add");
    action->setText(i18n("Add Files..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotAdd()));

    action = actionCollection()->addAction("cvs_remove");
    action->setText(i18n("Remove Files..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotRemove()));

    action = actionCollection()->addAction("cvs_commit");
    action->setText(i18n("Commit Files..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotCommit()));

    action = actionCollection()->addAction("cvs_update");
    action->setText(i18n("Update Files..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotUpdate()));

    action = actionCollection()->addAction("cvs_import");
    action->setText(i18n("Import Directory..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotImport()));

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

void CvsPart::slotLog()
{
    KUrl url = urlFocusedDocument();
    logview( url );
}

void CvsPart::slotDiff()
{
    KUrl url = urlFocusedDocument();
    diff( url );
}

void CvsPart::slotEdit()
{
    KUrl url = urlFocusedDocument();
    KUrl::List files;
    files << url;
    edit( files );
}

void CvsPart::slotUnEdit()
{
    KUrl url = urlFocusedDocument();
    KUrl::List files;
    files << url;
    unEdit( files );
}

void CvsPart::slotAdd()
{
    KUrl url = urlFocusedDocument();
    KUrl::List urls;
    urls << url;

    add(urls);
}

void CvsPart::slotRemove()
{
    KUrl url = urlFocusedDocument();
    KUrl::List urls;
    urls << url;

    remove(urls);
}

void CvsPart::slotCommit()
{
    KUrl url = urlFocusedDocument();
    KUrl::List urls;
    urls << url;

    commit(urls);
}

void CvsPart::slotUpdate()
{
    KUrl url = urlFocusedDocument();
    KUrl::List urls;
    urls << url;

    update(urls);
}


void CvsPart::slotEditors()
{
    KUrl url = urlFocusedDocument();
    KUrl::List urls;
    urls << url;

    editors(urls);
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
    QFileInfo info(url.path());

    createNewProject( KUrl(info.absolutePath()) );
}




////////////////////////////////////////////////////////////////////////////////
//  BEGIN of methods derived from IVersionControl
////////////////////////////////////////////////////////////////////////////////

void CvsPart::createNewProject(const KUrl & dir)
{
    ImportDialog dlg(this, dir);
    // the dialog handles the import by itself ...
    dlg.exec();
}

void CvsPart::fillContextMenu(const KDevelop::ProjectBaseItem * prjItem, QMenu & ctxMenu)
{
    Q_UNUSED(prjItem)
    Q_UNUSED(ctxMenu)
    /// @todo implemt me !
}

void CvsPart::fillContextMenu(const KUrl & ctxUrl, QMenu & ctxMenu)
{
    Q_UNUSED(ctxUrl)
    Q_UNUSED(ctxMenu)
    /// @todo implemt me !
}

bool CvsPart::isValidDirectory(const KUrl & dirPath) const
{
    return d->m_proxy->isValidDirectory(dirPath);
}

bool CvsPart::fetchFromRepository()
{
    /// @todo implemt me !
    return false;
}

bool CvsPart::statusASync(const KUrl & dirPath, WorkingMode mode, const QList< KDevelop::VcsFileInfo > & infos)
{
    Q_UNUSED(dirPath)
    Q_UNUSED(mode)
    Q_UNUSED(infos)
    /// @todo implemt me !
    return false;
}

const QList< KDevelop::VcsFileInfo > & CvsPart::statusSync(const KUrl & dirPath, KDevelop::IVersionControl::WorkingMode mode)
{
    Q_UNUSED(dirPath)
    Q_UNUSED(mode)
    /// @todo implemt me !
    static const QList< KDevelop::VcsFileInfo > null;
    return null;
}

void CvsPart::checkout(const KUrl & repository, const KUrl & targetDir, KDevelop::IVersionControl::WorkingMode mode)
{
    Q_UNUSED(repository)
    Q_UNUSED(targetDir)
    Q_UNUSED(mode)
    /// @todo implemt me !
}

void CvsPart::annotate(const KUrl & url)
{
    Q_UNUSED(url)
    /// @todo implemt me !
}

void CvsPart::logview(const KUrl & url)
{
    CvsJob* job = d->m_proxy->log( url );
    if (job) {
        job->start();
        LogView* view = new LogView(this, job);
        emit addNewTabToMainView( view, i18n("Log") );
    }
}

void CvsPart::update(const KUrl::List & urls)
{
    UpdateOptionsDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
        ///@todo find a common base directory for the files
        QFileInfo info( urls[0].path() );

        CvsJob* job = d->m_proxy->update( info.absolutePath(), urls, dlg.options() );
        if (job) {
            connect(job, SIGNAL( result(KJob*) ),
                    this, SIGNAL( jobFinished(KJob*) ));
            job->start();
        }
    }
}

void CvsPart::commit(const KUrl::List & urls)
{
    CommitDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
        ///@todo find a common base directory for the files
        QFileInfo info( urls[0].path() );

        CvsJob* job = d->m_proxy->commit( info.absolutePath(), urls, dlg.message() );
        if (job) {
            connect(job, SIGNAL( result(KJob*) ),
                    this, SIGNAL( jobFinished(KJob*) ));
            job->start();
        }
    }
}

void CvsPart::remove(const KUrl::List & urls)
{
    ///@todo find a common base directory for the files
    QFileInfo info( urls[0].path() );

    CvsJob* job = d->m_proxy->remove( info.absolutePath(), urls );
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

void CvsPart::add(const KUrl::List & urls)
{
    ///@todo find a common base directory for the files
    QFileInfo info( urls[0].path() );

    CvsJob* job = d->m_proxy->add( info.absolutePath(), urls );
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

////////////////////////////////////////////////////////////////////////////////
//  END <of methods derived from IVersionControl
////////////////////////////////////////////////////////////////////////////////



void CvsPart::diff(const KUrl& url)
{
    DiffOptionsDialog dlg(0, url);

    if (dlg.exec() == QDialog::Accepted) {
        CvsJob* job = d->m_proxy->diff( url, "", dlg.revA(), dlg.revB() );
        if (job) {
            CvsGenericOutputView* view = new CvsGenericOutputView(this, job);
            emit addNewTabToMainView( view, i18n("Diff") );
            job->start();
        }
    }
}

void CvsPart::edit(const KUrl::List& urls)
{
    ///@todo find a common base directory for the files
    QFileInfo info( urls[0].path() );

    CvsJob* job = d->m_proxy->edit( info.absolutePath(), urls );
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

void CvsPart::unEdit(const KUrl::List& urls)
{
    if ( KMessageBox::questionYesNo( 0,
                i18n("Do you really want to unedit the selected files?"),
                i18n("Unedit Files")) == KMessageBox::No) {
        return;
    }

    ///@todo find a common base directory for the files
    QFileInfo info( urls[0].path() );

    CvsJob* job = d->m_proxy->unedit( info.absolutePath(), urls );
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SIGNAL( jobFinished(KJob*) ));
        job->start();
    }
}

void CvsPart::editors(const KUrl::List& urls)
{
    ///@todo find a common base directory for the files
    QFileInfo info( urls[0].path() );

    CvsJob* job = d->m_proxy->editors( info.absolutePath(), urls );
    if (job) {
        EditorsView* view = new EditorsView(this, job);
        emit addNewTabToMainView( view, i18n("Editors") );
        job->start();
    }
}


#include "cvspart.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
