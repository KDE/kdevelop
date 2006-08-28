/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>
   Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

//BEGIN Includes
#include "fileselector_part.h"
#include "fileselector_widget.h"
#include "kactionselector.h"
#include "kbookmarkhandler.h"

#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <kdevdocumentcontroller.h>

#include <QLayout>
#include <qtoolbutton.h>
#include <q3hbox.h>
#include <q3vbox.h>
#include <QLabel>
#include <q3strlist.h>
#include <QToolTip>
#include <Q3WhatsThis>
#include <qapplication.h>
#include <q3listbox.h>
#include <qscrollbar.h>
#include <QSpinBox>
#include <q3groupbox.h>
#include <QCheckBox>
#include <QRegExp>
#include <q3dockarea.h>
#include <QTimer>
//Added by qt3to4:
#include <QPixmap>
#include <QFocusEvent>
#include <QEvent>
#include <Q3Frame>
#include <QShowEvent>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QLineEdit>

#include <ktexteditor/document.h>

#include <kmainwindow.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kurlcombobox.h>
#include <kurlcompletion.h>
#include <kprotocolinfo.h>
#include <kconfig.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kio/netaccess.h>
#include <kmenu.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kicon.h>

#include <kdebug.h>
//END Includes

#if defined(KDE_IS_VERSION)
# if KDE_IS_VERSION(3,1,3)
#  ifndef _KDE_3_1_3_
#   define _KDE_3_1_3_
#  endif
# endif
#endif

// from kfiledialog.cpp - avoid qt warning in STDERR (~/.xsessionerrors)
static void silenceQToolBar(QtMsgType, const char *)
{}


//BEGIN Constructor/destructor

KDevFileSelector::KDevFileSelector( FileSelectorPart *part, KDevMainWindow *mainWindow,
                                    KDevDocumentController *partController,
                                    QWidget * parent )
        : QWidget(parent),
        m_part(part),
        mainwin(mainWindow),
        partController(partController)
{
    setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );

    mActionCollection = new KActionCollection( this );

    QVBoxLayout* lo = new QVBoxLayout(this);
    lo->setMargin(0);
    lo->setSpacing(0);

    QtMsgHandler oldHandler = qInstallMsgHandler( silenceQToolBar );

    toolbar = new KToolBar(this);
    lo->addWidget(toolbar);

    qInstallMsgHandler( oldHandler );

    cmbPath = new KUrlComboBox( KUrlComboBox::Directories, true, this );
//     cmbPath->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
    KUrlCompletion* cmpl = new KUrlCompletion(KUrlCompletion::DirCompletion);
    cmbPath->setCompletionObject( cmpl );
    lo->addWidget(cmbPath);

    dir = new KDevDirOperator(m_part, KUrl(), this);
    dir->setView(KFile::/*Simple*/Detail);

    KActionCollection *coll = dir->actionCollection();
    // some shortcuts of diroperator that clashes with KDev
    coll->action( "delete" )->setShortcut( KShortcut( Qt::ALT + Qt::Key_Delete ) );
    coll->action( "reload" )->setShortcut( KShortcut( Qt::ALT + Qt::Key_F5 ) );
    coll->action( "back" )->setShortcut( KShortcut( Qt::ALT + Qt::SHIFT + Qt::Key_Left ) );
    coll->action( "forward" )->setShortcut( KShortcut( Qt::ALT + Qt::SHIFT + Qt::Key_Right ) );
    // some consistency - reset up for dir too
    coll->action( "up" )->setShortcut( KShortcut( Qt::ALT + Qt::SHIFT + Qt::Key_Up ) );
    coll->action( "home" )->setShortcut( KShortcut( Qt::CTRL + Qt::ALT + Qt::Key_Home ) );

    lo->addWidget(dir);
    lo->setStretchFactor(dir, 2);

    // bookmarks action!
    KActionMenu *acmBookmarks = new KActionMenu( KIcon("bookmark"), i18n("Bookmarks"),
                                mActionCollection, "bookmarks" );
    acmBookmarks->setDelayed( false );

    bookmarkHandler = new KBookmarkHandler( this, acmBookmarks->menu() );

    btnFilter = new QToolButton( this );
    btnFilter->setIcon( KIcon("filter" ) );
    btnFilter->setToggleButton( true );
    filter = new KHistoryCombo( true, this);
