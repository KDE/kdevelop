/****************************************************************************
 *   Copyright (C) 2001 by Hugo Varotto										*
 *   hugo@varotto-usa.com													*
 *																			*
 *	 Based on Kate's fileselector widget	by									*
 *          Matt Newell														*
 *			(C) 2001 by Matt Newell											*
 *			newellm@proaxis.com												*
 *																			*
 *   This program is free software; you can redistribute it and/or modify		*
 *   it under the terms of the GNU General Public License as published by		*
 *   the Free Software Foundation; either version 2 of the License, or			*
 *   (at your option) any later version										*
 *																			*
 ***************************************************************************/

#include <qlayout.h>
#include <qpushbutton.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qstrlist.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qstringlist.h>

#include <kiconloader.h>
#include <kurlcombobox.h>
#include <kurlcompletion.h>
#include <kprotocolinfo.h>
#include <kdiroperator.h>
#include <kconfig.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kapplication.h>


#include "kdevcore.h"
#include "kdevproject.h"
#include "fileselector_part.h"
#include "fileselectorwidget.h"

#include "kfilednddetailview.h"
#include "kfiledndiconview.h"

KDnDDirOperator::KDnDDirOperator ( const KURL &urlName, QWidget* parent, const char* name ) : KDirOperator ( urlName, parent, name )
{

}

KFileView* KDnDDirOperator::createView( QWidget* parent, KFile::FileView view )
{
    KFileView* new_view = 0L;

    if( (view & KFile::Detail) == KFile::Detail ) {
		kdDebug ( 9000 ) << "Create drag 'n drop enabled detailed view for file selector" << endl;
        new_view = new KFileDnDDetailView( parent, "detail view");
    }
    else if ((view & KFile::Simple) == KFile::Simple ) {
		kdDebug ( 9000 ) << "Create drag 'n drop enabled simple view for file selector" << endl;
        new_view = new KFileDnDIconView( parent, "simple view");
        new_view->setViewName( i18n("Short View") );
    }

    return new_view;
}


FileSelectorWidget::FileSelectorWidget(FileSelectorPart *part)
    : QWidget(0, "file selector widget"), m_popup(0)
{

	// widgets and layout

	QVBoxLayout* lo = new QVBoxLayout(this);

	QHBox *hlow = new QHBox (this);
    hlow->setMargin ( 2 );
    hlow->setSpacing ( 2 );
	lo->addWidget(hlow);

	home = new QPushButton( hlow );
	home->setPixmap(SmallIcon("gohome"));
	QToolTip::add(home, i18n("Home directory"));
	up = new QPushButton( /*i18n("&Up"),*/ hlow );
	up->setPixmap(SmallIcon("up"));
	QToolTip::add(up, i18n("Up one level"));
	back = new QPushButton( /*i18n("&Back"),*/ hlow );
	back->setPixmap(SmallIcon("back"));
	QToolTip::add(back, i18n("Previous directory"));
	forward = new QPushButton( /*i18n("&Next"),*/ hlow );
	forward->setPixmap(SmallIcon("forward"));
	QToolTip::add(forward, i18n("Next Directory"));

	// HACK
	QWidget* spacer = new QWidget(hlow);
	hlow->setStretchFactor(spacer, 1);
	hlow->setMaximumHeight(home->height());

	cmbPath = new KURLComboBox( KURLComboBox::Directories, true, this, "path combo" );
	cmbPath->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
	KURLCompletion* cmpl = new KURLCompletion();
	cmbPath->setCompletionObject( cmpl );
	lo->addWidget(cmbPath);

	dir = new KDnDDirOperator(QString::null, this, "operator");
	dir->setView(KFile::Detail);
	lo->addWidget(dir);
	lo->setStretchFactor(dir, 2);

	QHBox* filterBox = new QHBox(this);
	filterIcon = new QLabel(filterBox);
	filterIcon->setPixmap( BarIcon("filter") );
	filter = new KHistoryCombo(filterBox, "filter");
	filter->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
	QString group = kapp->config()->group();
	kapp->config()->setGroup( "FileSelector Plugin" );
	filter->setHistoryItems( kapp->config()->readListEntry("FilterHistory") );
	kapp->config()->setGroup( group );
	filterBox->setStretchFactor(filter, 2);
	lo->addWidget(filterBox);

	// slots and signals

	connect( filter, SIGNAL( activated(const QString&) ), SLOT( slotFilterChange(const QString&) ) );
	connect( filter, SIGNAL( returnPressed(const QString&) ),filter, SLOT( addToHistory(const QString&) ) );

	connect( home, SIGNAL( clicked() ), dir, SLOT( home() ) );
	connect( up, SIGNAL( clicked() ), dir, SLOT( cdUp() ) );
	connect( back, SIGNAL( clicked() ), dir, SLOT( back() ) );
	connect( forward, SIGNAL( clicked() ), dir, SLOT( forward() ) );

	connect( cmbPath, SIGNAL( urlActivated( const KURL&  )),
				this,  SLOT( cmbPathActivated( const KURL& ) ));
	connect( cmbPath, SIGNAL( returnPressed( const QString&  )),
				this,  SLOT( cmbPathReturnPressed( const QString& ) ));
	connect(dir, SIGNAL(urlEntered(const KURL&)),
				this, SLOT(dirUrlEntered(const KURL&)) );

	connect(dir, SIGNAL(finishedLoading()),
				this, SLOT(dirFinishedLoading()) );


    m_part = part;

  KActionMenu *menu = (KActionMenu*)dir->actionCollection()->action("popupMenu");
  if (menu)
  {
    m_popup = menu->popupMenu();
    if (m_popup)
    {
      connect(m_popup, SIGNAL(aboutToShow()), this, SLOT(popupAboutToShow()));
      connect(m_popup, SIGNAL(aboutToHide()), this, SLOT(popupAboutToHide()));
    }
  }
}


