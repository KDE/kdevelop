//Added by qt3to4:
#include <QCloseEvent>
%{CPP_TEMPLATE}

#include "%{APPNAMELC}.h"

#include <qimage.h>
#include <qpixmap.h>
#include <q3toolbar.h>
#include <qtoolbutton.h>
#include <q3popupmenu.h>
#include <qmenubar.h>
#include <q3textedit.h>
#include <qfile.h>
#include <q3filedialog.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qapplication.h>
#include <q3accel.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <q3paintdevicemetrics.h>
#include <q3whatsthis.h>

#include "filesave.xpm"
#include "fileopen.xpm"
#include "fileprint.xpm"

%{APPNAME}::%{APPNAME}()
    : Q3MainWindow( 0, "%{APPNAME}", Qt::WDestructiveClose )
{
    printer = new QPrinter;
    QPixmap openIcon, saveIcon, printIcon;

    Q3ToolBar * fileTools = new Q3ToolBar( this, "file operations" );
    fileTools->setLabel( tr("File Operations") );

    openIcon = QPixmap( fileopen );
    QToolButton * fileOpen
	= new QToolButton( openIcon, tr("Open File"), QString::null,
			   this, SLOT(choose()), fileTools, "open file" );

    saveIcon = QPixmap( filesave );
    QToolButton * fileSave
	= new QToolButton( saveIcon, tr("Save File"), QString::null,
			   this, SLOT(save()), fileTools, "save file" );

    printIcon = QPixmap( fileprint );
    QToolButton * filePrint
	= new QToolButton( printIcon, tr("Print File"), QString::null,
			   this, SLOT(print()), fileTools, "print file" );


    (void)Q3WhatsThis::whatsThisButton( fileTools );

    QString fileOpenText = tr("<p><img source=\"fileopen\"> "
	         "Click this button to open a <em>new file</em>. <br>"
                 "You can also select the <b>Open</b> command "
                 "from the <b>File</b> menu.</p>");

    Q3WhatsThis::add( fileOpen, fileOpenText );

    Q3MimeSourceFactory::defaultFactory()->setPixmap( "fileopen", openIcon );

    QString fileSaveText = tr("<p>Click this button to save the file you "
                 "are editing. You will be prompted for a file name.\n"
                 "You can also select the <b>Save</b> command "
                 "from the <b>File</b> menu.</p>");

    Q3WhatsThis::add( fileSave, fileSaveText );

    QString filePrintText = tr("Click this button to print the file you "
                 "are editing.\n You can also select the Print "
                 "command from the File menu.");

    Q3WhatsThis::add( filePrint, filePrintText );


    Q3PopupMenu * file = new Q3PopupMenu( this );
    menuBar()->insertItem( tr("&File"), file );


    file->insertItem( tr("&New"), this, SLOT(newDoc()), Qt::CTRL+Qt::Key_N );

    int id;
    id = file->insertItem( openIcon, tr("&Open..."),
			   this, SLOT(choose()), Qt::CTRL+Qt::Key_O );
    file->setWhatsThis( id, fileOpenText );

    id = file->insertItem( saveIcon, tr("&Save"),
			   this, SLOT(save()), Qt::CTRL+Qt::Key_S );
    file->setWhatsThis( id, fileSaveText );

    id = file->insertItem( tr("Save &As..."), this, SLOT(saveAs()) );
    file->setWhatsThis( id, fileSaveText );

    file->insertSeparator();

    id = file->insertItem( printIcon, tr("&Print..."),
			   this, SLOT(print()), Qt::CTRL+Qt::Key_P );
    file->setWhatsThis( id, filePrintText );

    file->insertSeparator();

    file->insertItem( tr("&Close"), this, SLOT(close()), Qt::CTRL+Qt::Key_W );

    file->insertItem( tr("&Quit"), qApp, SLOT( closeAllWindows() ), Qt::CTRL+Qt::Key_Q );

    menuBar()->insertSeparator();

    Q3PopupMenu * help = new Q3PopupMenu( this );
    menuBar()->insertItem( tr("&Help"), help );

    help->insertItem( tr("&About"), this, SLOT(about()), Qt::Key_F1 );
    help->insertItem( tr("About &Qt"), this, SLOT(aboutQt()) );
    help->insertSeparator();
    help->insertItem( tr("What's &This"), this, SLOT(whatsThis()), Qt::SHIFT+Qt::Key_F1 );

    e = new Q3TextEdit( this, "editor" );
    e->setFocus();
    setCentralWidget( e );
    statusBar()->message( tr("Ready"), 2000 );

    resize( 450, 600 );
}


