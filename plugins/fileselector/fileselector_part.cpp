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
#include <kdevgenericfactory.h>
#include <kdiroperator.h>
#include <kdialog.h>
#include <kvbox.h>

#include <kdevapi.h>
#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevmainwindow.h>
#include <kdevdocumentcontroller.h>
#include <kdevplugininfo.h>

#include <ktip.h>

#include "fileselector_widget.h"

typedef KGenericFactory<FileSelectorPart> FileSelectorFactory;
K_EXPORT_COMPONENT_FACTORY( kdevfileselector, FileSelectorFactory )

FileSelectorPart::FileSelectorPart(QObject *parent, const QStringList&)
    : KDevPlugin(FileSelectorFactory::instance(), parent)
{
    m_filetree = new KDevFileSelector( this, KDevApi::self()->mainWindow(), KDevApi::self()->documentController(), 0 );

    connect( m_filetree->dirOperator(), SIGNAL(fileSelected(const KFileItem*)),
	     this, SLOT(fileSelected(const KFileItem*)));
    connect( KDevApi::self()->core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );

    connect( KDevApi::self()->core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(slotConfigWidget(KDialogBase*)) );

    m_filetree->setWindowTitle( i18n("File Selector") );
    //m_filetree->setWindowIcon( KIcon(info()->icon()) ); FIXME port
    KDevApi::self()->mainWindow()->embedSelectView( m_filetree, i18n("File Selector"), i18n("File selector") );
    m_filetree->setWhatsThis( i18n("<b>File selector</b><p>This file selector lists directory contents and provides some file management functions."));

    m_filetree->readConfig( instance()->config(), "fileselector" );
}

FileSelectorPart::~FileSelectorPart()
{
    if (m_filetree){
        KDevApi::self()->mainWindow()->removeView( m_filetree );
    }

    delete (KDevFileSelector*) m_filetree;
}

void FileSelectorPart::fileSelected( const KFileItem * file )
{
    KUrl u(file->url());

    KDevApi::self()->documentController()->editDocument( u );
}

void FileSelectorPart::slotProjectOpened()
{
    KUrl u;
    u.setPath( KDevApi::self()->project()->projectDirectory() );
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
