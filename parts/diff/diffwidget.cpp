/***************************************************************************
 *   Copyright (C) 2001 by Harald Fernengel                                *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qtextedit.h>
#include <qpopupmenu.h>
#include <qcursor.h>

#include <klocale.h>
#include <kservice.h>
#include <ktempfile.h>
#include <kpopupmenu.h>
 
#include <kparts/componentfactory.h>
#include <kparts/part.h>

#include <kio/jobclasses.h>
#include <kio/job.h>

#include "diffwidget.h"

// yup, magic value for the popupmenu-id
static const int POPUP_BASE = 130977;

QStringList KDiffTextEdit::extParts;

KDiffTextEdit::KDiffTextEdit( QWidget* parent, const char* name ): QTextEdit( parent, name )
{
  searchExtParts();
}

KDiffTextEdit::~KDiffTextEdit()
{
}

QPopupMenu* KDiffTextEdit::createPopupMenu()
{ 
  return createPopupMenu( QPoint() ); 
}

QPopupMenu* KDiffTextEdit::createPopupMenu( const QPoint& p )
{
  QPopupMenu* popup = QTextEdit::createPopupMenu( p );
  if ( extParts.isEmpty() )
    return popup;
  if ( !popup )
    popup = new QPopupMenu( this );

  int i = 0;
  for ( QStringList::Iterator it = extParts.begin(); it != extParts.end(); ++it ) {
    popup->insertItem( i18n( "Show in %1" ).arg( *it ), i + POPUP_BASE, i );
    ++i;
  }
  popup->insertSeparator( i );
  connect( popup, SIGNAL(activated(int)), this, SLOT(popupActivated(int)) );

  return popup;
}

void KDiffTextEdit::searchExtParts()
{
  // only execute once
  static bool init = false;
  if ( init )
    return;
  init = true;
  
  // search all parts that can handle text/x-diff
  KTrader::OfferList offers = KTrader::self()->query("text/x-diff", "'KParts/ReadOnlyPart' in ServiceTypes");
  KTrader::OfferList::const_iterator it;
  for ( it = offers.begin(); it != offers.end(); ++it ) {
    KService::Ptr ptr = (*it);
    extParts << ptr->name();
  }
  return;
}

void KDiffTextEdit::popupActivated( int id )
{
  id -= POPUP_BASE;
  if ( id < 0 || id > (int)extParts.count() )
    return;

  emit externalPartRequested( extParts[ id ] ); 
}

DiffWidget::DiffWidget( QWidget *parent, const char *name, WFlags f ):
    QWidget( parent, name, f ), tempFile( 0 )
{
  job = 0;
  extPart = 0;

  te = new KDiffTextEdit( this, "Main Diff Viewer" );
  te->setReadOnly( true );
  te->setTextFormat( QTextEdit::PlainText );
  te->setMinimumSize( 300, 200 );
  connect( te, SIGNAL(externalPartRequested(const QString&)), this, SLOT(loadExtPart(const QString&)) );

  QVBoxLayout* layout = new QVBoxLayout( this );
  layout->addWidget( te );
}

DiffWidget::~DiffWidget()
{
    delete tempFile;
}

void DiffWidget::setExtPartVisible( bool visible )
{
  if ( !extPart || !extPart->widget() ) {
    te->show();
    return;
  }
  if ( visible ) {
    te->hide();
    extPart->widget()->show();
  } else {
    te->show();
    extPart->widget()->hide();
  }
}

void DiffWidget::loadExtPart( const QString& partName )
{
  if ( extPart ) {
    setExtPartVisible( false );
    delete extPart;
    extPart = 0;
  }

  KService::Ptr extService = KService::serviceByName( partName );
  if ( !extService )
    return;

  extPart = KParts::ComponentFactory::createPartInstanceFromService<KParts::ReadOnlyPart>( extService, this, 0, this, 0 );
  if ( !extPart || !extPart->widget() )
    return;

  layout()->add( extPart->widget() );

  setExtPartVisible( true );

  if ( te->paragraphs() > 0 )
    populateExtPart();
}

void DiffWidget::slotClear()
{
  te->clear();
  if ( extPart )
    extPart->closeURL();
}

// internally for the TextEdit only!
void DiffWidget::slotAppend( const QString& str )
{
  te->append( str );
}

// internally for the TextEdit only!
void DiffWidget::slotAppend( KIO::Job*, const QByteArray& ba )
{
  slotAppend( QString( ba ) );
}

void DiffWidget::populateExtPart()
{
  if ( !extPart )
    return;

  bool ok = false;
  int paragCount = te->paragraphs();
  if ( extPart->openStream( "text/plain", KURL() ) ) {
    for ( int i = 0; i < paragCount; ++i )
      extPart->writeStream( te->text( i ).local8Bit() );
    ok = extPart->closeStream();
  } else {
      // workaround for parts that cannot handle streams
      delete tempFile;
      tempFile = new KTempFile();
      tempFile->setAutoDelete( true );
      for ( int i = 0; i < paragCount; ++i )
        *(tempFile->textStream()) << te->text( i ) << endl;
      tempFile->close();
      ok = extPart->openURL( tempFile->name() );
  }
  if ( !ok )
    setExtPartVisible( false );
}

// internally for the TextEdit only!
void DiffWidget::slotFinished()
{
  // the diff has been loaded so we apply a simple highlighting
  static QColor cAdded( 190, 190, 237);
  static QColor cRemoved( 190, 237, 190 );

  int paragCount = te->paragraphs();
  for ( int i = 0; i < paragCount; ++i ) {
    QString txt = te->text( i );
    if ( txt.length() > 0 ) {
      if ( txt.startsWith( "+" ) || txt.startsWith( ">" ) ) {
        te->setParagraphBackgroundColor( i, cAdded );
      } else if ( txt.startsWith( "-" ) || txt.startsWith( "<" ) ) {
        te->setParagraphBackgroundColor( i, cRemoved );
      }
    }
  }
  populateExtPart();
}

void DiffWidget::setDiff( const QString& diff )
{
  slotClear();
  slotAppend( diff );
  slotFinished();
}

void DiffWidget::openURL( const KURL& url )
{
  if ( job )
    job->kill();

  KIO::TransferJob* job = KIO::get( url );
  if ( !job )
    return;

  connect( job, SIGNAL(data( KIO::Job *, const QByteArray & )),
           this, SLOT(slotAppend( KIO::Job*, const QByteArray& )) );
  connect( job, SIGNAL(result( KIO::Job * )),
           this, SLOT(slotFinished()) );  
}

void DiffWidget::contextMenuEvent( QContextMenuEvent* /* e */ )
{
  QPopupMenu* popup = new QPopupMenu( this );
  
  if ( !te->isVisible() )
    popup->insertItem( i18n("Display &raw output"), this, SLOT(showTextEdit()) );
 
  popup->exec( QCursor::pos() );
  delete popup;
}

void DiffWidget::showExtPart()
{
  setExtPartVisible( true );
}

void DiffWidget::showTextEdit()
{
  setExtPartVisible( false );
}

#include "diffwidget.moc"
