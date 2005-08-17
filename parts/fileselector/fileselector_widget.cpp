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
#include <kdevpartcontroller.h>

#include <qlayout.h>
#include <qtoolbutton.h>
#include <q3hbox.h>
#include <q3vbox.h>
#include <qlabel.h>
#include <q3strlist.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qapplication.h>
#include <q3listbox.h>
#include <qscrollbar.h>
#include <qspinbox.h>
#include <q3groupbox.h>
#include <qcheckbox.h>
#include <qregexp.h>
#include <q3dockarea.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QPixmap>
#include <QFocusEvent>
#include <QEvent>
#include <Q3Frame>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QResizeEvent>

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
#include <ktoolbarbutton.h>
#include <q3toolbar.h>
#include <kpopupmenu.h>
#include <kdialog.h>
#include <kio/netaccess.h>

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


KDevFileSelectorToolBar::KDevFileSelectorToolBar(QWidget *parent)
        : KToolBar( parent, "KDev FileSelector Toolbar", true )
{
    setMinimumWidth(10);
}

KDevFileSelectorToolBar::~KDevFileSelectorToolBar()
{}

void KDevFileSelectorToolBar::setMovingEnabled( bool)
{
    KToolBar::setMovingEnabled(false);
}


KDevFileSelectorToolBarParent::KDevFileSelectorToolBarParent(QWidget *parent)
        :Q3Frame(parent),m_tb(0)
{}
KDevFileSelectorToolBarParent::~KDevFileSelectorToolBarParent()
{}
void KDevFileSelectorToolBarParent::setToolBar(KDevFileSelectorToolBar *tb)
{
    m_tb=tb;
}

void KDevFileSelectorToolBarParent::resizeEvent ( QResizeEvent * )
{
    if (m_tb)
    {
        setMinimumHeight(m_tb->sizeHint().height());
        m_tb->resize(width(),height());
    }
}


//BEGIN Constructor/destructor

