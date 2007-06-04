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

#include "safesignaller.h"
#include"kdevteamwork.h"
#include "network/weaksafesharedptr.h"

QSafeSignaller::~QSafeSignaller() {
  delete m_proxy;
}

void QSafeSignaller::signal() {
  if(!KDevTeamwork::self())return;
  KDevTeamwork::self()->log( "QSafeSignaller::signal()" );
  m_proxy->signal();
}

QSafeSignaller& QSafeSignaller::operator = ( const QSafeSignaller& /*rhs*/ ) {
  return *this;
}

QSafeSignaller::QSafeSignaller( const QSafeSignaller& /*rhs*/ ) : QObject() {
}

#include "safesignaller.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
