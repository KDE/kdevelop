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
#include <qregexp.h>

#include <kxmlguiclient.h>
#include <kiconloader.h>
#include <kurlcombobox.h>
#include <kurlcompletion.h>
#include <kprotocolinfo.h>
#include <kconfig.h>
#include <klocale.h>
#include <kcombobox.h>

#include <kdebug.h>

#include "fileselectorwidget.h"
#include <kdiroperator.h>
#include <kcombiview.h>
#include <kfilepreview.h>
#include <kfileview.h>
#include <kfileitem.h>
#include <kimagefilepreview.h>

#include "autoprojectwidget.h"
#include "autoprojectpart.h"
#include "kdevlanguagesupport.h"

#include "kfilednddetailview.h"
#include "kfiledndiconview.h"

KDnDDirOperator::KDnDDirOperator ( const KURL &urlName, QWidget* parent, const char* name ) : KDirOperator ( urlName, parent, name )
{

}

KFileView* KDnDDirOperator::createView( QWidget* parent, KFile::FileView view )
{
    KFileView* new_view = 0L;

    if( (view & KFile::Detail) == KFile::Detail ) {
        new_view = new KFileDnDDetailView( parent, "detail view");
    }
    else if ((view & KFile::Simple) == KFile::Simple ) {
        new_view = new KFileDnDIconView( parent, "simple view");
        new_view->setViewName( i18n("Short View") );
    }

    return new_view;
}


FileSelectorWidget::FileSelectorWidget(AutoProjectPart* part, KFile::Mode mode, QWidget* parent, const char* name ) : QWidget(parent, name)
{
    m_part = part;

	// widgets and layout
	QVBoxLayout* lo = new QVBoxLayout(this);

	QHBox *hlow = new QHBox (this);
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
	QToolTip::add(forward, i18n("Next directory"));

	// HACK
	QWidget* spacer = new QWidget(hlow);
	hlow->setStretchFactor(spacer, 1);
	hlow->setMaximumHeight(up->height());

	cmbPath = new KURLComboBox( KURLComboBox::Directories, true, this, "path combo" );
	cmbPath->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
	KURLCompletion* cmpl = new KURLCompletion();
	cmbPath->setCompletionObject( cmpl );
	lo->addWidget(cmbPath);

	dir = new KDnDDirOperator(KURL(), this, "operator");
	dir->setView(KFile::Simple);
    dir->setMode(mode);

	lo->addWidget(dir);
	lo->setStretchFactor(dir, 2);

	QHBox* filterBox = new QHBox(this);
	filterIcon = new QLabel(filterBox);
	filterIcon->setPixmap( BarIcon("filter") );
	filter = new KHistoryCombo(filterBox, "filter");
	filter->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
	filterBox->setStretchFactor(filter, 2);
	lo->addWidget(filterBox);

	// slots and signals
	connect( filter, SIGNAL( activated(const QString&) ), SLOT( slotFilterChanged(const QString&) ) );
	connect( filter, SIGNAL( returnPressed(const QString&) ), SLOT( filterReturnPressed(const QString&) ) );

	connect( home, SIGNAL( clicked() ), dir, SLOT( home() ) );
	connect( up, SIGNAL( clicked() ), dir, SLOT( cdUp() ) );
	connect( back, SIGNAL( clicked() ), dir, SLOT( back() ) );
	connect( forward, SIGNAL( clicked() ), dir, SLOT( forward() ) );

	connect( cmbPath, SIGNAL( urlActivated( const KURL&  )), this,  SLOT( cmbPathActivated( const KURL& ) ));
	connect( cmbPath, SIGNAL( returnPressed( const QString&  )), this,  SLOT( cmbPathReturnPressed( const QString& ) ));
	connect( dir, SIGNAL(urlEntered(const KURL&)), this, SLOT(dirUrlEntered(const KURL&)) );

	connect( dir, SIGNAL(finishedLoading()), this, SLOT(dirFinishedLoading()) );

//    dirUrlEntered( dir->url() );

    QStringList list;

    /* read the file patterns from the project DOM */
    QDomElement docEl = m_part->projectDom()->documentElement();
    QDomElement fileviewEl = docEl.namedItem("kdevfileview").toElement();
    QDomElement groupsEl = fileviewEl.namedItem("groups").toElement();
    QDomElement groupEl = groupsEl.firstChild().toElement();

    while ( !groupEl.isNull() )
    {
        if (groupEl.tagName() == "group")
        {
            list << groupEl.attribute("pattern").replace ( QRegExp ( ";" ), " " ) + " (" + groupEl.attribute("name") + ")";
        }
        groupEl = groupEl.nextSibling().toElement();
    }

    filter->setHistoryItems ( list );

//    dir->rereadDir();
}


FileSelectorWidget::~FileSelectorWidget()
{
}

void FileSelectorWidget::dragEnterEvent ( QDragEnterEvent* /*ev*/ )
{
}

void FileSelectorWidget::dropEvent ( QDropEvent* /*ev*/ )
{
    kdDebug ( 9020 ) << "Dropped" << endl;

    QString path = "Something was dropped in the Destination directory file-selector";

    emit dropped ( path );

}

void FileSelectorWidget::filterReturnPressed ( const QString& nf )
{
    // KURL u ( m_part->project()->projectDirectory() );
    setDir ( nf );
}

void FileSelectorWidget::slotFilterChanged( const QString & nf )
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

void FileSelectorWidget::setDir(const QString& path)
{
    KURL u ( path );
    dir->setURL ( u, true );
}


#include "fileselectorwidget.moc"