//     filter->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
    connect( btnFilter, SIGNAL( clicked() ), this, SLOT( btnFilterClick() ) );

    QHBoxLayout* filterLayout = new QHBoxLayout(lo);
    filterLayout->addWidget(btnFilter);
    filterLayout->addWidget(filter);

    connect( filter, SIGNAL( activated(const QString&) ),
             SLOT( slotFilterChange(const QString&) ) );
    connect( filter, SIGNAL( returnPressed(const QString&) ),
             filter, SLOT( addToHistory(const QString&) ) );

    // kaction for the dir sync method
    acSyncDir = new KAction( KIcon("dirsynch"), i18n("Current Document Directory"), mActionCollection, "sync_dir" );
    connect(acSyncDir, SIGNAL(triggered(bool)), SLOT( setActiveDocumentDir() ));
    toolbar->setToolButtonStyle( Qt::ToolButtonIconOnly );
    toolbar->setIconSize( QSize(16, 16) );
    toolbar->setContextMenuEnabled( false );

    connect( cmbPath, SIGNAL( urlActivated( const KUrl&  )),
             this,  SLOT( cmbPathActivated( const KUrl& ) ));
    connect( cmbPath, SIGNAL( returnPressed( const QString&  )),
             this,  SLOT( cmbPathReturnPressed( const QString& ) ));
    connect(dir, SIGNAL(urlEntered(const KUrl&)),
            this, SLOT(dirUrlEntered(const KUrl&)) );

    connect(dir, SIGNAL(finishedLoading()),
            this, SLOT(dirFinishedLoading()) );

    // enable dir sync button if current doc has a valid URL
    connect ( partController, SIGNAL(documentActivated(KDevDocument*)),
              this, SLOT(viewChanged()) );

    // Connect the bookmark handler
    connect( bookmarkHandler, SIGNAL( openURL( const QString& )),
             this, SLOT( setDir( const QString& ) ) );

    waitingUrl = QString::null;

    // whatsthis help
    Q3WhatsThis::add
        ( cmbPath,
                i18n("<p>Here you can enter a path for a directory to display."
                     "<p>To go to a directory previously entered, press the arrow on "
                     "the right and choose one. <p>The entry has directory "
                     "completion. Right-click to choose how completion should behave.") );
    Q3WhatsThis::add
        ( filter,
                i18n("<p>Here you can enter a name filter to limit which files are displayed."
                     "<p>To clear the filter, toggle off the filter button to the left."
                     "<p>To reapply the last filter used, toggle on the filter button." ) );
    Q3WhatsThis::add
        ( btnFilter,
                i18n("<p>This button clears the name filter when toggled off, or "
                     "reapplies the last filter used when toggled on.") );
}

KDevFileSelector::~KDevFileSelector()
{
	writeConfig( m_part->instance()->config(), "fileselector" );
}
//END Constroctor/Destrctor

//BEGIN Public Methods

void KDevFileSelector::readConfig(KConfig *config, const QString & name)
{
    KConfigGroup cg(config, name + ":dir");
    dir->readConfig(&cg);
    dir->setView( KFile::Default );

    config->setGroup( name );

    // set up the toolbar
    setupToolbar( config );

    cmbPath->setMaxItems( config->readEntry( "pathcombo history len", 9 ) );
    cmbPath->setUrls( config->readEntry("dir history", QStringList()) );
    // if we restore history
    if ( config->readEntry( "restore location", true ) || qApp->isSessionRestored() )
    {
        QString loc( config->readPathEntry( "location" ) );
        if ( ! loc.isEmpty() )
        {
            waitingDir = loc;
            QTimer::singleShot(0, this, SLOT(initialDirChangeHack()));
        }
    }

    // else is automatic, as cmpPath->setURL is called when a location is entered.

    filter->setMaxCount( config->readEntry( "filter history len", 9 ) );
    filter->setHistoryItems( config->readEntry("filter history", QStringList()), true );
    lastFilter = config->readEntry( "last filter" );
    QString flt("");
    if ( config->readEntry( "restore last filter", true ) || qApp->isSessionRestored() )
        flt = config->readEntry("current filter");
    filter->lineEdit()->setText( flt );
    slotFilterChange( flt );

    autoSyncEvents = config->readEntry( "AutoSyncEvents", 0 );
    // connect events as needed
    /// @todo - solve startup problem: no need to set location for each doc opened!
    if ( autoSyncEvents & DocumentChanged )
        connect( partController, SIGNAL( viewChanged() ), this, SLOT( autoSync() ) );

    if ( autoSyncEvents & DocumentOpened )
        connect( partController, SIGNAL( partAdded(KParts::Part*) ),
                 this, SLOT( autoSync(KParts::Part*) ) );

}

