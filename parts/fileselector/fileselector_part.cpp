#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qvbox.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <ktextbrowser.h>
#include <kconfig.h>
#include <kfileitem.h>
#include <kdevgenericfactory.h>
#include <kdiroperator.h>
#include <kdialogbase.h>

#include <kdevapi.h>
#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevmainwindow.h>
#include <kdevpartcontroller.h>

#include <ktip.h>

#include "fileselector_widget.h"
#include "fileselector_part.h"

typedef KDevGenericFactory<FileSelectorPart> FileSelectorFactory;
static const KAboutData data("kdevfileselector", I18N_NOOP("File Selector"), "1.0");
K_EXPORT_COMPONENT_FACTORY( libkdevfileselector, FileSelectorFactory( &data ) )

FileSelectorPart::FileSelectorPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin("FileSelector", "fileselector", parent, name ? name : "FileSelectorPart")
{
    setInstance(FileSelectorFactory::instance());

    m_filetree = new KDevFileSelector( this, mainWindow(), partController(), 0, "fileselectorwidget" );

    connect( m_filetree->dirOperator(), SIGNAL(fileSelected(const KFileItem*)),
	     this, SLOT(fileSelected(const KFileItem*)));
    connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );

    connect( core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(slotConfigWidget(KDialogBase*)) );

    m_filetree->setCaption( i18n("File Selector") );
    m_filetree->setIcon( SmallIcon("view_detailed") );
    mainWindow()->embedSelectView( m_filetree, i18n("File Selector"), i18n("File selector") );
    QWhatsThis::add(m_filetree, i18n("<b>File selector</b><p>This file selector lists directory contents and provides some file management functions."));

    m_filetree->readConfig( instance()->config(), "fileselector" );
}

FileSelectorPart::~FileSelectorPart()
{
    if (m_filetree){
	mainWindow()->removeView( m_filetree );
    }

    delete (KDevFileSelector*) m_filetree;
}

void FileSelectorPart::fileSelected( const KFileItem * file )
{
    KURL u(file->url());

    partController()->editDocument( u );
}

void FileSelectorPart::slotProjectOpened()
{
    KURL u;
    u.setPath( project()->projectDirectory() );
    m_filetree->setDir( u );
}

void FileSelectorPart::slotConfigWidget( KDialogBase * dlg )
{
    QVBox* vbox = dlg->addVBoxPage( i18n( "File Selector" ) );
    KFSConfigPage* page = new KFSConfigPage( vbox, 0, m_filetree );
    connect( dlg, SIGNAL( okClicked( ) ), page, SLOT( apply( ) ) );
    // ### implement reload
}

#include "fileselector_part.moc"
