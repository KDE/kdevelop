#include "fileselector_part.h"


#include <QPushButton>
#include <QCheckBox>
#include <qslider.h>
#include <q3vbox.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <ktextbrowser.h>
#include <kconfig.h>
#include <kfileitem.h>
#include <kdiroperator.h>
#include <kdialog.h>
#include <kvbox.h>
#include <kgenericfactory.h>

#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevmainwindow.h>
#include <kdevdocumentcontroller.h>
#include <kdevplugin.h>

#include <ktip.h>

#include "fileselector_widget.h"

typedef KGenericFactory<FileSelectorPart> KDevFileSelectorFactory;
K_EXPORT_COMPONENT_FACTORY( kdevfileselector, KDevFileSelectorFactory( "kdevfileselector" ) )

FileSelectorPart::FileSelectorPart( QObject *parent, const QStringList& )
        : KDevPlugin( KDevFileSelectorFactory::instance(), parent )
{
    m_filetree = new KDevFileSelector( this, KDevCore::mainWindow(), KDevCore::documentController(), 0 );

    connect( m_filetree->dirOperator(), SIGNAL( fileSelected( const KFileItem* ) ),
             this, SLOT( fileSelected( const KFileItem* ) ) );

    //     FIXME find replacement
    //     connect( KDevApi::self()->core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
    //
    //     connect( KDevApi::self()->core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(slotConfigWidget(KDialogBase*)) );

    m_filetree->setObjectName( i18n( "File Selector" ) );
    m_filetree->setWindowTitle( i18n( "File Selector" ) );
    //m_filetree->setWindowIcon( KIcon(info()->icon()) ); FIXME port
    m_filetree->setWhatsThis( i18n( "<b>File selector</b><p>This file selector lists directory contents and provides some file management functions." ) );

    m_filetree->readConfig( instance() ->config(), "fileselector" );
}

FileSelectorPart::~FileSelectorPart()
{
    delete ( KDevFileSelector* ) m_filetree;
}

QWidget *FileSelectorPart::pluginView() const
{
    return m_filetree;
}

Qt::DockWidgetArea FileSelectorPart::dockWidgetAreaHint() const
{
    return Qt::LeftDockWidgetArea;
}

void FileSelectorPart::fileSelected( const KFileItem * file )
{
    KUrl u( file->url() );

    KDevCore::documentController() ->editDocument( u );
}

void FileSelectorPart::slotProjectOpened()
{
    KUrl u;
    u.setPath( KDevCore::activeProject() ->projectDirectory() );
    m_filetree->setDir( u );
}

void FileSelectorPart::slotConfigWidget( KDialog * dlg )
{
    //FIXME PORT!
    /*    KVBox* vbox = dlg->addVBoxPage( i18n("File Selector"), i18n("File Selector"), BarIcon( info()->icon(), K3Icon::SizeMedium) );
        KFSConfigPage* page = new KFSConfigPage( vbox, 0, m_filetree );
        connect( dlg, SIGNAL( okClicked( ) ), page, SLOT( apply( ) ) );*/ 
    // ### implement reload
}

#include "fileselector_part.moc"