void KDevFileSelector::initialDirChangeHack()
{
    setDir( waitingDir );
}

void KDevFileSelector::setupToolbar( KConfig *config )
{
    toolbar->clear();
    QStringList tbactions = config->readEntry( "toolbar actions", QStringList() );
    if ( tbactions.isEmpty() )
    {
        // resonable collection for default toolbar
        tbactions << "up" << "back" << "forward" << "home" <<
        "short view" << "detailed view" <<
        "bookmarks" << "sync_dir";
    }
    KAction *ac;
    for ( QStringList::Iterator it=tbactions.begin(); it != tbactions.end(); ++it )
    {
        if ( *it == "bookmarks" || *it == "sync_dir" )
            ac = mActionCollection->action( *it );
        else
            ac = dir->actionCollection()->action( *it );
        if ( ac )
            toolbar->addAction(ac);
    }
}

void KDevFileSelector::writeConfig(KConfig *config, const QString & name)
{
    {
        KConfigGroup cg(config,name + ":dir");
        dir->writeConfig(&cg);
    }

    KConfigGroup cg(config,name);
    cg.writeEntry( "pathcombo history len", cmbPath->maxItems() );
    QStringList l;
    for (int i = 0; i < cmbPath->count(); i++)
    {
        l.append( cmbPath->itemText( i ) );
    }
    cg.writeEntry( "dir history", l );
    cg.writeEntry( "location", cmbPath->currentText() );

    cg.writeEntry( "filter history len", filter->maxCount() );
    cg.writeEntry( "filter history", filter->historyItems() );
    cg.writeEntry( "current filter", filter->currentText() );
    cg.writeEntry( "last filter", lastFilter );
    cg.writeEntry( "AutoSyncEvents", autoSyncEvents );
}

void KDevFileSelector::setView(KFile::FileView view)
{
    dir->setView(view);
}

//END Public Methods

//BEGIN Public Slots

void KDevFileSelector::slotFilterChange( const QString & nf )
{
    btnFilter->setToolTip( "" ); //remove btnFilter as the tooltip
    QString f = nf.trimmed();
    bool empty = f.isEmpty() || f == "*";
    if ( empty )
    {
        dir->clearFilter();
        filter->lineEdit()->setText( QString::null );
        btnFilter->setToolTip( i18n( "Apply last filter (\"%1\")", lastFilter ) );
    }
    else
    {
        dir->setNameFilter( f );
        lastFilter = f;
        btnFilter->setToolTip( i18n("Clear filter") );
    }
    btnFilter->setOn( !empty );
    dir->updateDir();
    // this will be never true after the filter has been used;)
    btnFilter->setEnabled( !( empty && lastFilter.isEmpty() ) );

}
void KDevFileSelector::setDir( KUrl u )
{
    dir->setUrl(u, true);
}

//END Public Slots

//BEGIN Private Slots

void KDevFileSelector::cmbPathActivated( const KUrl& u )
{
    cmbPathReturnPressed( u.url() );
}

void KDevFileSelector::cmbPathReturnPressed( const QString& u )
{
    QStringList urls = cmbPath->urls();
    urls.removeAll( u );
    urls.prepend( u );
    cmbPath->setUrls( urls, KUrlComboBox::RemoveBottom );
    dir->setFocus();
    dir->setUrl( KUrl(u), true );
}

void KDevFileSelector::dirUrlEntered( const KUrl& u )
{
    cmbPath->setUrl( u );
}

void KDevFileSelector::dirFinishedLoading()
{}


/*
   When the button in the filter box toggles:
   If off:
   If the name filer is anything but "" or "*", reset it.
   If on:
   Set last filter.
*/
void KDevFileSelector::btnFilterClick()
{
    if ( !btnFilter->isOn() )
    {
        slotFilterChange( QString::null );
    }
    else
    {
        filter->lineEdit()->setText( lastFilter );
        slotFilterChange( lastFilter );
    }
}


void KDevFileSelector::autoSync()
{
    kDebug()<<"KDevFileSelector::autoSync()"<<endl;
    // if visible, sync
    if ( isVisible() )
    {
        setActiveDocumentDir();
        waitingUrl = QString::null;
    }
    // else set waiting url
    else
    {
        KUrl u = activeDocumentUrl();
        if (!u.isEmpty())
            waitingUrl = u.directory();
    }
}

