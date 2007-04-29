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
#include "messagetypeset.h"
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
Unlike the rest of the messaging-system, the Data-structure may use the standard boost-serialization versioning-system.
 
EASY_IMPLEMENT_MESSAGE(MessageName) must be used in a source-file to implement the message.
EASY_IMPLEMENT_MESSAGE also calls REGISTER_MESSAGE, so that doesn't have to be done separately when this system is used.

The constructors can be specialized to fit special needs(see EASY_CONSTRUCTOR_...)
 
If you want to override virtual functions or want to access protected members, use
EASY_DECLARE_MESSAGE_BEGIN(...)
override your functions here(this is inside of the classes body)
END()


*/

//These are class-initialization-declarations for with different redirections to Data / Parent
//First: Total count of parameters, second: how many of them go into Data
#define PARAMETER_SPLIT_0_0(Data, Parent) Data(), Parent( info )
#define PARAMETER_SPLIT_0_1(Data, Parent) Data(), Parent( info )
#define PARAMETER_SPLIT_0_2(Data, Parent) Data(), Parent( info )
#define PARAMETER_SPLIT_0_3(Data, Parent) Data(), Parent( info )
#define PARAMETER_SPLIT_0_4(Data, Parent) Data(), Parent( info )
#define PARAMETER_SPLIT_0_5(Data, Parent) Data(), Parent( info )

#define PARAMETER_SPLIT_1_0(Data, Parent) Data(), Parent( info, p1 )
#define PARAMETER_SPLIT_1_1(Data, Parent) Data( p1 ), Parent( info )
#define PARAMETER_SPLIT_1_2(Data, Parent) Data( p1 ), Parent( info )
#define PARAMETER_SPLIT_1_3(Data, Parent) Data( p1 ), Parent( info )
#define PARAMETER_SPLIT_1_4(Data, Parent) Data( p1 ), Parent( info )
#define PARAMETER_SPLIT_1_5(Data, Parent) Data( p1 ), Parent( info )

#define PARAMETER_SPLIT_2_0(Data, Parent) Data(), Parent( info, p1, p2 )
#define PARAMETER_SPLIT_2_1(Data, Parent) Data(p1), Parent( info, p2 )
#define PARAMETER_SPLIT_2_2(Data, Parent) Data(p1, p2), Parent( info )
#define PARAMETER_SPLIT_2_3(Data, Parent) Data(p1, p2), Parent( info )
#define PARAMETER_SPLIT_2_4(Data, Parent) Data(p1, p2), Parent( info )
#define PARAMETER_SPLIT_2_5(Data, Parent) Data(p1, p2), Parent( info )

#define PARAMETER_SPLIT_3_0(Data, Parent) Data(), Parent( info, p1, p2, p3 )
#define PARAMETER_SPLIT_3_1(Data, Parent) Data(p1), Parent( info, p2, p3 )
#define PARAMETER_SPLIT_3_2(Data, Parent) Data(p1, p2), Parent( info, p3 )
#define PARAMETER_SPLIT_3_3(Data, Parent) Data(p1, p2, p3), Parent( info )
#define PARAMETER_SPLIT_3_4(Data, Parent) Data(p1, p2, p3), Parent( info )
#define PARAMETER_SPLIT_3_5(Data, Parent) Data(p1, p2, p3), Parent( info )

#define PARAMETER_SPLIT_4_0(Data, Parent) Data(), Parent( info, p1, p2, p3, p4 )
#define PARAMETER_SPLIT_4_1(Data, Parent) Data(p1), Parent( info, p2, p3, p4 )
#define PARAMETER_SPLIT_4_2(Data, Parent) Data(p1, p2), Parent( info, p3, p4 )
#define PARAMETER_SPLIT_4_3(Data, Parent) Data(p1, p2, p3), Parent( info, p4 )
#define PARAMETER_SPLIT_4_4(Data, Parent) Data(p1, p2, p3, p4), Parent( info )
#define PARAMETER_SPLIT_4_5(Data, Parent) Data(p1, p2, p3, p4), Parent( info )

#define PARAMETER_SPLIT_5_0(Data, Parent) Data(), Parent( info, p1, p2, p3, p4, p5 )
#define PARAMETER_SPLIT_5_1(Data, Parent) Data(p1), Parent( info, p2, p3, p4, p5 )
#define PARAMETER_SPLIT_5_2(Data, Parent) Data(p1, p2), Parent( info, p3, p4, p5 )
#define PARAMETER_SPLIT_5_3(Data, Parent) Data(p1, p2, p3), Parent( info, p4, p5 )
#define PARAMETER_SPLIT_5_4(Data, Parent) Data(p1, p2, p3, p4), Parent( info, p5 )
#define PARAMETER_SPLIT_5_5(Data, Parent) Data(p1, p2, p3, p4, p5), Parent( info )