%{APPNAME}::~%{APPNAME}()
{
    delete printer;
}



void %{APPNAME}::newDoc()
{
    %{APPNAME} *ed = new %{APPNAME};
    ed->setCaption(tr("Qt Example - Application"));
    ed->show();
}

void %{APPNAME}::choose()
{
    QString fn = Q3FileDialog::getOpenFileName( QString::null, QString::null,
					       this);
    if ( !fn.isEmpty() )
	load( fn );
    else
	statusBar()->message( tr("Loading aborted"), 2000 );
}


void %{APPNAME}::load( const QString &fileName )
{
    QFile f( fileName );
    if ( !f.open( QIODevice::ReadOnly ) )
	return;

    QTextStream ts( &f );
    e->setText( ts.read() );
    e->setModified( FALSE );
    setCaption( fileName );
    statusBar()->message( tr("Loaded document %1").arg(fileName), 2000 );
}


void %{APPNAME}::save()
{
    if ( filename.isEmpty() ) {
	saveAs();
	return;
    }

    QString text = e->text();
    QFile f( filename );
    if ( !f.open( QIODevice::WriteOnly ) ) {
	statusBar()->message( tr("Could not write to %1").arg(filename),
			      2000 );
	return;
    }

    QTextStream t( &f );
    t << text;
    f.close();

    e->setModified( FALSE );

    setCaption( filename );

    statusBar()->message( tr( "File %1 saved" ).arg( filename ), 2000 );
}


void %{APPNAME}::saveAs()
{
    QString fn = Q3FileDialog::getSaveFileName( QString::null, QString::null,
					       this );
    if ( !fn.isEmpty() ) {
	filename = fn;
	save();
    } else {
	statusBar()->message( tr("Saving aborted"), 2000 );
    }
}


void %{APPNAME}::print()
{
    // ###### Rewrite to use QSimpleRichText to print here as well
    const int Margin = 10;
    int pageNo = 1;

    if ( printer->setup(this) ) {		// printer dialog
	statusBar()->message( tr("Printing...") );
	QPainter p;
	if( !p.begin( printer ) )               // paint on printer
	    return;

	p.setFont( e->font() );
	int yPos	= 0;			// y-position for each line
	QFontMetrics fm = p.fontMetrics();
	Q3PaintDeviceMetrics metrics( printer ); // need width/height
						// of printer surface
	for( int i = 0 ; i < e->lines() ; i++ ) {
	    if ( Margin + yPos > metrics.height() - Margin ) {
		QString msg( "Printing (page " );
		msg += QString::number( ++pageNo );
		msg += ")...";
		statusBar()->message( msg );
		printer->newPage();		// no more room on this page
		yPos = 0;			// back to top of page
	    }
	    p.drawText( Margin, Margin + yPos,
			metrics.width(), fm.lineSpacing(),
			ExpandTabs | DontClip,
			e->text( i ) );
	    yPos = yPos + fm.lineSpacing();
	}
	p.end();				// send job to printer
	statusBar()->message( tr("Printing completed"), 2000 );
    } else {
	statusBar()->message( tr("Printing aborted"), 2000 );
    }
}

void %{APPNAME}::closeEvent( QCloseEvent* ce )
{
    if ( !e->isModified() ) {
	ce->accept();
	return;
    }

    switch( QMessageBox::information( this, tr("Qt Application Example"),
				      tr("Do you want to save the changes"
				      " to the document?"),
				      tr("Yes"), tr("No"), tr("Cancel"),
				      0, 1 ) ) {
    case 0:
	save();
	ce->accept();
	break;
    case 1:
	ce->accept();
	break;
    case 2:
    default: // just for sanity
	ce->ignore();
	break;
    }
}


void %{APPNAME}::about()
{
    QMessageBox::about( this, tr("Qt Application Example"),
			tr("This example demonstrates simple use of "
			"QMainWindow,\nQMenuBar and QToolBar."));
}


void %{APPNAME}::aboutQt()
{
    QMessageBox::aboutQt( this, tr("Qt Application Example") );
}
