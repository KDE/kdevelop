#include "projectmodel.h"
#include "subversion_part.h"
#include "subversion_view.h"
#include "subversion_fileinfo.h"
#include "svn_models.h"
#include "svn_commitwidgets.h"
#include <svn_wc.h>

#include <iuicontroller.h>
#include <icore.h>

#include <kmessagebox.h>
#include <kparts/part.h>
#include <kparts/partmanager.h>
#include <kgenericfactory.h>
#include <kmenu.h>
#include <kurl.h>
#include <kactioncollection.h>
#include <qwidget.h>

#include <QPointer>
#include <QDir>
#include <QtDesigner/QExtensionFactory>

// KDEV_ADD_EXTENSION_FACTORY_NS(KDevelop, IVersionControl, KDevSubversionPart)

typedef KGenericFactory<KDevSubversionPart> KDevSubversionFactory;
K_EXPORT_COMPONENT_FACTORY( kdevsubversion,
                            KDevSubversionFactory( "kdevsubversion" )  )

class KDevSubversionViewFactory: public KDevelop::IToolViewFactory{
public:
    KDevSubversionViewFactory(KDevSubversionPart *part): m_part(part) {}
    virtual QWidget* create(QWidget *parent = 0)
    {
        return new KDevSubversionView(m_part, parent);
    }
    virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
    {
        return Qt::BottomDockWidgetArea;
    }
private:
    KDevSubversionPart *m_part;
};

class KDevSubversionPartPrivate{
public:
    KDevSubversionViewFactory *m_factory;
    QPointer<SubversionCore> m_impl;
    KUrl m_ctxUrl;
//     QPointer<SvnFileInfoProvider> m_infoProvider;
//     SvnFileInfoProvider *m_infoProvider;
    QList<KDevelop::VcsFileInfo> m_vcsInfoList;
    QMap< SvnStatusJob*, QList<KDevelop::VcsFileInfo> > m_fileInfoMap;
    QList<KDevelop::VcsFileInfo> asyncStatusList;

};

/** Design: One part can have many Views. One View has its own core.
 *  Therefore, one can do multiples of jobs simultaneously.
 *  For example, one can see blame in one view while retreiving logs at other view.
 */
