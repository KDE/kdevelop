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

#include "filesynchronizemessage.h"
#include "dynamictext/dynamictext.h"
#include "utils.h"
#include "qtserialization.h"

FileSynchronizeData::FileSynchronizeData( QString fileName, const QDynamicText& text, bool sendDynamic ) {
  if( sendDynamic ) {
    m_fileText = ~text.text().text();
  } else {
    m_text = new QDynamicText( text );
  }
  m_state = text.state();
  m_fileName = fileName;
}

QDynamicTextPointer FileSynchronizeData::createDynamicText() {
  if( m_text )
    return m_text;
  else
    return new QDynamicText( m_state, ~m_fileText );
}

EASY_IMPLEMENT_MESSAGE( FileSynchronize );

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
