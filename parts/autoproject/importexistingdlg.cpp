#include <qapplication.h>
#include <qvariant.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qfontmetrics.h>
#include <qrect.h>
#include <qsize.h>

#include <kdebug.h>
#include <kfileview.h>
#include <kguiitem.h>
#include <kprogress.h>
#include <kprocess.h>
#include <kurldrag.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kglobalsettings.h>
#include <kiconview.h>
#include <ksqueezedtextlabel.h>
#include <kwordwrap.h>

#include "importexistingdlg.h"
#include "autoprojectwidget.h"
#include "autoprojectpart.h"
#include "misc.h"

KImportIconView::KImportIconView ( const QString& strIntro, QWidget* parent, const char* name )
        : KFileDnDIconView ( parent, name )
{
    m_strIntro = strIntro;
    m_bDropped = false;

    setAcceptDrops ( true );

}


void KImportIconView::drawContents ( QPainter *p, int cx, int cy, int cw, int ch )
{
    if ( !m_bDropped)
    {
        QIconView::drawContents ( p, cx, cy, cw, ch );

        p->save();
        QFont font ( p->font() );
        font.setBold ( true );
        font.setFamily ( "Helvetica [Adobe]" );
        font.setPointSize ( 10 );
        p->setFont ( font );
        p->setPen ( QPen ( KGlobalSettings::inactiveTextColor() ) );

        QRect rect = frameRect();
        QFontMetrics fm ( p->font() );
        rect.setLeft ( rect.left() + 30 );
        rect.setRight ( rect.right() - 30 );

        resizeContents ( contentsWidth(), contentsHeight() );

        // word-wrap the string
        KWordWrap* wordWrap1 = KWordWrap::formatText( fm, rect, AlignHCenter | WordBreak, m_strIntro );
        KWordWrap* wordWrap2 = KWordWrap::formatText( fm, rect, AlignHCenter | WordBreak, "Or just use the buttons!" );

        QRect introRect1 = wordWrap1->boundingRect();
		QRect introRect2 = wordWrap2->boundingRect();

        wordWrap1->drawText ( p, ( ( frameRect().right() - introRect1.right() ) / 2 ), ( ( frameRect().bottom() - introRect1.bottom() ) / 2 ) - 50, AlignHCenter | AlignVCenter );
        wordWrap2->drawText ( p, ( ( frameRect().right() - introRect2.right() ) / 2 ), ( ( frameRect().bottom() - introRect2.bottom() ) / 2 ) + introRect1.bottom(), AlignHCenter | AlignVCenter );

        p->restore();
    }
    else
    {
        QIconView::drawContents ( p, cx, cy, cw, ch );
    }
}

void KImportIconView::somethingDropped ( bool dropped )
{
    m_bDropped = dropped;
}

/*
 *  Constructs a ImportExistingDialog which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */

ImportExistingDialog::ImportExistingDialog ( AutoProjectPart* part, AutoProjectWidget *widget, SubprojectItem* spitem, TargetItem* titem, QWidget* parent, const char* name, bool modal, WFlags fl )
        : ImportExistingDlgBase ( parent, name, modal, fl )
{
    m_bImportingFiles = true;

	m_spitem = spitem;
	m_titem = titem;

	m_part = part;
	m_widget = widget;

	KFile::Mode mode = KFile::Files;

    if ( titem && spitem && titem->type() == ProjectItem::Target && spitem->type() == ProjectItem::Subproject )
    {
        destStaticLabel->setText ( i18n ( "Target:" ) );
        destLabel->setText ( titem->name );
        targetLabel->setText ( titem->name );
        directoryLabel->setText ( spitem->path );
    }

    sourceSelector = new FileSelectorWidget ( part, mode, sourceGroupBox, "source file selector" );
    sourceGroupBoxLayout->addWidget ( sourceSelector );

    importView = new KImportIconView ( "Drag one or more files from above and drop it here!", destGroupBox, "destination icon view" );
   destGroupBoxLayout->addWidget ( importView );
    //destGroupBoxLayout->setStretchFactor(dir, 2);

    setIcon ( SmallIcon ( "fileimport.png" ) );

    init();
}