KDevSubversionPart::KDevSubversionPart( QObject *parent, const QStringList & )
    : KDevelop::IPlugin(KDevSubversionFactory::componentData(), parent)
    , d(new KDevSubversionPartPrivate)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IVersionControl )

    d->m_factory = new KDevSubversionViewFactory(this);
    core()->uiController()->addToolView("Subversion", d->m_factory);
    // init svn core
    d->m_impl = new SubversionCore(this, this);
    // for now, used to signal statusReady(QList<VcsFileInfo>&)
    connect( d->m_impl, SIGNAL(jobFinished( SubversionJob* )), this, SLOT(slotJobFinished( SubversionJob * )) );

    setXMLFile("kdevsubversion.rc");

    QAction *action;

    action = actionCollection()->addAction("svn_checkout");
    action->setText(i18n("Checkout from repository..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(checkout()));

    action = actionCollection()->addAction("svn_add");
    action->setText(i18n("Add to version control..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(add()));

    action = actionCollection()->addAction("svn_remove");
    action->setText(i18n("Remove from repository..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(remove()));

    action = actionCollection()->addAction("svn_commit");
    action->setText(i18n("Commit to repository..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(commit()));

    action = actionCollection()->addAction("svn_update");
    action->setText(i18n("Update to..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(update()));

    action = actionCollection()->addAction("svn_log");
    action->setText(i18n("Show Subversion Log..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(logView()));
    action->setToolTip( i18n("Show subversion log history") );
    action->setWhatsThis( i18n("<b>Show subversion log</b><p>"
            "View log in KDevelop. "
            "To use subversion integration, install kioslave-svn"
            "package which is under kdesdk") );

    action = actionCollection()->addAction("svn_blame");
    action->setText(i18n("Show Blame (annotate)..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(blame()));

    action = actionCollection()->addAction("svn_status_sync");
    action->setText(i18n("Show status with blocking mode"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(statusSync()));

    action = actionCollection()->addAction("svn_status_async");
    action->setText(i18n("Show status with non-blocking mode"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(statusASync()));

    action = actionCollection()->addAction("svn_info");
    action->setText(i18n("Show the system-generated metadatas"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(svnInfo()));

    // init context menu
//     connect( ((UiController*)(core()->uiController()))->defaultMainWindow(), SIGNAL(contextMenu(KMenu *, const Context *)),
//             this, SLOT(contextMenu(KMenu *, const Context *)));
//     connect( Core::mainWindow(), SIGNAL(contextMenu(KMenu *, const Context *)),
//             this, SLOT(contextMenu(KMenu *, const Context *)));

}

KDevSubversionPart::~KDevSubversionPart()
{
    delete d->m_impl;
//     delete d->m_infoProvider;
    delete d;
}
bool KDevSubversionPart::isValidDirectory(const KUrl &dirPath) const
{
    QString svn = "/.svn/";
    QDir svndir( dirPath.path() + svn );
    QString entriesFileName = dirPath.path() + svn + "entries";

    kDebug() << "dirpath " << dirPath.path() +"/.svn/" << " exists:" << svndir.exists() << endl;
    kDebug() << "entries " << entriesFileName << " exists:" << QFile::exists( entriesFileName ) << endl;
    return svndir.exists() &&
            QFile::exists( entriesFileName );
}
const QList<KDevelop::VcsFileInfo>& KDevSubversionPart::statusSync( const KUrl &dirPath,
                                                    KDevelop::IVersionControl::WorkingMode mode )
{
    d->m_vcsInfoList.clear();
    
    SvnStatusSyncJob *job = new SvnStatusSyncJob( SVN_STATUS );
    bool recurse = (mode == KDevelop::IVersionControl::Recursive) ? true : false;
    QList<SvnStatusHolder> &holder = job->statusExec( dirPath, -1, "WORKING", recurse, true, false, true, false );

    for( QList<SvnStatusHolder>::iterator it = holder.begin(); it != holder.end(); ++it ){
        // get revision. 
        QString rev = QString::number( (*it).baseRevision );
        // get status
        KDevelop::VcsFileInfo::VcsFileState state = KDevelop::VcsFileInfo::Unknown;
        
        SvnStatusHolder hold = (*it); // debug
        kDebug() << hold.wcPath << " textStat " << hold.textStatus << " propStat " << hold.propStatus << endl;
        kDebug() << hold.wcPath << " reposTextStat " << hold.reposTextStat<< " reposPropStat " << hold.reposPropStat<< endl;
                
        // get status -- working copy 
        if( (*it).textStatus == svn_wc_status_normal
            && ((*it).propStatus == svn_wc_status_normal||(*it).propStatus == svn_wc_status_none ) )
            // text status always exist if under VC. but in most cases, property value may not exist
            state = KDevelop::VcsFileInfo::Uptodate;
        else if( (*it).textStatus == svn_wc_status_added || (*it).propStatus == svn_wc_status_added )
            state = KDevelop::VcsFileInfo::Added;
        else if( (*it).textStatus == svn_wc_status_modified || (*it).propStatus == svn_wc_status_modified )
            state = KDevelop::VcsFileInfo::Modified;
        else if( (*it).textStatus == svn_wc_status_conflicted || (*it).propStatus == svn_wc_status_conflicted )
            state = KDevelop::VcsFileInfo::Conflict;
        else if( (*it).textStatus == svn_wc_status_deleted || (*it).propStatus == svn_wc_status_deleted )
            state = KDevelop::VcsFileInfo::Deleted;
        else if( (*it).textStatus == svn_wc_status_replaced || (*it).propStatus == svn_wc_status_replaced )
            state = KDevelop::VcsFileInfo::Replaced;
        
        KDevelop::VcsFileInfo vcsInfo( (*it).wcPath, rev, rev, state );
        d->m_vcsInfoList.append( vcsInfo );
    }
    delete job;
    return d->m_vcsInfoList;
}
bool KDevSubversionPart::statusASync( const KUrl &dirPath,
                                      KDevelop::IVersionControl::WorkingMode mode,
                                      const QList<KDevelop::VcsFileInfo> &infos )
{
    bool recurse = (mode == KDevelop::IVersionControl::Recursive) ? true : false;
    const SvnStatusJob *job;
    job = d->m_impl->spawnStatusThread( dirPath, -1, "HEAD", recurse, true, true, true, false );
    d->m_fileInfoMap.insert( (SvnStatusJob*)job, infos );
    return true;
}
void KDevSubversionPart::fillContextMenu( const KUrl &ctxUrl, QMenu &ctxMenu )
{
    d->m_ctxUrl = ctxUrl;
    QMenu *subMenu = new QMenu( "Subversion", (QWidget*)&ctxMenu );
    
    QAction *action;
    action = subMenu->addAction(i18n("Log View"));
    connect( action, SIGNAL(triggered()), this, SLOT(ctxLogView()) );
    action = subMenu->addAction(i18n("Blame (Annotate)"));
    connect( action, SIGNAL(triggered()), this, SLOT(ctxBlame()) );
    action = subMenu->addAction(i18n("Update"));
    connect( action, SIGNAL(triggered()), this, SLOT(ctxUpdate()) );
    action = subMenu->addAction(i18n("Commit..."));
    connect( action, SIGNAL(triggered()), this, SLOT(ctxCommit()) );
    action = subMenu->addAction(i18n("Add to version control"));
    connect( action, SIGNAL(triggered()), this, SLOT(ctxAdd()) );
    action = subMenu->addAction(i18n("Remove from version control"));
    connect( action, SIGNAL(triggered()), this, SLOT(ctxRemove()) );


    ctxMenu.addMenu( subMenu );
}

void KDevSubversionPart::fillContextMenu( const KDevelop::ProjectBaseItem *prjItem,
                                          QMenu &ctxMenu )
{

    if ( KDevelop::ProjectFolderItem *folder = prjItem->folder() ){
        if( !isValidDirectory( folder->url() ) )
            return;
        
        this->fillContextMenu( folder->url(), ctxMenu );
    }
    else if ( KDevelop::ProjectFileItem *file = prjItem->file() ){
        this->fillContextMenu( file->url(), ctxMenu );
    }

}
//////////////////////////////////////////////
void KDevSubversionPart::checkout( const KUrl &repository, const KUrl &targetDir, KDevelop::IVersionControl::WorkingMode mode )
{
}

void KDevSubversionPart::add( const KUrl::List &wcPaths )
{
//     void spawnAddThread( KUrl::List &wcPaths, bool recurse, bool force, bool noIgnore );
    d->m_impl->spawnAddThread( wcPaths, true, true, true );
}
void KDevSubversionPart::remove( const KUrl::List &urls )
{//void spawnRemoveThread( KUrl::List &urls, bool force );
    d->m_impl->spawnRemoveThread( urls, true );
}
void KDevSubversionPart::commit( const KUrl::List &wcPaths )
{
    SvnCommitOptionDlg dlg( this, NULL );
    dlg.setCommitCandidates( wcPaths );
    if( dlg.exec() != QDialog::Accepted )
        return;
    
    KUrl::List checkedList = dlg.checkedUrls();
    if( checkedList.count() < 1 )
        return;
    bool recurse = dlg.recursive();
    bool keeplocks = dlg.keepLocks();
    //debug
    for( QList<KUrl>::iterator it = checkedList.begin(); it!=checkedList.end() ; ++it ){
        kDebug() << "KDevSubversionPart::commit(KUrl::List&) : " << *it << endl;
    }
    
    d->m_impl->spawnCommitThread( checkedList, recurse, keeplocks );
}
void KDevSubversionPart::update( const KUrl::List &wcPaths )
{
    // paths, rev, revKind, recurse, ignoreExternals
    d->m_impl->spawnUpdateThread( wcPaths, -1, "HEAD", true, true );
}
void KDevSubversionPart::logview( const KUrl &wcPath_or_url )
{
    KUrl::List list;
    list << wcPath_or_url;
    d->m_impl->spawnLogviewThread(list, -1, "HEAD", 0, "", 0, true, true, false );
}
void KDevSubversionPart::annotate( const KUrl &path_or_url )
{
    d->m_impl->spawnBlameThread(path_or_url, true,  0, "", -1, "HEAD" );
}
void KDevSubversionPart::vcsInfo( const KUrl &path_or_url ) // not yet in interface
{
    SubversionUtils::SvnRevision peg;
    SubversionUtils::SvnRevision revision;
    peg.revKind = "UNSPECIFIED"; peg.revNum = -1;
    revision.revKind = "UNSPECIFIED"; revision.revNum = -1;
    
    
    d->m_impl->spawnInfoThread( path_or_url, peg, revision, false );
}
SubversionCore* KDevSubversionPart::svncore()
{
    return d->m_impl;
}
const KUrl KDevSubversionPart::urlFocusedDocument()
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
////////////////////////////////////////////
void KDevSubversionPart::checkout()
{
}
void KDevSubversionPart::add()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        KUrl::List list;
        list << activeUrl;
        add( list );
    } else {
        KMessageBox::error(NULL, i18n("No active docuement to add") );
    }

}
void KDevSubversionPart::remove()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        KUrl::List list;
        list << activeUrl;
        remove( list );
    } else {
        KMessageBox::error(NULL, i18n("No active docuement to remove") );
    }
}
void KDevSubversionPart::commit()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        KUrl::List list;
        list << activeUrl;
        commit( list );
    } else {
        KMessageBox::error(NULL, i18n("No active docuement to commit") );
    }
}
void KDevSubversionPart::update()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        KUrl::List list;
        list << activeUrl;
        update( list );
    } else {
        KMessageBox::error(NULL, i18n("No active docuement to update") );
    }
}
void KDevSubversionPart::logView()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        logview( activeUrl );
    } else{
        KMessageBox::error(NULL, "No active docuement to view log" );
    }
}
void KDevSubversionPart::blame()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        annotate( activeUrl );
    } else{
        KMessageBox::error(NULL, "No active docuement to view blame" );
    }
}
void KDevSubversionPart::statusSync()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        const QList<KDevelop::VcsFileInfo> &vcsList = statusSync( activeUrl, KDevelop::IVersionControl::Recursive );
        for( QList<KDevelop::VcsFileInfo>::const_iterator it = vcsList.constBegin(); it != vcsList.constEnd(); ++it ){
            // TODO print to GUI
            kDebug() << (*it).toString() << endl;
        }
    } else{
        KMessageBox::error(NULL, "No active docuement to view status" );
    }
}
void KDevSubversionPart::statusASync()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        statusASync( activeUrl, KDevelop::IVersionControl::Recursive, d->asyncStatusList );
    } else{
        KMessageBox::error(NULL, "No active docuement to view status" );
    }
}
void KDevSubversionPart::svnInfo()
{
    KUrl activeUrl = urlFocusedDocument();
    if( activeUrl.isValid() ){
        vcsInfo( activeUrl );
    } else{
        KMessageBox::error(NULL, "No active docuement to view information" );
    }
}
//////////////////////////////////////////////
void KDevSubversionPart::ctxLogView()
{
    logview( d->m_ctxUrl );
}
void KDevSubversionPart::ctxBlame()
{
    annotate( d->m_ctxUrl );
}
void KDevSubversionPart::ctxCommit()
{
    KUrl::List list;
    list << d->m_ctxUrl;
    commit( list );
}
void KDevSubversionPart::ctxUpdate()
{
    KUrl::List list;
    list << d->m_ctxUrl;
    update( list );
}
void KDevSubversionPart::ctxAdd()
{
    KUrl::List list;
    list << d->m_ctxUrl;
    add( list );
}
void KDevSubversionPart::ctxRemove()
{
    KUrl::List list;
    list << d->m_ctxUrl;
    remove( list );
}

