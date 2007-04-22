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
#ifndef FILESYNCHRONIZEMESSAGE_H
#define FILESYNCHRONIZEMESSAGE_H

#include "filecollaborationmessages.h"
#include "qdynamictext.h"

class DynamicText;

class FileSynchronizeData {

  public:
    FileSynchronizeData( QString fileName = "", const QDynamicText& text = QDynamicText(), bool sendDynamic = true );

    template <class Archive>
    void serialize( Archive& arch, unsigned int /*version*/ ) {
      try {
      arch & m_text;
      }
      catch( const DynamicTextError& err ) {
        throw Teamwork::NonFatalSerializationError( "FileSynchronizeData::serialize(): " + err.what() );
      }
      arch & m_fileName;
      arch & m_fileText;
      arch & m_state;
    }

    QString fileName() {
      return m_fileName;
    }

    VectorTimestamp state() {
      return m_state;
    }

    QDynamicTextPointer createDynamicText();
  private:
    QString m_fileText;
    QString m_fileName;
    VectorTimestamp m_state;
    SharedPtr<QDynamicText, BoostSerializationNormal> m_text;
};

EASY_DECLARE_MESSAGE( FileSynchronize, DocumentWrapperMessage, 3, FileSynchronizeData, 3 );

BOOST_CLASS_IMPLEMENTATION( FileSynchronizeData, boost::serialization::object_serializable );

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
///Eventually think about sending a whole history with a FileSynchronize

#endif