ImportExistingDialog::ImportExistingDialog ( AutoProjectPart* part, AutoProjectWidget *widget, SubprojectItem* spitem, QWidget* parent, const char* name, bool modal, WFlags fl )
        : ImportExistingDlgBase ( parent, name, modal, fl )
{
	m_spitem = spitem;

	m_part = part;
	m_widget = widget;

	m_bImportingFiles = false;

    KFile::Mode mode = KFile::Directory;

    if ( spitem && spitem->type() == ProjectItem::Subproject )
    {
        destStaticLabel->setText ( i18n ( "Subproject:" ) );
        destLabel->setText ( spitem->subdir );
        targetLabel->setText ( i18n ( "none" ) );
        directoryLabel->setText ( i18n ( spitem->path ) );
    }

    sourceSelector = new FileSelectorWidget ( part, mode, sourceGroupBox, "source file selector" );
    sourceGroupBoxLayout->addWidget ( sourceSelector );

    importView = new KImportIconView ( "Drag one or more directories with an existing Makefile.am from above and drop it here!", destGroupBox, "destination icon view" );
    destGroupBoxLayout->addWidget ( importView );

    setIcon ( SmallIcon ( "fileimport.png" ) );

    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
ImportExistingDialog::~ImportExistingDialog()
{
    // no need to delete child widgets, Qt does it all for us
}


void ImportExistingDialog::resizeEvent ( QResizeEvent* ev )
{
    ImportExistingDlgBase::resizeEvent ( ev );
    //importView->update();
}


void ImportExistingDialog::init()
{
	progressBar->hide();

	importView->setMode ( KIconView::Select );
    importView->setItemsMovable ( false );

	connect ( okButton, SIGNAL ( clicked () ), this, SLOT ( slotOk () ) );
	connect ( helpButton, SIGNAL ( clicked () ), this, SLOT ( slotHelp () ) );

    connect ( addSelectedButton, SIGNAL ( clicked () ), this, SLOT ( slotAddSelected() ) );
    connect ( addAllButton, SIGNAL ( clicked () ), this, SLOT ( slotAddAll() ) );
    connect ( removeSelectedButton, SIGNAL ( clicked () ), this, SLOT ( slotRemoveSelected() ) );
    connect ( removeAllButton, SIGNAL ( clicked () ), this, SLOT ( slotRemoveAll() ) );

    connect ( importView, SIGNAL ( dropped( KFileView*, QDropEvent* ) ), this, SLOT ( slotDropped ( KFileView*, QDropEvent* ) ) );

    importView->setSelectionMode ( KFile::Multi );
	
	sourceSelector->setDir ( m_spitem->path );
}

void ImportExistingDialog::importItems()
{
	// items added via button or drag 'n drop
	KFileItemListIterator itemList ( m_importList );
	
	// items already added to the importView
	KFileItemListIterator importedList ( *importView->items() );

	QListViewItem* child = m_titem->firstChild();
	
	QStringList duplicateList;

	while ( child )
	{
		FileItem* curItem = static_cast<FileItem*> ( child );

		itemList.toFirst();

		for ( ; itemList.current(); ++itemList )
		{
			if ( ( *itemList )->name() == curItem->name )
			{
				duplicateList.append ( ( *itemList )->name() );
				m_importList.remove ( ( *itemList ) );
			}
		}

		child = child->nextSibling();
	}

	importedList.toFirst();

	for ( ; importedList.current(); ++importedList )
	{
		itemList.toFirst();

		for ( ; itemList.current(); ++itemList )
		{
			if ( ( *importedList )->name() == ( *itemList )->name() )
			{
				m_importList.remove ( ( *itemList ) );

				// to avoid that a item is added twice
				if ( !duplicateList.remove ( ( *importedList )->name() ) )
				{
					duplicateList.append ( ( *importedList )->name() );
				}
			}
		}
	}

	if ( duplicateList.count() > 0 )
	{
		if ( KMessageBox::warningContinueCancelList ( this, i18n (
																		"The following file(s) already exit(s) in the target!\n"
																		"Press Continue to import only the new files.\n"
																		"Press Cancel to abort the complete import." ),
																		duplicateList, "Warning", KGuiItem ( i18n ( "Continue" ) ) ) == KMessageBox::Cancel )
		{
			m_importList.clear();
			return;
		}
	}

	itemList.toFirst();
	
	for ( ; itemList.current(); ++itemList )
	{
		if ( m_bImportingFiles && ( *itemList )->isFile() ||
			!m_bImportingFiles && ( *itemList )->isDir() )
		{
			importView->insertItem ( ( *itemList ) );
		}
	}
	
	importView->somethingDropped ( true );

	m_importList.clear();

	importView->update ();
}

void ImportExistingDialog::slotOk()
{
	progressBar->show();
	progressBar->setFormat ( i18n ( "Importing... %p%" ) );
	
	qApp->processEvents();

	KFileItemListIterator items ( *importView->items() );

	// contains at the end only the imported files outside the subproject directory
	KFileItemList outsideList;

	QStringList stringList;

	for ( ; items.current(); ++items )
	{
		// kdDebug ( 9000 ) << " **** " << (  *items )->url().directory() << "***** " << m_spitem->path << endl;
		if ( (  *items )->url().directory() != m_spitem->path )
		{
			stringList.append ( ( *items )->name() );
			outsideList.append ( ( *items ) );
		}
	}

	progressBar->setTotalSteps ( outsideList.count() + importView->items()->count() );

	if ( outsideList.count() > 0 )
	{
		if ( KMessageBox::questionYesNoList ( this, i18n (
																		"The following file(s) are not in the Subproject directory!\n"
																		"Press Link to add those files by creating symbolic links.\n"
																		"Press Copy to copy those files into the directory." ),
																		stringList, "Warning", KGuiItem ( i18n ( "Link (recommended)" ) ), KGuiItem ( i18n ( "Copy (unrecommended)" ) ) ) == KMessageBox::No )
		{
			// Copy files into the Subproject directory
			KFileItemListIterator it ( outsideList ) ;

			for ( ; it.current(); ++it )
			{
				KShellProcess proc("/bin/sh");

				proc << "cp";
				proc << ( *it )->url().path();
				proc << m_spitem->path;
				proc.start(KProcess::DontCare);

				progressBar->setValue ( progressBar->value() + 1 );
			}
		}
		else
		{
			// Link them into the Subproject directory
			KFileItemListIterator it ( outsideList ) ;

			for ( ; it.current(); ++it )
			{
				KShellProcess proc("/bin/sh");

				proc << "ln -s";
				proc << ( *it )->url().path();
				proc << m_spitem->path;
				proc.start(KProcess::DontCare);

				progressBar->setValue ( progressBar->value() + 1 );
			}
		}
	}

	items.toFirst();

	QString canontargetname = AutoProjectTool::canonicalize ( m_titem->name );
	QString varname = canontargetname + "_SOURCES";
	QMap<QString,QString> replaceMap;
	FileItem* fitem = 0L;

	for ( ; items.current(); ++items )
	{
		m_spitem->variables [ varname ] += ( " " + ( *items )->name() );
		replaceMap.insert ( varname, m_spitem->variables [ varname ] );

		fitem = m_widget->createFileItem ( ( *items )->name() );
		m_titem->sources.append ( fitem );
		m_titem->insertItem ( fitem );

		m_widget->emitAddedFile ( m_spitem->path + "/" + ( *items )->name() );

		progressBar->setValue ( progressBar->value() + 1 );
	}

	AutoProjectTool::modifyMakefileam ( m_spitem->path + "/Makefile.am", replaceMap );

	QDialog::accept();

}

void ImportExistingDialog::slotHelp()
{

}

void ImportExistingDialog::slotAddSelected()
{
	KFileItemListIterator it ( *sourceSelector->dirOperator()->selectedItems() );

	for ( ; it.current(); ++it )
	{
 		if ( ( *it )->url().isLocalFile() ) // maybe unnecessary
 		{
			m_importList.append ( ( *it ) );
		}
	}

	importItems();
}


void ImportExistingDialog::slotAddAll()
{
	KFileItemListIterator it ( *sourceSelector->dirOperator()->view()->items() );

	for ( ; it.current(); ++it )
	{
 		if ( ( *it )->url().isLocalFile() ) // maybe unnecessary
 		{
			m_importList.append ( ( *it ) );
		}
	}
	
	importItems();
}

void ImportExistingDialog::slotRemoveAll()
{
	KURL::List deletedFiles;
	KFileItemListIterator it ( *importView->items() );

	for ( ; it.current(); ++it )
	{
		kdDebug ( 9000 ) << "ImportExistingDialog::slotRemoveAll()" << endl;
		//deletedFiles.append ( ( *it )->url() );
		if ( (*it ) ) importView->removeItem ( *it );
	}

	importView->somethingDropped ( false );

	importView->viewport()->update();
}

void ImportExistingDialog::slotRemoveSelected()
{
	KFileItemListIterator items ( *importView->items() );

	KFileItemList* selectedList = (KFileItemList*) importView->selectedItems();

	KFileItem * deleteItem = 0L;

	for ( ; items.current(); ++items )
	{
		deleteItem = selectedList->first();

		while ( deleteItem )
		{
			if ( deleteItem == ( *items ) )
			{
				importView->removeItem ( deleteItem );
				deleteItem = selectedList->current();
			}
			else
			{
				deleteItem = selectedList->next();
			}
		}
	}

	if ( importView->items()->count() == 0 ) importView->somethingDropped ( false );

	importView->viewport()->update();
}


void ImportExistingDialog::slotDropped ( KFileView* view, QDropEvent* ev )
{
    kdDebug ( 9000 ) << "ImportExistingDialog::dropped()" << endl;

	KURL::List urls;

	KURLDrag::decode( ev, urls );
	
	KFileItem* item = 0L;
	KMimeType::Ptr type = 0L;


    for ( KURL::List::Iterator it = urls.begin(); it != urls.end(); ++it )
    {
 		if ( ( *it ).isLocalFile() ) // maybe unnecessary
 		{
			type = KMimeType::findByURL ( ( *it ).url() );
			
			if ( type->name() != KMimeType::defaultMimeType() )
			{
				item = new KFileItem ( ( *it ).url() , type->name(), 0 );
			}
			else
			{
				// take a text-file-icon instead of the ugly question-mark-icon
				item = new KFileItem ( ( *it ).url(), "text/plain", 0 );
			}

			m_importList.append ( item );
		}
    }

	importItems();
}

#include "importexistingdlg.moc"
