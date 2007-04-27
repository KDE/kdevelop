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

#include "network/serialization.h"
#include "qdynamictext.h"

QDynamicText::QDynamicText( const VectorTimestamp& initialState, const std::string& initialText ) : DynamicText( initialState, initialText ) {
}

void QDynamicText::notifyInserted( const ReplacementPointer& rep ) {
  emit inserted( rep, *this );
}

void QDynamicText::notifyStateChanged() {
  emit stateChanged( *this );
}

QDynamicText::QDynamicText( const QDynamicText& rhs ) : QObject(), DynamicText( rhs ) {
}

QDynamicText& QDynamicText::operator = ( const QDynamicText& rhs ) {
  DynamicText::operator=( rhs );
  return *this;
}

BOOST_CLASS_EXPORT_GUID(QDynamicText, "QDynamicText");


#include "qdynamictext.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
