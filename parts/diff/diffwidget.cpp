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

#include <klocale.h>
#include <kservice.h>
#include <ktempfile.h>
 
#include <kparts/componentfactory.h>
#include <kparts/part.h>

#include <kio/jobclasses.h>
#include <kio/job.h>

#include "diffwidget.h"

DiffWidget::DiffWidget( QWidget *parent, const char *name, WFlags f ):
    QWidget( parent, name, f )
{
  job = 0;
  komparePart = 0;

  fileCleanupHandler.setAutoDelete( true );

  loadKomparePart( this );

  te = new QTextEdit( this, "Main Text Edit" );
  te->setReadOnly( true );
  te->setMinimumSize( 300, 200 );

  QVBoxLayout* layout = new QVBoxLayout( this );
  layout->addWidget( te );

  if ( komparePart ) {
    // if compare is installed, we take it instead of the QTextEdit
    te->hide();
    layout->addWidget( komparePart->widget() );
  }
}

DiffWidget::~DiffWidget()
{
  fileCleanupHandler.clear();
}

void DiffWidget::loadKomparePart( QWidget* parent )
{
  if ( komparePart )
    return;

  // ### might be easier to use:
  // createPartInstanceFromQuery( "text/x-diff", QString::null, parent, 0, this, 0 ); (Simon)

  KService::Ptr kompareService = KService::serviceByDesktopName( "komparepart" );
  if ( !kompareService )
    return;

  komparePart = KParts::ComponentFactory::createPartInstanceFromService<KParts::ReadOnlyPart>( kompareService, parent, 0, this, 0 );
}

void DiffWidget::slotClear()
{
  te->clear();
  if ( komparePart )
    komparePart->closeURL();
}

// internally for the TextEdit only!
void DiffWidget::slotAppend( const QString& str )
{
  te->append( QStyleSheet::escape( str ) );
}

// internally for the TextEdit only!
void DiffWidget::slotAppend( KIO::Job*, const QByteArray& ba )
{
  slotAppend( QString( ba ) );
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
}

void DiffWidget::setDiff( const QString& diff )
{
  if ( komparePart ) {
    // workaround until kompare can view patches directly from a QString
    KTempFile* tmpFile = new KTempFile();
    fileCleanupHandler.append( tmpFile ); // make sure it gets erased
    tmpFile->setAutoDelete( true );
    *(tmpFile->textStream()) << diff;
    tmpFile->close();
    openURL( tmpFile->name() );
    return;
  }

  slotAppend( diff );
  slotFinished();
}

void DiffWidget::openURL( const KURL& url )
{
  if ( komparePart ) {
    komparePart->openURL( url );
    return;
  }

  if ( job )
    job->kill();

  KIO::TransferJob* job = KIO::get( url );
  if ( !job )
    return;

  connect( job, SIGNAL(data( KIO::Job *, const QByteArray &)),
           this, SLOT(slotAppend( KIO::Job*, const QByteArray& )) );
  connect( job, SIGNAL(result( KIO::Job * )),
           this, SLOT(slotFinished()) );  
}

#include "diffwidget.moc"
