/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdevwidgetaction.h"

#if !(KDE_VERSION > 305)
# include <ktoolbar.h>
# include <kdebug.h>
# include <kapplication.h>
//--------------------begin-of-copy-of-kde-3.1-----------
KWidgetAction::KWidgetAction( QWidget* widget,
    const QString& text, const KShortcut& cut,
    const QObject* receiver, const char* slot,
    KActionCollection* parent, const char* name )
  : KAction( text, cut, receiver, slot, parent, name )
  , m_widget( widget )
  , m_autoSized( false )
{
}

KWidgetAction::~KWidgetAction()
{
}

void KWidgetAction::setAutoSized( bool autoSized )
{
  if( m_autoSized == autoSized )
    return;

  m_autoSized = autoSized;

  if( !m_widget || !isPlugged() )
    return;

  KToolBar* toolBar = (KToolBar*)m_widget->parent();
  int i = findContainer( toolBar );
  if ( i == -1 )
    return;
  int id = itemId( i );

  toolBar->setItemAutoSized( id, m_autoSized );
}

int KWidgetAction::plug( QWidget* w, int index )
{
  if (kapp && !kapp->authorizeKAction(name()))
      return -1;

  if ( !w->inherits( "KToolBar" ) ) {
    kdError() << "KWidgetAction::plug: KWidgetAction must be plugged into KToolBar." << endl;
    return -1;
  }
  if ( !m_widget ) {
    kdError() << "KWidgetAction::plug: Widget was deleted or null!" << endl;
    return -1;
  }

  KToolBar* toolBar = static_cast<KToolBar*>( w );

  int id = KAction::getToolButtonID();

  m_widget->reparent( toolBar, QPoint() );
  toolBar->insertWidget( id, 0, m_widget, index );
  toolBar->setItemAutoSized( id, m_autoSized );

  addContainer( toolBar, id );

  connect( toolBar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

  return containerCount() - 1;
}

void KWidgetAction::unplug( QWidget *w )
{
  // ### shouldn't this method check if w == m_widget->parent() ? (Simon)
  if( !m_widget )
    return;

  m_widget->reparent( 0L, QPoint(), false /*showIt*/ );

  KAction::unplug( w );
}


void KWidgetAction::virtual_hook( int id, void* data )
{ KAction::virtual_hook( id, data ); }

//--------------------end-of-copy-of-kde-3.1-----------

#include "kdevwidgetaction.moc"

#endif // !(KDE_VERSION > 305)

