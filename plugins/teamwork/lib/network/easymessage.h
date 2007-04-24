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

#ifndef EASYMESSAGE
#define EASYMESSAGE
#include "message.h"
#include  <boost/serialization/base_object.hpp>
#include <boost/serialization/extended_type_info_no_rtti.hpp>

/**
This header offers by far the easiest way of declaring a message.
The Macro
EASY_DECLARE_MESSAGE( Name, Parent, index, Data, paramSplitPosition )
Declares a complete Message named "Name", derived from "Parent", and also derived on a custom data-structure called "Data".
That Data-structure should implement a classic boost "template<class Archive> void serialize( Archive& arch )"-function,
and should have a default-constructor to create the structure while loading.
Also it can have a normal constructor with arbitrary count of arguments. That Argument-count must be given in the last
macro-parameter called "paramSplitPosition". Then, all parameters given to the function-constructor up the the specified count will
be given to the Data-constructor, and all other parameters to the Parent.
Unlike the rest of the messaging-system, the data-structure may use the standard boost-serialization versioning-system.
 
The constructors can be specialized to fit special needs(see EASY_CONSTRUCTOR_...)
 
If you want to override virtual functions, use
EASY_DECLARE_MESSAGE_BEGIN(...)
override your functions here
END()
 
*/

#define EASY_CONSTRUCTOR_0(Name, Parent, index, Data)                                   \
inline Name( const Teamwork::MessageInfo& info ) : Data(), Parent( info ) {   \
}                                                       \
template<class Param1>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1 ) : Data(), Parent( info ) {   \
}                                                       \
template<class Param1>                                  \
Name( const Teamwork::MessageInfo& info, const Param1& p1 ) : Data(), Parent( info ) {   \
}                                                       \
template<class Param1, class Param2>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2 ) : Data(), Parent( info, p1, p2 ) {   \
} \
template<class Param1, class Param2, class Param3>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3 ) : Data(), Parent( info, p1, p2, p3 ) {   \
} \
template<class Param1, class Param2, class Param3, class Param4>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3, Param4& p4 ) : Data(), Parent( info, p1, p2, p3, p4 ) {   \
}                                 \
template<class Param1, class Param2, class Param3, class Param4, class Param5>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3, Param4& p4, Param5& p5 ) : Data(), Parent( info, p1, p2, p3, p4, p5 ) {   \
}           \
/**/

#define EASY_CONSTRUCTOR_1(Name, Parent, index, Data)                                   \
inline Name( const Teamwork::MessageInfo& info ) : Data(), Parent( info ) {   \
}                                                       \
template<class Param1>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1 ) : Data(p1), Parent( info ) {   \
}                                                       \
template<class Param1>                                  \
Name( const Teamwork::MessageInfo& info, const Param1& p1 ) : Data(p1), Parent( info ) {   \
}                                                       \
template<class Param1, class Param2>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2 ) : Data(p1), Parent( info, p2 ) {   \
} \
template<class Param1, class Param2, class Param3>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3 ) : Data(p1), Parent( info, p2, p3 ) {   \
} \
template<class Param1, class Param2, class Param3, class Param4>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3, Param4& p4 ) : Data(p1), Parent( info, p2, p3, p4 ) {   \
}                                 \
template<class Param1, class Param2, class Param3, class Param4, class Param5>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3, Param4& p4, Param5& p5 ) : Data(p1), Parent( info, p2, p3, p4, p5 ) {   \
} \
/**/

#define EASY_CONSTRUCTOR_2(Name, Parent, index, Data)                                   \
inline Name( const Teamwork::MessageInfo& info ) : Data(), Parent( info ) {   \
}                                                       \
template<class Param1>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1 ) : Data(p1), Parent( info ) {   \
}                                                       \
template<class Param1>                                  \
Name( const Teamwork::MessageInfo& info, const Param1& p1 ) : Data(p1), Parent( info ) {   \
}                                                       \
template<class Param1, class Param2>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2 ) : Data(p1, p2), Parent( info ) {   \
} \
template<class Param1, class Param2, class Param3>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3 ) : Data(p1, p2), Parent( info, p3 ) {   \
} \
template<class Param1, class Param2, class Param3, class Param4>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3, Param4& p4 ) : Data(p1, p2), Parent( info, p3, p4 ) {   \
}                                 \
template<class Param1, class Param2, class Param3, class Param4, class Param5>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3, Param4& p4, Param5& p5 ) : Data(p1, p2), Parent( info, p3, p4, p5 ) {   \
} \
/**/

