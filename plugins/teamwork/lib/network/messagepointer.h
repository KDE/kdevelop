#ifndef MESSAGEPOINTER_H
#define MESSAGEPOINTER_H

///This header defines a normal SafeSharedPtr wich uses the message-serialization-system and the MessageTypeSet to serialize and deserialize messages(it stores and loads the messages as vectors). It automatically uses the global message-set stored in globalTypeSet. A hard lock is done while saving.

#include "common.h"
#include "message.h"
#include "pointer.h"
#include "helpers.h"
#include  <vector>



template <class Item>
class SafeSharedMessagePtr : public SafeSharedPtr<Item> {
  public:
    typedef SharedPtr<Item> Precursor;
    typedef LockedSharedPtr<Item> Locked;
    inline SafeSharedMessagePtr( const SharedPtr<Item>& i = 0 ) : SafeSharedPtr<Item>( i ) {}
    ;

    inline SafeSharedMessagePtr( Item* i ) : SafeSharedPtr<Item>( i ) {}
    ;

    inline SafeSharedMessagePtr( const SafeSharedPtr<Item>& rhs ) : SafeSharedPtr<Item>( rhs ) {}

    inline SafeSharedMessagePtr( const SafeSharedMessagePtr<Item>& rhs ) : SafeSharedPtr<Item>( rhs ) {}

    inline SafeSharedMessagePtr( SafeSharedPtr<Item>& rhs ) : SafeSharedPtr<Item>( rhs ) {}

    inline SafeSharedMessagePtr( LockedSharedPtr<Item>& rhs ) : SafeSharedPtr<Item>( rhs.data() ) {}

    template <class Archive>
    void load( Archive& arch, uint /*version*/ ) {
      bool b = false;
      arch >> boost::serialization::make_nvp( "valid", b );
      if ( b ) {
        std::vector<char> v;
        arch & v;
        *this = ( ( MessagePointer ) buildMessageFromBuffer( v, globalTypeSet, 0 ) ).cast<Item>();
      } else {
        *this = 0;
      }
    }

    inline operator Locked() const {
      //if( SafeSharedLockTimeout ) {
      return SafeSharedPtr<Item>::operator Locked();
      /*} else {
      return Locked(  *(Precursor*)this );
      }*/
    }

    template <class Archive>
    void save( Archive& arch, uint /*version*/ ) const {
      if ( this->data() ) {
        Locked l = this->lock ()
                   ;
        bool b = true;
        arch << boost::serialization::make_nvp( "valid", b );
        std::vector<char> v;
        serializeMessageToBuffer( v, *l );
        arch & v;
      } else {
        bool b = false;
        arch << boost::serialization::make_nvp( "valid", b );
      }
    }

    inline SafeSharedMessagePtr<Item>& operator =( const SafeSharedMessagePtr<Item>& rhs ) {
      SharedPtr<Item>::operator=( rhs );
      return *this;
    };

    BOOST_SERIALIZATION_SPLIT_MEMBER();
};

namespace boost {
namespace serialization {
template <class T >
struct implementation_level< SafeSharedMessagePtr<T> > {
  typedef mpl::integral_c_tag tag;
  typedef mpl::int_< boost::serialization::object_serializable > type;
  BOOST_STATIC_CONSTANT(
    int,
    value = implementation_level::type::value
  );
};
}
};



#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