void KDevFileSelector::autoSync( KParts::Part *part )
{
    KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( part );
    if( !doc )
	return;

    // as above, but using document url.
    kDebug()<<"KDevFileSelector::autoSync( KTextEditor::Document )"<<endl;
    KUrl u ( doc->url() );
    if ( u.isEmpty() )
    {
        waitingUrl = QString::null;
        return;
    }
    if ( isVisible() )
    {
        setDir( u.directory() );
        waitingUrl = QString::null;
    }
    else
    {
        waitingUrl = u.directory();
    }
}
/// \FIXME crash on shutdown
void KDevFileSelector::setActiveDocumentDir()
{
    //kDebug()<<"KDevFileSelector::setActiveDocumentDir()"<<endl;
    KUrl u = activeDocumentUrl();
    if (!u.isEmpty())
        setDir( u.upUrl() );
}

void KDevFileSelector::viewChanged()
{
    /// @todo make sure the button is disabled if the directory is unreadable, eg
    ///       the document URL has protocol http
    acSyncDir->setEnabled( ! activeDocumentUrl().directory().isEmpty() );
}

//END Private Slots

//BEGIN Protected

void KDevFileSelector::focusInEvent( QFocusEvent * )
{
    dir->setFocus();
}

void KDevFileSelector::showEvent( QShowEvent * )
{
    // sync if we should
    if ( autoSyncEvents & GotVisible )
    {
        kDebug()<<"syncing fs on show"<<endl;
        setActiveDocumentDir();
        waitingUrl = QString::null;
    }
    // else, if we have a waiting URL set it
    else if ( ! waitingUrl.isEmpty() )
    {
        setDir( waitingUrl );
        waitingUrl = QString::null;
    }
}

//END Protected

//BEGIN ACtionLBItem
/*
   QListboxItem that can store and return a string,
   used for the toolbar action selector.
*/
class ActionLBItem : public Q3ListBoxPixmap
{
public:
    ActionLBItem( Q3ListBox *lb=0,
                  const QPixmap &pm = QPixmap(),
                  const QString &text=QString::null,
                  const QString &str=QString::null ) :
            Q3ListBoxPixmap( lb, pm, text ),
            _str(str)
    {}
    ;
    QString idstring()
    {
        return _str;
    };
private:
    QString _str;
};

KUrl KDevFileSelector::activeDocumentUrl( )
{
	return partController->activeDocument()->url();
}
//END ActionLBItem

