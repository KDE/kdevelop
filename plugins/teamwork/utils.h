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

#ifndef KDEVTEAMWORK_UTILS
#define KDEVTEAMWORK_UTILS

#include <QString>
#include <string>
#include <QVariant>
#include "network/sharedptr.h"
#include "network/basicsession.h"
#include <iostream>
#include <QDataStream>
#include <QTextStream>
#include <QByteArray>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/level.hpp>
#include <boost/serialization/binary_object.hpp>
#include <vector>

QString toQ( const std::string& rhs );
std::string fromQ( const QString& str );

///This operator converts between Q- and std-classes
std::string operator ~ ( const QString& rhs );

QString operator ~ ( const std::string& rhs );

ostream& operator << ( ostream& stream, const QString str );

void indexToLineCol( int index, const QString& text, int& line, int& col );

///Returns -1 if the index does not exist
int lineColToIndex( const QString& text, int line, int col );

void indexToLineCol( int index, const std::string& text, int& line, int& col );

///Returns -1 if the index does not exist
int lineColToIndex( const std::string& text, int line, int col );


#define QSTREAM QTextStream

namespace boost {
namespace serialization {
template <class Archive>
void load( Archive & arch, QByteArray& b, const unsigned int /*version*/ ) {
  uint size;
  arch & size;
  b.resize( size );

  binary_object o( b.data(), b.size() );
  arch & o;
}

template <class Archive>
void save( Archive & arch, const QByteArray& b, const unsigned int /*version*/ ) {
  uint size = b.size();
  arch & size;
  binary_object o( const_cast<QByteArray&>( b ).data(), b.size() );
  arch & o;
}
}
}

BOOST_SERIALIZATION_SPLIT_FREE( QByteArray );

///Usually uses a QDataStream to convert to a binary object, and stores that.
template <class Type>
class QSerialize {
    Type& m_t;
  public:
    QSerialize( const Type& t ) : m_t( const_cast<Type&>( t ) ) {}

    template <class Archive>
    void save( Archive& arch, unsigned int /*version*/ ) const {
      QByteArray b;
      QDataStream s( &b, QIODevice::WriteOnly );
      s << m_t;
      arch & b;
    }

    template <class Archive>
    void load( Archive& arch, unsigned int /*version*/ ) {
      QByteArray b;
      arch & b;

      QDataStream s( &b, QIODevice::ReadOnly );
      s >> m_t;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER();
};

/**This function returns a temporary serialization-object that can be used to serialize Qt-Types using the boost-serialization-library. */
template <class Type>
QSerialize<Type> qStore( const Type& t ) {
  return QSerialize<Type>( t );
}

namespace boost {
namespace serialization {

template <class Archive>
void serialize( Archive & ar, QString& str, const unsigned int /*version*/ ) {
  QSerialize<QString> s( str );
  ar & s;
}

template <class Archive>
void serialize( Archive & ar, QStringList& t, const unsigned int /*version*/ ) {
  QSerialize<QStringList> s( t );
  ar & s;
}

template <class Archive>
void serialize( Archive & ar, QVariant& t, const unsigned int /*version*/ ) {
  QSerialize<QVariant> s( t );
  ar & s;
}
} // namespace serialization
} // namespace

///Tell boost not to store type-information for QSerialize<T>
namespace boost {
namespace serialization {
template <class T>
struct implementation_level< QSerialize<T> > {
  typedef mpl::integral_c_tag tag;
  typedef mpl::int_< object_serializable > type;
  BOOST_STATIC_CONSTANT(
    int,
    value = implementation_level::type::value
  );
};
}
}

struct Block {
  bool& b;
  Block( bool& bl ) : b( bl ) {
    b = true;
  }
  ~Block() {
    b = false;
  }
};

BOOST_CLASS_IMPLEMENTATION(QString, object_serializable);
BOOST_CLASS_IMPLEMENTATION(QStringList, object_serializable);
BOOST_CLASS_IMPLEMENTATION(QVariant, object_serializable);
BOOST_CLASS_IMPLEMENTATION(QByteArray, object_serializable);

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