#define EASY_CONSTRUCTOR(Name, Parent, index, Data, SplitPosition)                                   \
inline Name( const Teamwork::MessageTypeSet& info ) : Data(), Parent( info ) {   \
}                                                       \
template<class Param1>                                  \
Name( const Teamwork::MessageTypeSet& info, Param1& p1 ) : PARAMETER_SPLIT_ ## 1 ## _ ## SplitPosition (Data,Parent)  {   \
}                                                       \
template<class Param1>                                  \
Name( const Teamwork::MessageTypeSet& info, const Param1& p1 ) : PARAMETER_SPLIT_ ## 1 ## _ ## SplitPosition (Data,Parent) {   \
}                                                       \
\
template<class Param1, class Param2>                                  \
Name( const Teamwork::MessageTypeSet& info, Param1& p1, Param2& p2 ) : PARAMETER_SPLIT_ ## 2 ## _ ## SplitPosition (Data,Parent) {   \
} \
template<class Param1, class Param2>                                  \
Name( const Teamwork::MessageTypeSet& info, const Param1& p1, Param2& p2 ) : PARAMETER_SPLIT_ ## 2 ## _ ## SplitPosition (Data,Parent) {   \
} \
template<class Param1, class Param2>                                  \
Name( const Teamwork::MessageTypeSet& info, Param1& p1, const Param2& p2 ) : PARAMETER_SPLIT_ ## 2 ## _ ## SplitPosition (Data,Parent) {   \
} \
template<class Param1, class Param2>                                  \
Name( const Teamwork::MessageTypeSet& info, const Param1& p1, const Param2& p2 ) : PARAMETER_SPLIT_ ## 2 ## _ ## SplitPosition (Data,Parent) {   \
} \
\
template<class Param1, class Param2, class Param3>                                  \
Name( const Teamwork::MessageTypeSet& info, Param1& p1, Param2& p2, Param3& p3 ) : PARAMETER_SPLIT_ ## 3 ## _ ## SplitPosition (Data,Parent) {   \
} \
template<class Param1, class Param2, class Param3>                                  \
Name( const Teamwork::MessageTypeSet& info, Param1& p1, Param2& p2, const Param3& p3 ) : PARAMETER_SPLIT_ ## 3 ## _ ## SplitPosition (Data,Parent) {   \
} \
template<class Param1, class Param2, class Param3>                                  \
Name( const Teamwork::MessageTypeSet& info, const Param1& p1, const Param2& p2, const Param3& p3 ) : PARAMETER_SPLIT_ ## 3 ## _ ## SplitPosition (Data,Parent) {   \
} \
template<class Param1, class Param2, class Param3>                                  \
Name( const Teamwork::MessageTypeSet& info, const Param1& p1, Param2& p2, const Param3& p3 ) : PARAMETER_SPLIT_ ## 3 ## _ ## SplitPosition (Data,Parent) {   \
} \
template<class Param1, class Param2, class Param3>                                  \
Name( const Teamwork::MessageTypeSet& info, Param1& p1, const Param2& p2, const Param3& p3 ) : PARAMETER_SPLIT_ ## 3 ## _ ## SplitPosition (Data,Parent) {   \
} \
\
template<class Param1, class Param2, class Param3, class Param4>                                  \
Name( const Teamwork::MessageTypeSet& info, Param1& p1, Param2& p2, Param3& p3, Param4& p4 ) : PARAMETER_SPLIT_ ## 4 ## _ ## SplitPosition (Data,Parent) {   \
}                                 \
template<class Param1, class Param2, class Param3, class Param4>                                  \
Name( const Teamwork::MessageTypeSet& info, Param1& p1, Param2& p2, const Param3& p3, const Param4& p4 ) : PARAMETER_SPLIT_ ## 4 ## _ ## SplitPosition(Data,Parent) {   \
}                                 \
template<class Param1, class Param2, class Param3, class Param4, class Param5>                                  \
Name( const Teamwork::MessageTypeSet& info, Param1& p1, Param2& p2, Param3& p3, Param4& p4, Param5& p5 ) : PARAMETER_SPLIT_ ## 5 ## _ ## SplitPosition(Data,Parent) {   \
}           \
/**/


//The first 4 lines of the following macros are used to store the values for the implementation, so they must not be given to the implementation again
#define EASY_DECLARE_MESSAGE_BEGIN( Name, Parent, index, Data, paramSplitPosition  ) \
class Name : public Data, public Parent {                 \
DECLARE_MESSAGE( Name, Parent, index );                   \
typedef Data DataType;                                    \
  public:                                                 \
                                                                \
EASY_CONSTRUCTOR ( Name, Parent, index, Data, paramSplitPosition )       \
virtual void serialize( OutArchive& arch );                                     \
Name( InArchive& arch, const Teamwork::MessageInfo& info );                      \
/**/

#define EASY_IMPLEMENT_MESSAGE(Name) \
  void Name :: serialize( OutArchive& arch ) {  \
    Precursor::serialize( arch );                         \
    /*static_cast<DataType&>(*this).serialize( arch, 0 );*/  \
    arch & boost::serialization::base_object<DataType>(*this);  \
}                                                                \
Name :: Name ( InArchive& arch, const Teamwork::MessageInfo& info ) : Precursor( arch, info ) {      \
    /*static_cast<DataType&>(*this).serialize( arch, 0 );*/  \
    arch & boost::serialization::base_object<DataType>(*this);          \
}                                                                     \
REGISTER_MESSAGE(Name)                                \
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
