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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "../../crossmap.h"
#include <assert.h>
#include <iostream>
#include <list>

using namespace Utils;
using namespace std;

struct TestMappable {
  NAMED_TYPE( Name, std::string );
  int ints[ 2 ];

  char* bla;
  std::string string;
  template <class Type, int number>
  Type getKey() const {
    return Type::Error;
  }

  std::list<int> intList;
  
  Name name;

  bool operator == ( const TestMappable& rhs ) const {
    return ints[ 0 ] == rhs.ints[ 0 ] && ints[ 1 ] == rhs.ints[ 1 ] && bla == rhs.bla && string == rhs.string && name == rhs.name;
  }
};


BIND_LIST_6( KeyList, TestMappable::Name, int, char*, std::string, int, std::list<int> );

///A little test for GetListItem
Assert< IfSame< GetListItem< 0, KeyList >::Result, TestMappable::Name, Success, Fail >::Result::value >::OK ok1;
Assert<IfSame< GetListItem< 1, KeyList >::Result, int, Success, Fail >::Result::value >::OK ok2;
Assert<IfSame< GetListItem< 2, KeyList >::Result, char*, Success, Fail >::Result::value >::OK ok3;
Assert<IfSame< GetListItem< 3, KeyList >::Result, std::string, Success, Fail >::Result::value >::OK ok4;
Assert<IfSame< GetListItem< 4, KeyList >::Result, int, Success, Fail >::Result::value >::OK ok42;

Assert < FindInList< TestMappable::Name, KeyList >::value == 0 > ::OK ok5;
Assert < FindInList< int, KeyList >::value == 1 > ::OK ok6;
Assert < FindInList< char*, KeyList >::value == 2 > ::OK ok7;
Assert < FindInList< std::string, KeyList >::value == 3 > ::OK ok8;
Assert < FindInList< int, KeyList, 1 >::value == 4 > ::OK ok62;

Assert < FindRelativeInList< TestMappable::Name, KeyList >::value == 0 > ::OK ok51;
Assert < FindRelativeInList< int, KeyList, 1 >::value == 1 > ::OK ok61;
Assert < FindRelativeInList< char*, KeyList, 2 >::value == 2 > ::OK ok71;
Assert < FindRelativeInList< std::string, KeyList, 3 >::value == 3 > ::OK ok81;
Assert < FindRelativeInList< int, KeyList, 4 >::value == 4 > ::OK ok621;

AssertSame< FindRelativeInList< TestMappable::Name, KeyList >::relativeIndex, 0 >::OK ok52;
AssertSame< FindRelativeInList< int, KeyList, 1 >::relativeIndex, 0 >::OK ok623;
AssertSame< FindRelativeInList< char*, KeyList, 2 >::relativeIndex, 0 >::OK ok72;
AssertSame< FindRelativeInList< std::string, KeyList, 3 >::relativeIndex, 0 >::OK ok82;
AssertSame< FindRelativeInList< int, KeyList, 4 >::relativeIndex, 1 >::OK ok622;

template<class Type>
std::list<Type> make_list( const Type& t ) {
  std::list<Type> ret;
  ret.push_back( t );
  return ret;
}

template<class Type>
std::list<Type> make_list( const Type& i, const Type& i2 ) {
  std::list<Type> ret;
  ret.push_back( i );
  ret.push_back( i2 );
  return ret;
}

template<class Type>
std::list<Type> make_list( const Type& i, const Type& i2, const Type& i3 ) {
  std::list<Type> ret;
  ret.push_back( i );
  ret.push_back( i2 );
  ret.push_back( i3 );
  return ret;
}

template<class Type>
std::list<Type> make_list( const Type& i, const Type& i2, const Type& i3, const Type& i4 ) {
  std::list<Type> ret;
  ret.push_back( i );
  ret.push_back( i2 );
  ret.push_back( i3 );
  ret.push_back( i4 );
  return ret;
}

template <>
int TestMappable::getKey<int, 0>() const {
  return ints[ 0 ];
}

template <>
int TestMappable::getKey<int, 1>() const {
  return ints[ 1 ];
}

template <>
TestMappable::Name TestMappable::getKey<TestMappable::Name, 0>() const {
  return name;
}

template <>
std::string TestMappable::getKey<std::string, 0>() const {
  return string;
}

