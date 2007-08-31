/***************************************************************************
Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "vectortimestampeditor.h"
#include <QStandardItemModel>
#include <QModelIndex>
#include <QVBoxLayout>
#include <kdialog.h>
#include "qdynamictext.h"
#include "verify.h"
#include "utils.h"

std::ostream& operator << ( std::ostream& o, const SimpleReplacement& rhs );

VectorTimestampEditorLogger::VectorTimestampEditorLogger( VectorTimestampEditor* ed ) : m_editor(ed) {
}

void VectorTimestampEditorLogger::log( const std::string& str , Level lv ) {
  m_editor->log( ~str, (LogLevel)lv );
}

TimestampEditor::TimestampEditor( Teamwork::LoggerPointer logger, VectorTimestampEditor* parent, uint index ) : SafeLogger( logger, ~QString("TimestampEditor %1: ").arg( index ) ), m_parent( parent ), m_index( index ), m_currentStamp( 0 ), m_block( false ) {
  QWidget* w = new QWidget( parent->m_widgets.timestampsGroup );
  m_widgets.setupUi( w );
  connect( m_widgets.stamp, SIGNAL( valueChanged(int) ), this, SLOT( stampChanged() ) );
  connect( m_widgets.date, SIGNAL( dateTimeChanged( const QDateTime& ) ), this, SLOT( dateChanged( const QDateTime& ) ) );
  connect( m_widgets.tail, SIGNAL( clicked( bool ) ), this, SLOT( tail() ) );
  connect( m_widgets.none, SIGNAL( clicked( bool ) ), this, SLOT( none() ) );
  connect( m_widgets.enableAll, SIGNAL( clicked( bool ) ), this, SLOT( enableAll() ) );
  connect( m_widgets.enableBox, SIGNAL( toggled( bool ) ), this, SLOT( enableChanged( bool ) ) );
  fillWidgets();
  m_parent->m_layout->addWidget( w );
}

ReplacementPointer TimestampEditor::replacement() {
  return m_parent->text()->firstUnapplied( m_index );
}

Timestamp TimestampEditor::stamp() {
  return m_parent->text()->state()[m_index] + 1;
}

bool TimestampEditor::trySeekTo( Timestamp s ) {
  VectorTimestamp state = m_parent->text()->state();
  state.setStamp( m_index, s ); state.setPrimaryIndex( m_index );
  try {
  return m_parent->text()->changeState( state );
  } catch( const DynamicTextError& err ) {
    m_parent->log( "TimestampEditor::trySeekTo( " + ~toText( s ) + " ) error: " + ~err.what(), Error );
    return false;
  }
}

void TimestampEditor::fillWidgets() {
  QDynamicTextPointer t = m_parent->text();

  ReplacementPointer p = replacement();

  {
    Block b( m_block );
    m_widgets.stamp->setValue( stamp() );
  }

  if( p ) {
    {
      Block b( m_block );
      m_widgets.enableBox->setChecked( p->enabled() );
    }
    m_widgets.replacementText->setText( ~toText( p->replacement() ) );
    m_widgets.tail->setEnabled( true );
  } else {
    m_widgets.replacementText->setText( "" );
    m_widgets.tail->setEnabled( false );
  }

  m_widgets.stampCount->setText( QString("(%1)").arg( t->highestStamp( m_index ) ) );
  m_currentStamp = stamp();

  if( m_currentStamp == 1 ) {
    m_widgets.none->setEnabled( false );
  } else {
    m_widgets.none->setEnabled( true );
  }

  uint disabledCount = 0;
  ///Now count all disabled replacements lower than the current together
  ReplacementPointer p2 = t->first( m_index );
  while( p2 != p && p2 != 0 ) {
    if( !p2->enabled() ) disabledCount++;

    p2 = p2->next();
  }

  m_widgets.enableAll->setText( QString( "Enable Disabled (%1)" ).arg( disabledCount ) );
  if( disabledCount == 0 )
    m_widgets.enableAll->hide();
  else
    m_widgets.enableAll->show();
}

void TimestampEditor::update() {
  if( m_currentStamp != stamp() ) fillWidgets();
}

void TimestampEditor::tail() {
  QDynamicTextPointer t = m_parent->text();
  ReplacementPointer p = t->last( m_index );
  if( p ) {
    if( !trySeekTo( p->primaryStamp() ) )
      m_parent->log( "could not seek to current state", Error );
  }

  fillWidgets();
}

void TimestampEditor::none() {
  QDynamicTextPointer t = m_parent->text();
  if( !trySeekTo( t->initialState()[m_index] ) )
    m_parent->log( "could not seek to initial state", Error );

  fillWidgets();
}

void TimestampEditor::enableAll() {
  QDynamicTextPointer t = m_parent->text();

  VectorTimestamp cur = t->state();

  ReplacementPointer p = replacement();

  ReplacementPointer p2 = t->first( m_index );
  if( !p2 ) {
    m_parent->log( "enableAll: error", Error );
    return;
  }

  ///Find the lowest disabled stamp
  Timestamp lowest = stamp();

  {
    ReplacementPointer cur = t->lastApplied( m_index );
    while( cur != 0 ) {
      if( lowest > cur->primaryStamp() ) lowest = cur->primaryStamp();
      cur = cur->prev();
    }
  }

  if( lowest == stamp() ) return;

  if( !trySeekTo( lowest - 1 ) ) {
    m_parent->log( QString( "enableAll: could not seek to bottom-state %1").arg( lowest-1 ), Error );
    fillWidgets();
    return;
  }

  while( p2 != p && p2 != 0 ) {
    if( !p2->enabled() ) p2->setEnabled( true );

    p2 = p2->next();
  }

  if( !trySeekTo( cur[m_index] ) ) {
    m_parent->log( "enableAll: seek back failed", Error );
  }

  fillWidgets();
}

void TimestampEditor::stampChanged() {
  if( m_block ) return;
  Timestamp t = m_widgets.stamp->value();
  if( t != 0 ) t-=1;

  Timestamp maxStamp = m_parent->text()->highestStamp( m_index );
  if( t > maxStamp ) t = maxStamp;

  if( !trySeekTo( t ) )
    m_parent->log( "TimestampEditor::TimestampEditor::stampChanged(): trySeekTo failed", Error );

  fillWidgets();
}

void TimestampEditor::dateChanged( const QDateTime& ) {
}

void TimestampEditor::enableChanged( bool enabled ) {
  ReplacementPointer r = replacement();
  try {
  ReplacementPointer firstUnapplied = m_parent->text()->firstUnapplied( m_index );
  if( r && firstUnapplied && (*firstUnapplied) <= (*r) ) {
    r->setEnabled( enabled );
  }
  } catch( const DynamicTextError& err ) {
    m_parent->log( "TimestampEditor::enableChanged(..): serious corruption-error: " + ~err.what(), Error );
  }

  fillWidgets();
}


VectorTimestampEditor::VectorTimestampEditor( Teamwork::LoggerPointer logger, QObject* parent, bool embed, QDynamicTextPointer text , bool editable ) : QObject( parent ), SafeLogger( logger, "VectorTimestampEditor: " ), m_text( text ), m_editable( editable ), m_embed( embed ), m_layout( 0 ), m_dialog( 0 ) {
  QWidget* widget;
  if( embed && qobject_cast<QWidget*>( parent ) )
  {
    widget = qobject_cast<QWidget*>( parent );
    if( !widget ) {
      err() << "parent is no QWidget";
      deleteLater();
      return;
    }
  } else {
    m_dialog = new KDialog;
    m_dialog->setButtons( KDialog::Close );
    m_dialog->setAttribute( Qt::WA_DeleteOnClose, true );
    widget = m_dialog->mainWidget();
    connect( m_dialog, SIGNAL( closeClicked() ), this, SLOT( finish() ) );
  }
  m_widgets.setupUi( widget );
  m_layout = new QVBoxLayout( m_widgets.timestampsGroup );
  m_widgets.timestampsGroup->setLayout( m_layout );
  widget->show();

  m_logModel = new QStandardItemModel( m_widgets.log );
  m_widgets.log->setModel( m_logModel );
  m_logModel->insertColumn( 0 );
  m_logModel->insertRow( 0 );
  m_logModel->setData( m_logModel->index( 0, 0 ), "welcome", Qt::DisplayRole );

  connect( m_widgets.toTailTimestamp, SIGNAL( clicked( bool ) ), this, SLOT( toTailTimestamp() ) );
  connect( m_widgets.clearLog, SIGNAL( clicked( bool ) ), this, SLOT( clearLog() ) );
  connect( text.data(), SIGNAL( stateChanged( QDynamicText& ) ), this, SLOT( textChanged() ) );
  connect( text.data(), SIGNAL( inserted( const ReplacementPointer&, QDynamicText& ) ), this, SLOT( textChanged() ) );
  connect( m_text, SIGNAL( destroyed( QObject* ) ), this, SLOT( textDestroyed() ) );
  connect( widget, SIGNAL( destroyed( QObject* ) ), this, SLOT( finish() ) );

  fillWidgets();
}

VectorTimestampEditor::~VectorTimestampEditor() {
}

void VectorTimestampEditor::fillWidgets() {
  uint cnt = m_timestamps.count();
  uint sz = m_text->state().size();
  if( cnt < sz ) {  ///Add missing widgets
    for( uint a = 0; a < sz - cnt; a++ ) {
      m_timestamps << new TimestampEditor( new VectorTimestampEditorLogger( this ), this, a+cnt );
    }
  }

  for( QList< TimestampEditorPointer >::iterator it = m_timestamps.begin(); it != m_timestamps.end(); ++it ) {
    (*it)->update();
  }

  VectorTimestamp state = m_text->state();
  VectorTimestamp tailState = m_text->tailState();

  QString txt = QString( "Current State: %1 Tail-State: %2" ).arg( ~toText( state ) ).arg( ~toText( tailState ) );
  m_widgets.stateInfo->setText( txt );

  if( state == tailState ) {
    m_widgets.toTailTimestamp->setEnabled( false );
  } else {
    m_widgets.toTailTimestamp->setEnabled( true );
  }
}

void VectorTimestampEditor::log( const QString& str, LogLevel level ) {
  m_logModel->insertRow( 0 );
  QModelIndex i = m_logModel->index( 0, 0 );
  if( i.isValid() ) {
    m_logModel->setData( i, str, Qt::DisplayRole );
    m_logModel->setData( i, iconFromLevel( level ), Qt::DecorationRole );
  }
}

void VectorTimestampEditor::textDestroyed() {
  m_text = 0;
  finish();
}

void VectorTimestampEditor::textChanged() {
  fillWidgets();
}

QDynamicTextPointer VectorTimestampEditor::text() {
  return m_text;
}

void VectorTimestampEditor::toTailTimestamp() {
  try {
    m_text->changeState();
  } catch( const DynamicTextError& error ) {
    log( "error in toTailTimestamp: " + ~error.what(), Error );
  }
}

void VectorTimestampEditor::clearLog() {
  m_logModel->clear();
  m_logModel->insertColumn( 0 );
}

void VectorTimestampEditor::finish() {
  if( m_text ) m_text->changeState();
  if( m_dialog ) m_dialog->deleteLater();
  deleteLater();
}



#include "vectortimestampeditor.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
