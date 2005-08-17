#include <kiconloader.h>
#include <qstring.h>
#include <qstringlist.h>
#include <q3ptrlist.h>
#include <q3iconview.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <q3filedialog.h>
#include <qtabwidget.h>
#include <qpoint.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qfileinfo.h>
#include <qmap.h>
#include <qdatetime.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <q3url.h>

#include "kdevdesigner_part.h"

#include "mainwindow.h"
#include "startdialogimpl.h"
#include "designerapp.h"

#include <klocale.h>

FileDialog::FileDialog( const QString &dir, QWidget *parent )
    : Q3FileDialog( dir, QString::null, parent )
{
}

void FileDialog::accept()
{
    emit fileSelected();
}


StartDialog::StartDialog( QWidget *parent, const QString &templatePath )
    : StartDialogBase( parent, 0 )
{
    newForm = new NewForm( templateView, templatePath );
    recentFiles.clear();
    initFileOpen();
    showInFuture = TRUE;

    connect( buttonHelp, SIGNAL( clicked() ),
	     MainWindow::self, SLOT( showDialogHelp() ) );
    connect( recentView, SIGNAL( doubleClicked(Q3IconViewItem*) ),
	     this, SLOT( accept() ) );
    connect( recentView, SIGNAL( returnPressed(Q3IconViewItem*) ),
	     this, SLOT( accept() ) );
    connect( fd, SIGNAL( fileSelected() ), this, SLOT( accept() ) );
}

void StartDialog::accept()
{
    hide();
    showInFuture = !checkShowInFuture->isChecked(); // means don't show it anymore

    int tabindex = tabWidget->currentPageIndex();
    QString filename;
    if( !tabindex ) {
	if ( !templateView->currentItem() )
	    return;
	Project *pro = MainWindow::self->findProject( i18n( "<No Project>" ) );
	if ( !pro )
	    return;
	MainWindow::self->setCurrentProject( pro );
	( (NewItem*)templateView->currentItem() )->insert( pro );
    } else if ( tabindex == 1 ) {
	filename = fd->selectedFile();
    } else if ( tabindex == 2 ) {
	filename = recentFiles[recentView->currentItem()->index()];
    }
    if ( tabindex ) {
	if ( !filename.isEmpty() ) {
	    QFileInfo fi( filename );
	    if ( fi.extension() == "pro" )
		MainWindow::self->openProject( filename );
	    else
		MainWindow::self->fileOpen( "", "", filename );
	}
    }

    done( Accepted );
}

void StartDialog::reject()
{
    done( Rejected );
}

void StartDialog::recentItemChanged( Q3IconViewItem *item )
{
    QString msg( recentFiles[item->index()] );
    QFileInfo f( recentFiles[item->index()] );
    uint s = f.size();
    QString unit( "B" );
    if( s > 1024 ) {
	s /= 1024;
	unit = "KB";
    }
    if ( s > 1024 ) {
	s /= 1024;
	unit = "MB";
    }
    QDateTime dt( f.lastModified() );
    QString date( dt.toString( "MMMM dd hh:mm" ));
    msg = QString( "%1 (%2 %3)  %4" ).arg(msg).arg(s).arg(unit).arg(date);
    fileInfoLabel->setText( msg );
}

void StartDialog::clearFileInfo()
{
    fileInfoLabel->clear();
}

void StartDialog::setRecentlyFiles( QStringList &files )
{
    insertRecentItems( files, FALSE );
}

void StartDialog::setRecentlyProjects( QStringList &projects )
{
    insertRecentItems( projects, TRUE );
}

void StartDialog::insertRecentItems( QStringList &files, bool isProject )
{
    QString iconName = "designer_newform.png";
    if ( isProject )
	iconName = "designer_project.png";
    Q3IconViewItem *item;
    QStringList::iterator it = files.begin();
    for( ; it != files.end(); ++it ) {
	QFileInfo fi( *it );
	item = new Q3IconViewItem( recentView, fi.fileName() );
	recentFiles[recentView->index( item )] = *it;
	item->setPixmap( BarIcon( iconName, KDevDesignerPartFactory::instance() ) );
	item->setDragEnabled( FALSE );
    }
}

void StartDialog::initFileOpen()
{
    QString encode = QDir::currentDirPath();
    Q3Url::encode( encode );
    fd = new FileDialog( encode, this );
    QPoint point( 0, 0 );
    fd->reparent( tab, point );

    QObjectList *l = fd->queryList( "QPushButton" );
    QObjectListIt it( *l );
    QObject *obj;
    while ( (obj = it.current()) != 0 ) {
        ++it;
	((QPushButton*)obj)->hide();
    }
    delete l;

    fd->setSizeGripEnabled ( FALSE );
    tabLayout->addWidget( fd );

    QPluginManager<ImportFilterInterface> manager( IID_ImportFilter, QApplication::libraryPaths(),
						   MainWindow::self->pluginDirectory() );
    QStringList filterlist;
    filterlist << i18n( "Designer Files (*.ui *.pro)" );
    filterlist << i18n( "Qt User-Interface Files (*.ui)" );
    filterlist << i18n( "QMAKE Project Files (*.pro)" );

    QStringList list = manager.featureList();
    for ( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 )
	filterlist << *it2;

    filterlist << i18n( "All Files (*)" );
    QString filters = filterlist.join( ";;" );
    fd->setFilters( filters );

    fd->show();
}
