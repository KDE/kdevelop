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

#ifndef QDYNAMICTEXT_H
#define QDYNAMICTEXT_H

#include <QObject>
#include <boost/serialization/base_object.hpp>
#include "dynamictext/dynamictext.h"
#include <boost/serialization/export.hpp>

class QDynamicText : public QObject, public DynamicText {
    Q_OBJECT;
  public:
    QDynamicText( const VectorTimestamp& initialState = VectorTimestamp(), const std::string& initialText = std::string() );

    QDynamicText( const QDynamicText& rhs );

    QDynamicText& operator = ( const QDynamicText& rhs );

    template<class Archive>
    void serialize( Archive& arch, const unsigned int ) {
      arch & boost::serialization::base_object<DynamicText>(*this);
    }

  signals:
    void stateChanged( QDynamicText& text );
    void inserted( const ReplacementPointer& replacement, QDynamicText& text );
  private:
    virtual void notifyInserted( const ReplacementPointer& rep );
    virtual void notifyStateChanged();
};

typedef SharedPtr<QDynamicText, BoostSerializationNormal> QDynamicTextPointer;


#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
