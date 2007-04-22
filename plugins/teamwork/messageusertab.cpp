/***************************************************************************
copyright            : (C) 2006 by David Nolden
email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QWidget>
#include<QListView>
#include<QStandardItemModel>

#include "messageusertab.h"
#include "kdevteamwork.h"
#include "kdevteamwork_user.h"
#include <QMetaType>

Q_DECLARE_METATYPE( MessagePointer );


MessageUserTab& MessageUserTab::operator = ( const MessageUserTab& /*rhs*/ ) {
  return * this;
}

MessageUserTab::~MessageUserTab() {
  delete m_widget;
}

bool MessageUserTab::operator < ( const MessageUserTab& rhs ) {
  return m_user < rhs.m_user;
}

MessageUserTab::MessageUserTab( KDevTeamwork* tw, KDevTeamworkUserPointer user ) : m_user( user ), m_teamwork( tw ) {
  m_widget = new QWidget;
  m_list->setupUi( m_widget );
  qRegisterMetaType<KDevTeamworkUserPointer>( "KDevTeamworkUserPointer" );
  m_view = m_list->list;
  m_model = new QStandardItemModel( 0, 1, m_widget );
  m_view->setModel( m_model );
  KDevTeamworkUserPointer::Locked l = user;
  if ( l ) {
    tw->m_widgets->messageUsers->addTab( m_widget, l->icon(), ~l->safeName() );
    connect( l.data(), SIGNAL( userStateChanged( KDevTeamworkUserPointer ) ), this, SLOT( userStateChanged( KDevTeamworkUserPointer ) ), Qt::QueuedConnection );
  }
  connect( m_view, SIGNAL( clicked( const QModelIndex& ) ), tw, SLOT( messageClicked( const QModelIndex& ) ) );

  userStateChanged( m_user );
}

void MessageUserTab::userStateChanged( KDevTeamworkUserPointer ) {
  if ( !m_teamwork->m_widget )
    return ;
  int i = m_teamwork->m_widgets->messageUsers->indexOf( m_widget );
  if ( i != -1 ) {
    KDevTeamworkUserPointer::Locked l = m_user;
    if ( l ) {
      m_teamwork->m_widgets->messageUsers->setTabIcon( i, l->icon() );
    }
  }
}

void MessageUserTab::messageUpdated( const Teamwork::MessagePointer& msg ) {
  for ( int a = 0; a < m_model->rowCount(); ++a ) {
    QModelIndex i = m_model->index( a, 0 );
    if ( i.isValid() ) {
      QVariant v = m_model->data( i, Qt::UserRole );
      if ( v.canConvert<MessagePointer>() ) {
        if ( v.value<MessagePointer>() == msg ) {
          MessagePointer::Locked l = msg;
          if ( l ) {
            AbstractGUIMessage * guiMsg = l.freeCast<AbstractGUIMessage>();
            if ( guiMsg ) {
              m_model->setData( i, guiMsg->messageIcon(), Qt::DecorationRole );
            }
          }
        }
      }
    }
  }
}

QStandardItemModel* MessageUserTab::model() {
  return m_model;
}

QListView* MessageUserTab::view() {
  return m_view;
}

KDevTeamworkUserPointer MessageUserTab::user() {
  return m_user;
}

QWidget* MessageUserTab::widget() {
  return m_widget;
}

#include "messageusertab.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