KDevFileSelector::KDevFileSelector( FileSelectorPart *part, KDevMainWindow *mainWindow,
                                    KDevPartController *partController,
                                    QWidget * parent, const char * name )
        : QWidget(parent, name),
        m_part(part),
        mainwin(mainWindow),
        partController(partController)
{
    mActionCollection = new KActionCollection( this );

    QVBoxLayout* lo = new QVBoxLayout(this);

    QtMsgHandler oldHandler = qInstallMsgHandler( silenceQToolBar );

    KDevFileSelectorToolBarParent *tbp=new KDevFileSelectorToolBarParent(this);
    toolbar = new KDevFileSelectorToolBar(tbp);
    tbp->setToolBar(toolbar);
    lo->addWidget(tbp);
    toolbar->setMovingEnabled(false);
    toolbar->setFlat(true);
    qInstallMsgHandler( oldHandler );

    cmbPath = new KURLComboBox( KURLComboBox::Directories, true, this, "path combo" );
    cmbPath->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
    KURLCompletion* cmpl = new KURLCompletion(KURLCompletion::DirCompletion);
    cmbPath->setCompletionObject( cmpl );
    lo->addWidget(cmbPath);
    cmbPath->listBox()->installEventFilter( this );

    dir = new KDevDirOperator(m_part, KURL(), this, "operator");
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
    KActionMenu *acmBookmarks = new KActionMenu( i18n("Bookmarks"), "bookmark",
                                mActionCollection, "bookmarks" );
    acmBookmarks->setDelayed( false );

    bookmarkHandler = new KBookmarkHandler( this, acmBookmarks->popupMenu() );

    Q3HBox* filterBox = new Q3HBox(this);

    btnFilter = new QToolButton( filterBox );
    btnFilter->setIconSet( SmallIconSet("filter" ) );
    btnFilter->setToggleButton( true );
    filter = new KHistoryCombo( true, filterBox, "filter");
    filter->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
    filterBox->setStretchFactor(filter, 2);
    connect( btnFilter, SIGNAL( clicked() ), this, SLOT( btnFilterClick() ) );
    lo->addWidget(filterBox);

    connect( filter, SIGNAL( activated(const QString&) ),
             SLOT( slotFilterChange(const QString&) ) );
    connect( filter, SIGNAL( returnPressed(const QString&) ),
             filter, SLOT( addToHistory(const QString&) ) );

    // kaction for the dir sync method
    acSyncDir = new KAction( i18n("Current Document Directory"), "dirsynch", 0,
                             this, SLOT( setActiveDocumentDir() ), mActionCollection, "sync_dir" );
    toolbar->setIconText( KToolBar::IconOnly );
    toolbar->setIconSize( 16 );
    toolbar->setEnableContextMenu( false );

    connect( cmbPath, SIGNAL( urlActivated( const KURL&  )),
             this,  SLOT( cmbPathActivated( const KURL& ) ));
    connect( cmbPath, SIGNAL( returnPressed( const QString&  )),
             this,  SLOT( cmbPathReturnPressed( const QString& ) ));
    connect(dir, SIGNAL(urlEntered(const KURL&)),
            this, SLOT(dirUrlEntered(const KURL&)) );

    connect(dir, SIGNAL(finishedLoading()),
            this, SLOT(dirFinishedLoading()) );

    // enable dir sync button if current doc has a valid URL
    connect ( partController, SIGNAL(activePartChanged(KParts::Part*) ),
              this, SLOT(viewChanged() ) );

    // Connect the bookmark handler
    connect( bookmarkHandler, SIGNAL( openURL( const QString& )),
             this, SLOT( setDir( const QString& ) ) );

    waitingUrl = QString();

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
    dir->readConfig(config, name + ":dir");
    dir->setView( KFile::Default );

    config->setGroup( name );

    // set up the toolbar
    setupToolbar( config );

    cmbPath->setMaxItems( config->readNumEntry( "pathcombo history len", 9 ) );
#if defined(_KDE_3_1_3_)
    cmbPath->setURLs( config->readPathListEntry("dir history") );
#else
    cmbPath->setURLs( config->readListEntry("dir history") );
#endif
    // if we restore history
    if ( config->readBoolEntry( "restore location", true ) || kapp->isRestored() )
    {
        QString loc( config->readPathEntry( "location" ) );
        if ( ! loc.isEmpty() )
        {
            waitingDir = loc;
            QTimer::singleShot(0, this, SLOT(initialDirChangeHack()));
        }
    }

    // else is automatic, as cmpPath->setURL is called when a location is entered.

    filter->setMaxCount( config->readNumEntry( "filter history len", 9 ) );
    filter->setHistoryItems( config->readListEntry("filter history"), true );
    lastFilter = config->readEntry( "last filter" );
    QString flt("");
    if ( config->readBoolEntry( "restore last filter", true ) || kapp->isRestored() )
        flt = config->readEntry("current filter");
    filter->lineEdit()->setText( flt );
    slotFilterChange( flt );

    autoSyncEvents = config->readNumEntry( "AutoSyncEvents", 0 );
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
    QStringList tbactions = config->readListEntry( "toolbar actions", ',' );
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
            ac = mActionCollection->action( (*it).latin1() );
        else
            ac = dir->actionCollection()->action( (*it).latin1() );
        if ( ac )
            ac->plug( toolbar );
    }
}

void KDevFileSelector::writeConfig(KConfig *config, const QString & name)
{
    dir->writeConfig(config,name + ":dir");

    config->setGroup( name );
    config->writeEntry( "pathcombo history len", cmbPath->maxItems() );
    QStringList l;
    for (int i = 0; i < cmbPath->count(); i++)
    {
        l.append( cmbPath->text( i ) );
    }
#if defined(_KDE_3_1_3_)
    config->writePathEntry( "dir history", l );
    config->writePathEntry( "location", cmbPath->currentText() );
#else
    config->writeEntry( "dir history", l );
    config->writeEntry( "location", cmbPath->currentText() );
#endif

    config->writeEntry( "filter history len", filter->maxCount() );
    config->writeEntry( "filter history", filter->historyItems() );
    config->writeEntry( "current filter", filter->currentText() );
    config->writeEntry( "last filter", lastFilter );
    config->writeEntry( "AutoSyncEvents", autoSyncEvents );
}

void KDevFileSelector::setView(KFile::FileView view)
{
    dir->setView(view);
}

//END Public Methods

//BEGIN Public Slots

void KDevFileSelector::slotFilterChange( const QString & nf )
{
    QToolTip::remove( btnFilter );
    QString f = nf.stripWhiteSpace();
    bool empty = f.isEmpty() || f == "*";
    if ( empty )
    {
        dir->clearFilter();
        filter->lineEdit()->setText( QString() );
        QToolTip::add
            ( btnFilter,
                    QString( i18n("Apply last filter (\"%1\")") ).arg( lastFilter ) );
    }
    else
    {
        dir->setNameFilter( f );
        lastFilter = f;
        QToolTip::add
            ( btnFilter, i18n("Clear filter") );
    }
    btnFilter->setOn( !empty );
    dir->updateDir();
    // this will be never true after the filter has been used;)
    btnFilter->setEnabled( !( empty && lastFilter.isEmpty() ) );

}
void KDevFileSelector::setDir( KURL u )
{
    dir->setURL(u, true);
}

