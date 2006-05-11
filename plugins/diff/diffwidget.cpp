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

#include <QLayout>
#include <q3textedit.h>
#include <q3popupmenu.h>
#include <qcursor.h>
#include <QFile>
//Added by qt3to4:
#include <QContextMenuEvent>
#include <QVBoxLayout>
#include <QTextStream>

#include <kconfig.h>
#include <kapplication.h>
#include <klocale.h>
#include <kservice.h>
#include <ktempfile.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include <kparts/componentfactory.h>
#include <kparts/part.h>

#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kglobal.h>

#include "diffwidget.h"

// yup, magic value for the popupmenu-id
static const int POPUP_BASE = 130977;

QStringList KDiffTextEdit::extParts;
QStringList KDiffTextEdit::extPartsTranslated;

KDiffTextEdit::KDiffTextEdit( QWidget* parent, const char* name ): Q3TextEdit( parent, name )
{
  KConfig* config = KGlobal::config();
  config->setGroup( "Diff" );
  _highlight = config->readBoolEntry( "Highlight", true );

  searchExtParts();
}

KDiffTextEdit::~KDiffTextEdit()
{
  KConfig* config = KGlobal::config();

  config->setGroup( "Diff" );
  config->writeEntry( "Highlight", _highlight );
}

Q3PopupMenu* KDiffTextEdit::createPopupMenu()
{
  return createPopupMenu( QPoint() );
}

Q3PopupMenu* KDiffTextEdit::createPopupMenu( const QPoint& p )
{
  Q3PopupMenu* popup = Q3TextEdit::createPopupMenu( p );
  if ( !popup )
    popup = new Q3PopupMenu( this );

  int i = 0;

  for ( QStringList::Iterator it = extPartsTranslated.begin(); it != extPartsTranslated.end(); ++it ) {
    popup->insertItem( i18n( "Show in %1", *it ), i + POPUP_BASE, i );
    i++;
  }
  if ( !extPartsTranslated.isEmpty() )
    popup->insertSeparator( i );
  connect( popup, SIGNAL(activated(int)), this, SLOT(popupActivated(int)) );

  popup->insertItem( SmallIconSet( "filesaveas" ), i18n( "&Save As..." ), this, SLOT(saveAs()), Qt::CTRL + Qt::Key_S, POPUP_BASE - 2, 0 );
  popup->setItemEnabled( POPUP_BASE - 2, length() > 0 );

  popup->insertSeparator( 1 );

  popup->insertItem( i18n( "Highlight Syntax" ), this, SLOT(toggleSyntaxHighlight()), 0, POPUP_BASE - 1, 2 );
  popup->setItemChecked( POPUP_BASE - 1, _highlight );
  popup->insertSeparator( 3 );

  return popup;
}

void KDiffTextEdit::saveAs()
{
  QString fName = KFileDialog::getSaveFileName();
  if ( fName.isEmpty() )
    return;

  QFile f( fName );
  if ( f.open( QIODevice::WriteOnly ) ) {
    QTextStream stream( &f );
    int pCount = paragraphs();
    for ( int i = 0; i < pCount; ++i )
      stream << text( i ) << "\n";
    f.close();
  } else {
    KMessageBox::sorry( 0, i18n("Unable to open file."), i18n("Diff Frontend") );
  }
}

void KDiffTextEdit::toggleSyntaxHighlight()
{
  _highlight = !_highlight;
  if ( _highlight )
    applySyntaxHighlight();
  else
    clearSyntaxHighlight();
}

void KDiffTextEdit::applySyntaxHighlight()
{
  // the diff has been loaded so we apply a simple highlighting
  static QColor cAdded( 190, 190, 237);
  static QColor cRemoved( 190, 237, 190 );

  if ( !_highlight )
    return;

  int paragCount = paragraphs();
  for ( int i = 0; i < paragCount; ++i ) {
    QString txt = text( i );
    if ( txt.length() > 0 ) {
      if ( txt.startsWith( "+" ) || txt.startsWith( ">" ) ) {
        setParagraphBackgroundColor( i, cAdded );
      } else if ( txt.startsWith( "-" ) || txt.startsWith( "<" ) ) {
        setParagraphBackgroundColor( i, cRemoved );
      }
    }
  }
}

void KDiffTextEdit::clearSyntaxHighlight()
{
  int paragCount = paragraphs();
  for ( int i = 0; i < paragCount; ++i ) {
    clearParagraphBackground( i );
  }
}

void KDiffTextEdit::searchExtParts()
{
  // only execute once
  static bool init = false;
  if ( init )
    return;
  init = true;

  // search all parts that can handle text/x-diff
  KTrader::OfferList offers = KTrader::self()->query("text/x-diff", "('KParts/ReadOnlyPart' in ServiceTypes) and ('text/x-diff' in ServiceTypes) and (DesktopEntryName != 'katepart')");
  KTrader::OfferList::const_iterator it;
  for ( it = offers.begin(); it != offers.end(); ++it ) {
    KService::Ptr ptr = (*it);
    extPartsTranslated << ptr->name();
    extParts << ptr->desktopEntryName();
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

DiffWidget::DiffWidget( QWidget *parent, const char *name, Qt::WFlags f ):
    QWidget( parent, name, f ), tempFile( 0 )
{
  job = 0;
  extPart = 0;

  te = new KDiffTextEdit( this, "Main Diff Viewer" );
  te->setReadOnly( true );
  te->setTextFormat( Qt::PlainText );
//  te->setMinimumSize( 300, 200 );
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

  KService::Ptr extService = KService::serviceByDesktopName( partName );
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
  rawDiff = QString();
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
  if ( extPart->openStream( "text/plain", KUrl() ) ) {
    for ( int i = 0; i < paragCount; ++i )
      extPart->writeStream( rawDiff.local8Bit() );
    ok = extPart->closeStream();
  } else {
      // workaround for parts that cannot handle streams
      delete tempFile;
      tempFile = new KTempFile();
      tempFile->setAutoDelete( true );
        *(tempFile->textStream()) << rawDiff.local8Bit() << endl;
      tempFile->close();
      ok = extPart->openURL( KUrl::fromPathOrURL( tempFile->name() ) );
  }
  if ( !ok )
    setExtPartVisible( false );
}

// internally for the TextEdit only!
void DiffWidget::slotFinished()
{
    te->applySyntaxHighlight();
    populateExtPart();
}

void DiffWidget::setDiff( const QString& diff )
{
  slotClear();
  rawDiff = diff;
  slotAppend( diff );
  slotFinished();
}

void DiffWidget::openURL( const KUrl& url )
{
  if ( job )
    job->kill();

  KIO::TransferJob* job = KIO::get( url );
  if ( !job )
    return;

  connect( job, SIGNAL(data( KIO::Job *, const QByteArray & )),
           this, SLOT(slotAppend( KIO::Job*, const QByteArray& )) );
  connect( job, SIGNAL(result( KJob * )),
           this, SLOT(slotFinished()) );
}

void DiffWidget::contextMenuEvent( QContextMenuEvent* /* e */ )
{
  Q3PopupMenu* popup = new Q3PopupMenu( this );

  if ( !te->isVisible() )
    popup->insertItem( i18n("Display &Raw Output"), this, SLOT(showTextEdit()) );

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