//BEGIN KFSConfigPage
////////////////////////////////////////////////////////////////////////////////
// KFSConfigPage implementation
////////////////////////////////////////////////////////////////////////////////
KFSConfigPage::KFSConfigPage( QWidget *parent, const char *name, KDevFileSelector *kfs )
        : QWidget( parent, name ),
        fileSelector( kfs ),
        bDirty( false )
{
    QVBoxLayout *lo = new QVBoxLayout( this );
    int spacing = KDialog::spacingHint();
    lo->setSpacing( spacing );

    // Toolbar - a lot for a little...
    Q3GroupBox *gbToolbar = new Q3GroupBox( 1, Qt::Vertical, i18n("Toolbar"), this );
    acSel = new KActionSelector( gbToolbar );
    acSel->setAvailableLabel( i18n("A&vailable actions:") );
    acSel->setSelectedLabel( i18n("S&elected actions:") );
    lo->addWidget( gbToolbar );
    connect( acSel, SIGNAL( added( Q3ListBoxItem * ) ), this, SLOT( slotChanged() ) );
    connect( acSel, SIGNAL( removed( Q3ListBoxItem * ) ), this, SLOT( slotChanged() ) );
    connect( acSel, SIGNAL( movedUp( Q3ListBoxItem * ) ), this, SLOT( slotChanged() ) );
    connect( acSel, SIGNAL( movedDown( Q3ListBoxItem * ) ), this, SLOT( slotChanged() ) );

    // Sync
    Q3GroupBox *gbSync = new Q3GroupBox( 1, Qt::Horizontal, i18n("Auto Synchronization"), this );
    cbSyncActive = new QCheckBox( i18n("When a docu&ment becomes active"), gbSync );
    cbSyncOpen = new QCheckBox( i18n("When a document is o&pened"), gbSync );
    cbSyncShow = new QCheckBox( i18n("When the file selector becomes visible"), gbSync );
    lo->addWidget( gbSync );
    connect( cbSyncActive, SIGNAL( toggled( bool ) ), this, SLOT( slotChanged() ) );
    connect( cbSyncOpen, SIGNAL( toggled( bool ) ), this, SLOT( slotChanged() ) );
    connect( cbSyncShow, SIGNAL( toggled( bool ) ), this, SLOT( slotChanged() ) );

    // Histories
    Q3HBox *hbPathHist = new Q3HBox ( this );
    QLabel *lbPathHist = new QLabel( i18n("Remember &locations:"), hbPathHist );
    sbPathHistLength = new QSpinBox( hbPathHist );
    lbPathHist->setBuddy( sbPathHistLength );
    lo->addWidget( hbPathHist );
    connect( sbPathHistLength, SIGNAL( valueChanged ( int ) ), this, SLOT( slotChanged() ) );

    Q3HBox *hbFilterHist = new Q3HBox ( this );
    QLabel *lbFilterHist = new QLabel( i18n("Remember &filters:"), hbFilterHist );
    sbFilterHistLength = new QSpinBox( hbFilterHist );
    lbFilterHist->setBuddy( sbFilterHistLength );
    lo->addWidget( hbFilterHist );
    connect( sbFilterHistLength, SIGNAL( valueChanged ( int ) ), this, SLOT( slotChanged() ) );

    // Session
    Q3GroupBox *gbSession = new Q3GroupBox( 1, Qt::Horizontal, i18n("Session"), this );
    cbSesLocation = new QCheckBox( i18n("Restore loca&tion"), gbSession );
    cbSesFilter = new QCheckBox( i18n("Restore last f&ilter"), gbSession );
    lo->addWidget( gbSession );
    connect( cbSesLocation, SIGNAL( toggled( bool ) ), this, SLOT( slotChanged() ) );
    connect( cbSesFilter, SIGNAL( toggled( bool ) ), this, SLOT( slotChanged() ) );

    // make it look nice
    lo->addStretch( 1 );

    // be helpfull
    /*
    QWhatsThis::add( lbAvailableActions, i18n(
          "<p>Available actions for the toolbar. To add an action, select it here "
          "and press the add (<strong>-&gt;</strong>) button" ) );
    QWhatsThis::add( lbUsedActions, i18n(
          "<p>Actions used in the toolbar. To remove an action, select it and "
          "press the remove (<strong>&lt;-</strong>) button."
          "<p>To change the order of the actions, use the Up and Down buttons to "
          "move the selected action.") );
    */
    QString lhwt( i18n(
                      "<p>Decides how many locations to keep in the history of the location "
                      "combo box") );
    Q3WhatsThis::add
        ( lbPathHist, lhwt );
    Q3WhatsThis::add
        ( sbPathHistLength, lhwt );
    QString fhwt( i18n(
                      "<p>Decides how many filters to keep in the history of the filter "
                      "combo box") );
    Q3WhatsThis::add
        ( lbFilterHist, fhwt );
    Q3WhatsThis::add
        ( sbFilterHistLength, fhwt );
    QString synwt( i18n(
                       "<p>These options allow you to have the File Selector automatically "
                       "change location to the directory of the active document on certain "
                       "events."
                       "<p>Auto synchronization is <em>lazy</em>, meaning it will not take "
                       "effect until the file selector is visible."
                       "<p>None of these are enabled by default, but you can always sync the "
                       "location by pressing the sync button in the toolbar.") );
    Q3WhatsThis::add
        ( gbSync, synwt );
    Q3WhatsThis::add
        ( cbSesLocation, i18n(
                    "<p>If this option is enabled (default), the location will be restored "
                    "when you start KDev.<p><strong>Note</strong> that if the session is "
                    "handled by the KDE session manager, the location is always restored.") );
    Q3WhatsThis::add
        ( cbSesFilter, i18n(
                    "<p>If this option is enabled (default), the current filter will be "
                    "restored when you start KDev.<p><strong>Note</strong> that if the "
                    "session is handled by the KDE session manager, the filter is always "
                    "restored."
                    "<p><strong>Note</strong> that some of the autosync settings may "
                    "override the restored location if on.") );

    init();

}