template <>
char* TestMappable::getKey<char*, 0>() const {
  return bla;
}

template <>
std::list<int> TestMappable::getKey<std::list<int>, 0>() const {
  return intList;
}

typedef CrossMap< TestMappable, KeyList> TestSet;
CrossMap< TestMappable, KeyList> testSet;


TestMappable randomMappable() {
  TestMappable t1;
  int r = rand() % 5;
  switch ( r ) {
    case 0:
    t1.name = "honk";
    break;
    case 1:
    t1.name = "peter";
    break;
    case 2:
    t1.name = "frank";
    break;
    case 3:
    t1.name = "guildo";
    break;
    case 4:
    t1.name = "joschi";
    break;
  }
  t1.ints[ 0 ] = rand() % 100;
  t1.ints[ 1 ] = rand() % 100;
  r = rand() % 5;

  switch ( r ) {
    case 0:
    t1.string = "urug";
    break;
    case 1:
    t1.string = "welsch";
    break;
    case 2:
    t1.string = "ick";
    break;
    case 3:
    t1.string = "harr";
    break;
    case 4:
    t1.string = "bloog";
    break;
  }
  return t1;
}


int main() {

  cout << "testing" << endl;
  TestMappable t1;
  t1.name = "honk";
  t1.ints[ 0 ] = 15;
  t1.ints[ 1 ] = 16;
  t1.string = "someString";

  TestMappable t2;
  t2.name = "noHonk";
  t2.ints[ 0 ] = 15;
  t2.ints[ 1 ] = 27;
  t2.string = "no string";

  t1.intList = make_list<int>( 10, 11, 12, 13 );
  t2.intList = make_list<int>( 11, 13, 17 );

  testSet.insert( t1 );

  assert( (testSet.value<2, int>( 10 ) == t1) );
  assert( (testSet.value<2, int>( 11 ) == t1) );
  assert( (testSet.value<2, int>( 12 ) == t1) );
  assert( (testSet.value<2, int>( 13 ) == t1) );
  assert( !(testSet.value<2, int>( 14 ) == t1) );

  assert( testSet.value<TestMappable::Name> ( "honk" ) == t1 );
  assert( testSet.value<std::string> ( "someString" ) == t1 );
  assert( testSet.value<int> ( 15 ) == t1 );
  assert( ( testSet.value<1, int>( 16 ) == t1 ) );

  testSet.update( testSet.find( t1 ) );

  assert( testSet.value<TestMappable::Name> ( "honk" ) == t1 );
  assert( testSet.value<std::string> ( "someString" ) == t1 );
  assert( testSet.value<int> ( 15 ) == t1 );
  assert( ( testSet.value<1, int>( 16 ) == t1 ) );

  assert( testSet.update( testSet.find( testSet[ 15 ] ) ) );

  assert( testSet.hasContent() );

  testSet.remove( testSet.value<1, int>( 16 ) );

  assert( !testSet.hasContent() );

  testSet.insert( t1 );
  testSet.insert( t1 );
  testSet.insert( t2 );

  assert( (testSet.value<2, int>( 10 ) == t1) );
  assert( (testSet.value<2, int>( 17 ) == t2) );
  assert( (testSet.values<2, int>( 11 ).count() == 3) );

  CrossMap< TestMappable, KeyList>::Iterator it = testSet.values<int>( 15 );
  assert( it.count() == 3 );
  while ( it ) {
    cout << "Item: " << ( *it ).name.value << endl;
    ++it;
  }

  assert( testSet.remove( t2 ) );

  assert( testSet.count() == 2 );

  assert( testSet.value<TestMappable::Name> ( "honk" ) == t1 );
  assert( testSet.value<std::string> ( "someString" ) == t1 );
  assert( testSet.value<int> ( 15 ) == t1 );
  assert( ( testSet.value<1, int>( 16 ) == t1 ) );

  testSet.remove( t1 );

  assert( testSet.value<TestMappable::Name> ( "honk" ) == t1 );
  assert( testSet.value<std::string> ( "someString" ) == t1 );
  assert( testSet.value<int> ( 15 ) == t1 );
  assert( ( testSet.value<1, int>( 16 ) == t1 ) );

  testSet.remove( t1 );

  assert( testSet.count() == 0 );
  assert( !testSet.hasContent() );

  cout << "success" << endl;

  return 0;
}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