//END Public Slots

//BEGIN Private Slots

void KDevFileSelector::cmbPathActivated( const KURL& u )
{
    cmbPathReturnPressed( u.url() );
}

void KDevFileSelector::cmbPathReturnPressed( const QString& u )
{
    QStringList urls = cmbPath->urls();
    urls.remove( u );
    urls.prepend( u );
    cmbPath->setURLs( urls, KURLComboBox::RemoveBottom );
    dir->setFocus();
    dir->setURL( KURL(u), true );
}

void KDevFileSelector::dirUrlEntered( const KURL& u )
{
    cmbPath->setURL( u );
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
        slotFilterChange( QString() );
    }
    else
    {
        filter->lineEdit()->setText( lastFilter );
        slotFilterChange( lastFilter );
    }
}


void KDevFileSelector::autoSync()
{
    kdDebug()<<"KDevFileSelector::autoSync()"<<endl;
    // if visible, sync
    if ( isVisible() )
    {
        setActiveDocumentDir();
        waitingUrl = QString();
    }
    // else set waiting url
    else
    {
        KURL u = activeDocumentUrl();
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
    kdDebug()<<"KDevFileSelector::autoSync( KTextEditor::Document )"<<endl;
    KURL u ( doc->url() );
    if ( u.isEmpty() )
    {
        waitingUrl = QString();
        return;
    }
    if ( isVisible() )
    {
        setDir( u.directory() );
        waitingUrl = QString();
    }
    else
    {
        waitingUrl = u.directory();
    }
}
/// \FIXME crash on shutdown
void KDevFileSelector::setActiveDocumentDir()
{
    //kdDebug()<<"KDevFileSelector::setActiveDocumentDir()"<<endl;
    KURL u = activeDocumentUrl();
    if (!u.isEmpty())
        setDir( u.upURL() );
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
        kdDebug()<<"syncing fs on show"<<endl;
        setActiveDocumentDir();
        waitingUrl = QString();
    }
    // else, if we have a waiting URL set it
    else if ( ! waitingUrl.isEmpty() )
    {
        setDir( waitingUrl );
        waitingUrl = QString();
    }
}

bool KDevFileSelector::eventFilter( QObject* o, QEvent *e )
{
    /*
        This is rather unfortunate, but:
        QComboBox does not support setting the size of the listbox to something
        resonable. Even using listbox->setVariableWidth() does not yeld a
        satisfying result, something is wrong with the handling of the sizehint.
        And the popup is rather useless, if the paths are only partly visible.
    */
    Q3ListBox *lb = cmbPath->listBox();
    if ( o == lb && e->type() == QEvent::Show )
    {
        int add
            = lb->height() < lb->contentsHeight() ? lb->verticalScrollBar()->width() : 0;
        int w = qMin( mainwin->main()->width(), lb->contentsWidth() + add );
        lb->resize( w, lb->height() );
        /// @todo - move the listbox to a suitable place if nessecary
        /// @todo - decide if it is worth caching the size while untill the contents
        ///        are changed.
    }
    /// @todo - same thing for the completion popup?
    return QWidget::eventFilter( o, e );
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
                  const QString &text=QString(),
                  const QString &str=QString() ) :
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

KURL KDevFileSelector::activeDocumentUrl( )
{
	KParts::ReadOnlyPart * part = dynamic_cast<KParts::ReadOnlyPart*>( partController->activePart() );
	if ( part )
	{
		return part->url();
	}

    return KURL();
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
    QStringList l = config->readListEntry( "toolbar actions", ',' );
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
            ac = fileSelector->actionCollection()->action( (*it).latin1() );
        else
            ac = fileSelector->dirOperator()->actionCollection()->action( (*it).latin1() );
        if ( ac )
            new ActionLBItem( lb, SmallIcon( ac->icon() ), ac->text().replace( re, "" ), *it );
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
    cbSesLocation->setChecked( config->readBoolEntry( "restore location", true ) );
    cbSesFilter->setChecked( config->readBoolEntry( "restore last filter", true ) );
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
    KPopupMenu *popup = am->popupMenu();

    if (fi)
    {
        FileContext context( KURL::List(fi->url()));
        if ( (m_part) && (m_part->core()))
            m_part->core()->fillContextMenu(popup, &context);
    }

    popup->popup(pos);
}

//END KDevDirOperator

#include "fileselector_widget.moc"
