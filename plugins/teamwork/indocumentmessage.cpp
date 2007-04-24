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

#include "indocumentmessage.h"
#include "message.h"
#include "kdevteamwork_messageshower.h"

///@todo move some stuff from conversationmanager.cpp here

///Can be used to get and set the context-lines
DocumentContextLines& InDocumentMessage::contextLines() {
  return m_contextLines;
}

InDocumentReference& InDocumentMessage::start() {
  return m_start;
}

InDocumentReference& InDocumentMessage::end() {
  return m_end;
}

QString InDocumentMessage::shortName() const {
  return "Document-Message";
}

void InDocumentMessage::showInWidget( QWidget* widget, KDevTeamwork* tw ) {
  new InDocumentMessageShower( this, widget, tw );
}

REGISTER_MESSAGE( InDocumentMessage );

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