FileSelectorWidget::~FileSelectorWidget()
{
  QString group = kapp->config()->group();
  kapp->config()->setGroup( "FileSelector Plugin" );
  kapp->config()->writeEntry( "FilterHistory", filter->historyItems() );
  kapp->config()->setGroup( group );
}


void FileSelectorWidget::popupAboutToShow()
{
  // ugly hack: mark the "original" items
  m_popupIds.resize(m_popup->count());
  for (uint index=0; index < m_popup->count(); ++index)
    m_popupIds[index] = m_popup->idAt(index);

  KFileItem *item = const_cast<KFileItemList*>(dir->selectedItems())->first();
  if (item)
  {
    FileContext context(item->url().path(), item->isDir());
    m_part->core()->fillContextMenu(m_popup, &context);
  }
}


void FileSelectorWidget::popupAboutToHide()
{
  QTimer::singleShot(0, this, SLOT(popupDelete()));
}


void FileSelectorWidget::popupDelete()
{
  if (!m_popup)
    return;

  // ugly hack: remove all but the "original" items
  for (int index=m_popup->count()-1; index >= 0; --index)
  {
    int id = m_popup->idAt(index);
    if (m_popupIds.contains(id) == 0)
    {
      QMenuItem *item = m_popup->findItem(id);
      if (item->popup())
        delete item->popup();
      m_popup->removeItemAt(index);
    }
  }
}


void FileSelectorWidget::slotFilterChange( const QString & nf )
{
  dir->setNameFilter( nf );
  dir->rereadDir();
}

void FileSelectorWidget::cmbPathActivated( const KURL& u )
{
   dir->setURL( u, true );
}

void FileSelectorWidget::cmbPathReturnPressed( const QString& u )
{
   dir->setFocus();
   dir->setURL( KURL(u), true );
}


void FileSelectorWidget::dirUrlEntered( const KURL& u )
{
   cmbPath->removeURL( u );
   QStringList urls = cmbPath->urls();
   urls.prepend( u.url() );
   while ( urls.count() >= (uint)cmbPath->maxItems() )
      urls.remove( urls.last() );
   cmbPath->setURLs( urls );
}


void FileSelectorWidget::dirFinishedLoading()
{
   // HACK - enable the nav buttons
   // have to wait for diroperator...
   up->setEnabled( dir->actionCollection()->action( "up" )->isEnabled() );
   back->setEnabled( dir->actionCollection()->action( "back" )->isEnabled() );
   forward->setEnabled( dir->actionCollection()->action( "forward" )->isEnabled() );
   home->setEnabled( dir->actionCollection()->action( "home" )->isEnabled() );
}


void FileSelectorWidget::focusInEvent(QFocusEvent*)
{
   dir->setFocus();
}

void FileSelectorWidget::setDir( KURL u )
{
  dir->setURL(u, true);
}

#include "fileselectorwidget.moc"