//////////////////////////////////////////////
void KDevSubversionPart::slotJobFinished( SubversionJob *job )
{
    switch( job->type() ){
        case SVN_STATUS:{
            if( !job->wasSuccessful() ){
                KMessageBox::error(NULL, job->errorMsg());
                break;
            }
            SvnStatusJob *statusJob = (SvnStatusJob*)job;
            QList<KDevelop::VcsFileInfo> infos = d->m_fileInfoMap.value( statusJob );

            for( QList<SvnStatusHolder>::iterator it = statusJob->m_holderList.begin() ;
                 it != statusJob->m_holderList.end() ;
                 ++it ) {
                SvnStatusHolder hold = (*it); // debug
                kDebug() << hold.wcPath << " textStat " << hold.textStatus << " propStat " << hold.propStatus << endl;
                kDebug() << hold.wcPath << " reposTextStat " << hold.reposTextStat<< " reposPropStat " << hold.reposPropStat<< endl;
                // get revision. 
                QString rev = QString::number( (*it).baseRevision );
                
                KDevelop::VcsFileInfo::VcsFileState state = KDevelop::VcsFileInfo::Unknown;
                // get status -- working copy first
                if( (*it).textStatus == svn_wc_status_normal
                    && ((*it).propStatus == svn_wc_status_normal||(*it).propStatus == svn_wc_status_none ) )
                    // text status always exist if under VC. but in most cases, property value may not exist
                    state = KDevelop::VcsFileInfo::Uptodate;
                else if( (*it).textStatus == svn_wc_status_added || (*it).propStatus == svn_wc_status_added )
                    state = KDevelop::VcsFileInfo::Added;
                else if( (*it).textStatus == svn_wc_status_modified || (*it).propStatus == svn_wc_status_modified )
                    state = KDevelop::VcsFileInfo::Modified;
                else if( (*it).textStatus == svn_wc_status_conflicted || (*it).propStatus == svn_wc_status_conflicted )
                    state = KDevelop::VcsFileInfo::Conflict;
                else if( (*it).textStatus == svn_wc_status_deleted || (*it).propStatus == svn_wc_status_deleted )
                    state = KDevelop::VcsFileInfo::Deleted;
                else if( (*it).textStatus == svn_wc_status_replaced || (*it).propStatus == svn_wc_status_replaced )
                    state = KDevelop::VcsFileInfo::Replaced;
                // get status -- override the previous result if remote repository is modified
                if( (*it).reposTextStat == svn_wc_status_added || (*it).reposTextStat == svn_wc_status_modified
                    || (*it).reposPropStat == svn_wc_status_modified )
                    state = KDevelop::VcsFileInfo::NeedsPatch;
                
                // TODO retrive repository revision. Set workingcopy and reposit revision same for a moment
                KDevelop::VcsFileInfo vcsInfo( (*it).wcPath, rev, rev, state );
                infos.append( vcsInfo );
            }
            
            emit statusReady( infos );
            d->m_fileInfoMap.remove( statusJob );
            break;
        } //end of case SVN_STATUS
        case SVN_INFO: {
            if( !job->wasSuccessful() ){
                KMessageBox::error(NULL, job->errorMsg());
                break;
            }
            SvnInfoJob *infojob = dynamic_cast<SvnInfoJob*>( job );
            if( !infojob ) return;
            QList<SvnInfoHolder> list = infojob->m_holderMap.values();
            // TODO print to GUI
            for( QList<SvnInfoHolder>::iterator it = list.begin(); it != list.end(); ++it ){
                SvnInfoHolder holder = (*it);
                kDebug() << " ReqPath " << holder.path << endl;
                kDebug() << " url" << holder.url.pathOrUrl() << endl;
                kDebug() << " Reposit Root " << holder.reposRootUrl.pathOrUrl() << endl;
                kDebug() << " Repos UUID" << holder.reposUuid << endl;
            }
            break;
        } //end of case SVN_INFO
        default:
            break;
    };
}
#include "subversion_part.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
