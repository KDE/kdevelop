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
#include <qlabel.h>

#include <klineedit.h>
#include <klocale.h>
#include <kservice.h>
 
#include <kparts/factory.h>
#include <kparts/part.h>

#include <kio/jobclasses.h>
#include <kio/job.h>

#include "diffdlg.h"

DiffDlg::DiffDlg( QWidget *parent, const char *name ):
    KDialogBase( parent, name, true, i18n("Difference Viewer"), Ok )
{
  job = 0;
  komparePart = 0;

  QWidget* w = new QWidget( this, "Main Widget" );
  setMainWidget( w );

  loadKomparePart( w );

  te = new QTextEdit( w, "Main Text Edit" );
  te->setReadOnly( true );
  te->setMinimumSize( 300, 200 );

  QVBoxLayout* layout = new QVBoxLayout( w, 0, spacingHint() );
  layout->addWidget( te );

  if ( komparePart ) {
    // if compare is installed, we take it instead of the QTextEdit
    te->hide();
    layout->addWidget( komparePart->widget() );
  } else {
    QLabel* lbl = new QLabel( i18n("<i>Note:</i> Please install Kompare from the KDE SDK package "
                                   "to get a more powerful viewer."), w );
    layout->addWidget( lbl );
  }
}

DiffDlg::~DiffDlg()
{
}

void DiffDlg::loadKomparePart( QWidget* parent )
{
  if ( komparePart )
    return;

  KService::Ptr kompareService = KService::serviceByDesktopName( "komparepart" );
  if ( !kompareService )
    return;

  KParts::Factory* factory = static_cast<KParts::Factory*>(KLibLoader::self()->factory( kompareService->library()
));
  if ( !factory )
    return;

  KParts::ReadOnlyPart* part = static_cast<KParts::ReadOnlyPart*>(factory->createPart( parent, 0, this, 0, "KParts::ReadOnlyPart" ));
  if ( !part ) 
    return;

  komparePart = part;
  return;
}

void DiffDlg::slotClear()
{
  te->clear();
}

void DiffDlg::slotAppend( const QString& str )
{
  te->append( QStyleSheet::escape( str ) );
}

void DiffDlg::slotAppend( KIO::Job*, const QByteArray& ba )
{
  slotAppend( QString( ba ) );
}

void DiffDlg::slotFinished()
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

void DiffDlg::openURL( const KURL& url )
{
  if ( komparePart ) {
    komparePart->openURL( url );
    return;
  }

  if ( job )
    job->kill();

  KIO::TransferJob* job = KIO::get( "/home/harry/temp/coutputwidget.patch" );
  if ( !job )
    return;

  connect( job, SIGNAL(data( KIO::Job *, const QByteArray &)),
           this, SLOT(slotAppend( KIO::Job*, const QByteArray& )) );
  connect( job, SIGNAL(result( KIO::Job * )),
           this, SLOT(slotFinished()) );  
}

#include "diffdlg.moc"