#define EASY_CONSTRUCTOR_3(Name, Parent, index, Data)                                   \
inline Name( const Teamwork::MessageInfo& info ) : Data(), Parent( info ) {   \
}                                                       \
template<class Param1>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1 ) : Data(p1), Parent( info ) {   \
}                                                       \
template<class Param1>                                  \
Name( const Teamwork::MessageInfo& info, const Param1& p1 ) : Data(p1), Parent( info ) {   \
}                                                       \
template<class Param1, class Param2>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2 ) : Data(p1, p2), Parent( info ) {   \
} \
template<class Param1, class Param2, class Param3>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3 ) : Data(p1, p2, p3), Parent( info ) {   \
} \
template<class Param1, class Param2, class Param3, class Param4>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3, Param4& p4 ) : Data(p1, p2, p3), Parent( info, p4 ) {   \
}                                 \
template<class Param1, class Param2, class Param3, class Param4, class Param5>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3, Param4& p4, Param5& p5 ) : Data(p1, p2, p3), Parent( info, p4, p5 ) {   \
} \
/**/

#define EASY_CONSTRUCTOR_4(Name, Parent, index, Data)                                   \
inline Name( const Teamwork::MessageInfo& info ) : Data(), Parent( info ) {   \
}                                                       \
template<class Param1>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1 ) : Data(p1), Parent( info ) {   \
}                                                       \
template<class Param1>                                  \
Name( const Teamwork::MessageInfo& info, const Param1& p1 ) : Data(p1), Parent( info ) {   \
}                                                       \
template<class Param1, class Param2>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2 ) : Data(p1, p2), Parent( info ) {   \
} \
template<class Param1, class Param2, class Param3>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3 ) : Data(p1, p2, p3), Parent( info ) {   \
} \
template<class Param1, class Param2, class Param3, class Param4>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3, Param4& p4 ) : Data(p1, p2, p3, p4), Parent( info ) {   \
}                                 \
template<class Param1, class Param2, class Param3, class Param4, class Param5>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3, Param4& p4, Param5& p5 ) : Data(p1, p2, p3, p4), Parent( info, p5 ) {   \
} \
/**/

#define EASY_CONSTRUCTOR_5(Name, Parent, index, Data)                                   \
inline Name( const Teamwork::MessageInfo& info ) : Data(), Parent( info ) {   \
}                                                       \
template<class Param1>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1 ) : Data(p1), Parent( info ) {   \
}                                                       \
template<class Param1>                                  \
Name( const Teamwork::MessageInfo& info, const Param1& p1 ) : Data(p1), Parent( info ) {   \
}                                                       \
template<class Param1, class Param2>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2 ) : Data(p1, p2), Parent( info ) {   \
} \
template<class Param1, class Param2, class Param3>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3 ) : Data(p1, p2, p3), Parent( info ) {   \
} \
template<class Param1, class Param2, class Param3, class Param4>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3, Param4& p4 ) : Data(p1, p2, p3, p4), Parent( info ) {   \
}                                 \
template<class Param1, class Param2, class Param3, class Param4, class Param5>                                  \
Name( const Teamwork::MessageInfo& info, Param1& p1, Param2& p2, Param3& p3, Param4& p4, Param5& p5 ) : Data(p1, p2, p3, p4, p5), Parent( info ) {   \
} \
/**/

#define EASY_DECLARE_MESSAGE_BEGIN( Name, Parent, index, Data, paramSplitPosition  ) \
class Name : public Data, public Parent {                 \
DECLARE_MESSAGE( Name, Parent, index );                   \
  public:                                                 \
                                                                \
EASY_CONSTRUCTOR_ ## paramSplitPosition ( Name, Parent, index, Data )       \
  virtual void serialize( OutArchive& arch ) {  \
    Precursor::serialize( arch );                         \
    /*static_cast<Data&>(*this).serialize( arch, 0 );*/  \
    arch & boost::serialization::base_object<Data>(*this);  \
};                                                                \
Name( InArchive& arch, const Teamwork::MessageInfo& info ) : Precursor( arch, info ) {      \
    /*static_cast<Data&>(*this).serialize( arch, 0 );*/  \
    arch & boost::serialization::base_object<Data>(*this);          \
}                                                                     \
/**/

#define END()    \
}

#define EASY_DECLARE_MESSAGE( Name, Parent, index, Data, paramSplitPosition  ) \
EASY_DECLARE_MESSAGE_BEGIN( Name, Parent, index, Data, paramSplitPosition )  \
END()

///Can be used as a dummy for EASY_DECLARE_MESSAGE, if no data is wished
struct NoData {
  template <class Archive>
  void serialize( Archive& /*arch*/, unsigned int /*version*/ ) {}
}
;

template <class T = int>
struct NoDataB {
  template <class Archive>
  void serialize( Archive& /*arch*/, unsigned int /*version*/ ) {}
}
;

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
