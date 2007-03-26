#include "subversion_part.h"
#include "subversion_view.h"
#include "subversion_fileinfo.h"

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
//     d->m_infoProvider = new SvnFileInfoProvider(this);

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
bool KDevSubversionPart::statusASync( const KUrl &dirPath, KDevelop::IVersionControl::WorkingMode mode,  const QList<KDevelop::VcsFileInfo> &map )
{
    //TODO
}
void KDevSubversionPart::fillContextMenu( const KUrl &ctxUrl, QMenu &ctxMenu )
{
    d->m_ctxUrl = ctxUrl;
    QAction *action = ctxMenu.addAction(i18n("Subversion Log View"));
    connect( action, SIGNAL(triggered()), this, SLOT(ctxLogView()) );
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
//     void spawnCommitThread( KUrl::List &urls, bool recurse, bool keepLocks );
    d->m_impl->spawnCommitThread( wcPaths, true, false );
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

const KUrl& KDevSubversionPart::urlFocusedDocument()
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
//////////////////////////////////////////////
void KDevSubversionPart::ctxLogView()
{
    KUrl::List list;
    list << d->m_ctxUrl;
    d->m_impl->spawnLogviewThread( list, -1, "HEAD", 0, "", 0, true, true, false );
}
SubversionCore* KDevSubversionPart::svncore()
{
    return d->m_impl;
}

#include "subversion_part.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