void KFSConfigPage::apply()
{
    KConfig *config = fileSelector->m_part->instance()->config();
    config->setGroup( "fileselector" );
    // toolbar
    QStringList l;
    Q3ListBoxItem *item = acSel->selectedListBox()->firstItem();
    ActionLBItem *aItem;
    while ( item )
    {
        aItem = (ActionLBItem*)item;
        if ( aItem )
        {
            l << aItem->idstring();
        }
        item = item->next();
    }
    config->writeEntry( "toolbar actions", l );
    fileSelector->setupToolbar( config );
    // sync
    int s = 0;
    if ( cbSyncActive->isChecked() )
        s |= KDevFileSelector::DocumentChanged;
    if ( cbSyncOpen->isChecked() )
        s |= KDevFileSelector::DocumentOpened;
    if ( cbSyncShow->isChecked() )
        s |= KDevFileSelector::GotVisible;
    fileSelector->autoSyncEvents = s;
    // reset connections
    disconnect( fileSelector->partController, 0, fileSelector, SLOT( autoSync() ) );
    disconnect( fileSelector->partController, 0,
                fileSelector, SLOT( autoSync( KParts::Part *) ) );
    if ( s & KDevFileSelector::DocumentChanged )
        connect( fileSelector->partController, SIGNAL( viewChanged() ),
                 fileSelector, SLOT( autoSync() ) );
    if ( s & KDevFileSelector::DocumentOpened )
        connect( fileSelector->partController,
                 SIGNAL( partAdded(KParts::Part *) ),
                 fileSelector, SLOT( autoSync(KParts::Part *) ) );

    // histories
    fileSelector->cmbPath->setMaxItems( sbPathHistLength->value() );
    fileSelector->filter->setMaxCount( sbFilterHistLength->value() );
    // session - theese are read/written directly to the app config,
    //           as they are not needed during operation.
    config->writeEntry( "restore location", cbSesLocation->isChecked() );
    config->writeEntry( "restore last filter", cbSesFilter->isChecked() );
}

void KFSConfigPage::reload()
{
    // hmm, what is this supposed to do, actually??
    init();
}

void KFSConfigPage::init()
{
    KConfig *config = fileSelector->m_part->instance()->config();
    config->setGroup( "fileselector" );
    // toolbar
    QStringList l = config->readEntry( "toolbar actions", QStringList() );
    if ( l.isEmpty() ) // default toolbar
        l << "up" << "back" << "forward" << "home" <<
        "short view" << "detailed view" <<
        "bookmarks" << "sync_dir";

    // actions from diroperator + two of our own
    QStringList allActions;
    allActions << "up" << "back" << "forward" << "home" <<
    "reload" << "mkdir" << "delete" <<
    "short view" << "detailed view" /*<< "view menu" <<
                      "show hidden" << "properties"*/ <<
    "bookmarks" << "sync_dir";
    QRegExp re("&(?=[^&])");
    KAction *ac;
    Q3ListBox *lb;
    for ( QStringList::Iterator it=allActions.begin(); it != allActions.end(); ++it )
    {
        lb = l.contains( *it ) ? acSel->selectedListBox() : acSel->availableListBox();
        if ( *it == "bookmarks" || *it == "sync_dir" )
            ac = fileSelector->actionCollection()->action( *it );
        else
            ac = fileSelector->dirOperator()->actionCollection()->action( *it );
        if ( ac )
            new ActionLBItem( lb, ac->icon().pixmap(QSize(16,16)), ac->text().replace( re, "" ), *it );
    }

    // sync
    int s = fileSelector->autoSyncEvents;
    cbSyncActive->setChecked( s & KDevFileSelector::DocumentChanged );
    cbSyncOpen->setChecked( s & KDevFileSelector::DocumentOpened );
    cbSyncShow->setChecked( s & KDevFileSelector::GotVisible );
    // histories
    sbPathHistLength->setValue( fileSelector->cmbPath->maxItems() );
    sbFilterHistLength->setValue( fileSelector->filter->maxCount() );
    // session
    cbSesLocation->setChecked( config->readEntry( "restore location", true ) );
    cbSesFilter->setChecked( config->readEntry( "restore last filter", true ) );
}

void KFSConfigPage::slotChanged()
{
}

//END KFSConfigPage


//BEGIN KDevDirOperator

void KDevDirOperator::activatedMenu( const KFileItem *fi, const QPoint & pos )
{
    setupMenu();
    updateSelectionDependentActions();

    KActionMenu * am = dynamic_cast<KActionMenu*>(actionCollection()->action("popupMenu"));
    if (!am)
        return;
    KMenu *popup = am->menu();

    /*if (fi)
    {
        FileContext context( KUrl::List(fi->url()));
//         if ( KDevApi::self()->core() )
//             KDevApi::self()->core()->fillContextMenu(popup, &context); FIXME find replacement
    }*/

    popup->popup(pos);
}

//END KDevDirOperator

#include "fileselector_widget.moc"
