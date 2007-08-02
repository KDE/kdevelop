/***************************************************************************
Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "offsetutils.h"
#include <assert.h>
#include "sumtree.h"

#define CNT 500
#define SMALLCNT 50
#define CYCLES 1

#define private public
#include "dynamictext.h"
#include "flexibletext.h"
#include <time.h>
#include <sstream>

typedef std::list<std::string> StringList;

std::list<std::string> splitString( const std::string& text ) {
  std::list<std::string> ret;
  uint len = text.length();
  int currentStart = 0;

  for ( uint a = 0; a < len; a++ ) {
    if ( text[ a ] == '\n' ) {
      ret.push_back( std::string( text.substr( currentStart, a - currentStart ) ) );
      currentStart = a + 1;
    }
  }
  //if ( currentStart != len )
  ret.push_back( std::string( text.substr( currentStart, len - currentStart ) ) );

  return ret;
}


std::string compareText( const std::string& str1, const std::string& str2 ) {
  std::ostringstream os;
  StringList l1 = splitString( str1 );
  StringList l2 = splitString( str2 );
  uint size = l1.size();
  if ( l2.size() > size )
    size = l2.size();

  StringList::iterator it1 = l1.begin();
  StringList::iterator it2 = l2.begin();
  int cnt1 = 0;
  int cnt2 = 0;
  os << "total length:" << str1.length() << "" << str2.length() << endl;

  for ( uint a = 0; a < size; a++ ) {
    std::string text1 = "empty";
    std::string text2 = "empty";

    if ( it1 != l1.end() )
      text1 = "\"" + *it1 + "\"";

    if ( it2 != l2.end() )
      text2 = "\"" + *it2 + "\"";

    os << a << "(total counts" << cnt1 << ":" << cnt2 << "):" << text1 << ":" << text2 << "(length" << ( text1 == "empty" ? 0 : it1->length() ) << ":" << ( text2 == "empty" ? 0 : it2->length() ) << ")" << ( text1 == text2 ? "match" : "MISMATCH" ) << "\n" ;

    if ( it1 != l1.end() ) {
      cnt1 += it1->length() + 1;
      ++it1;
    }
    if ( it2 != l2.end() ) {
      cnt2 += it2->length() + 1;
      ++it2;
    }
  }
  return os.str();
  //": \nflexText:\"" << flexText.text() << "\"\nstring: \n\"" << string << "\"" << endl
}

struct doubleText {
  SumTree::FlexibleText<std::string> flexText;
  std::string string;
  clock_t flexClock;
  clock_t stringClock;
  doubleText( const std::string& str ) : flexText( str ), string( str ), flexClock( 0 ), stringClock( 0 ) {}
  int lastActionType;
  int lastPosition;
  std::string lastText;
  int lastLength;

  void insert( int position, std::string text ) {
    lastActionType = 0;
    lastPosition = position;
    lastText = text;

    //flexText.rebuildTree();
    clock_t c = clock();
    flexText.insert( position, text );
    flexClock += clock() - c;
    c = clock();
    string.insert( position, text );
    stringClock += clock() - c;
    //compare();
  }
  void remove
    ( int position, int length ) {
    lastActionType = 1;
    lastPosition = position;
    lastLength = length;

    clock_t c = clock();
    flexText.erase( position, length );
    flexClock += clock() - c;
    c = clock();
    string.erase( position, length );
    stringClock += clock() - c;
    //compare();
  }
  void compare() {
    bool fail = false;


    if ( flexText.text().length() != string.length() )
      fail = true;

    if( flexText.length() < 10000 ) {
      //As long as the text is not big, check for line/column conversion consistency
      int len = flexText.length();
      for( int a = 0; a < len; a++ ) {
        int line, column;
        flexText.linearToLineColumn( a, line, column );
        if( flexText.lineColumnToLinear( line, column ) != a ) {
          cout << "linear to line/column-check: back-conversion failed at position" << a << endl;
          fail = false;
        }
      }
    }

    if( fail ){
      cout << "error, last action:" << ( lastActionType ? "removal " : "insertion " ) << "of \"";
      if ( lastActionType )
        cout << lastLength;
      else
        cout << lastText;
      cout << "\" at" << lastPosition << "comparing flexText(left) to string(right): \n" << compareText( flexText.text(), string );
    }

    DYN_VERIFY_SAME( flexText.text().length(), string.length() );
    //cout << "length:" << flexText.text().length() << "" << string.length() << endl;
    DYN_VERIFY_SAME( flexText.text(), string );
  }
  void showClock() {
    cout << "\\flexible:" << float( flexClock ) / float( CLOCKS_PER_SEC ) << endl << "\\string:" << float( stringClock ) / float( CLOCKS_PER_SEC ) << endl;
  }
};

typedef SumTree::FlexibleText<std::string> Text;
void verifyFlexibleText() {
  std::list<std::string> bla;
  bla.push_back( "hallo" );
  bla.push_back( "Ich bin der David" );

  {
    /*Text:
a (length 2, total 2)
b (length 3, total 5)
ccc (length 4, total 9)
    (length 1, total 10)
dddd (length 5, total 15)

     */
    Text text( "a\nbb\nccc\n\ndddd\n" );
    int line, column;
    text.linearToLineColumn( 0, line, column );
    DYN_VERIFY_SAME( line, 0 ); DYN_VERIFY_SAME( column, 0 );

    text.linearToLineColumn( 1, line, column );
    DYN_VERIFY_SAME( line, 0 ); DYN_VERIFY_SAME( column, 1 );

    text.linearToLineColumn( 2, line, column );
    DYN_VERIFY_SAME( line, 1 ); DYN_VERIFY_SAME( column, 0 );

    text.linearToLineColumn( 4, line, column );
    DYN_VERIFY_SAME( line, 1 ); DYN_VERIFY_SAME( column, 2 );

    text.linearToLineColumn( 5, line, column );
    DYN_VERIFY_SAME( line, 2 ); DYN_VERIFY_SAME( column, 0 );

    text.remove( 3, 1 );
    text.linearToLineColumn( 3, line, column );
    DYN_VERIFY_SAME( line, 1 ); DYN_VERIFY_SAME( column, 1 );

    text.linearToLineColumn( 4, line, column );
    DYN_VERIFY_SAME( line, 2 ); DYN_VERIFY_SAME( column, 0 );
    cout << "Line-wrap tested ok" << endl;
  };

  Text text( "hallo\nIch bin der David" );
  cout << text.text() << endl;
  text.erase( 1, 1, 2 );
  text.erase( 0, 6 );
  text.insert( text.length() - 1, "ende!\naber" );
  text.insert( text.length() - 1, "\n" );
  cout << text.text() << endl;

  char inputs[ 44 ] = {'a', 'b', 'c', 'd', ' ', 'f', 'g', 'h', 'i', 'j', '\n', 'a', 'b', ' ', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'q', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'q'};

  std::string input;

  for ( int a = 0; a < rand() % 1000 + 50; a++ ) {
    input += inputs[ rand() % 44 ];
  }

  doubleText d( input );
  int cnt = 0;

  for ( int b = 0; b < 100000; b++ ) {
    ///remove a random small range
    if ( d.string.size() != 0 ) {
      uint remSize = rand() % 5;
      uint remPos = rand() % d.string.size();
      if ( remPos + remSize > d.string.size() )
        remSize = d.string.size() - remPos;
      d.remove( remPos, remSize );
    }

    uint addSize = rand() % 10;
    uint addPos = rand() % ( d.string.size() + 1 );
    std::string input;
    for ( uint a = 0; a < addSize; a++ ) {
      input += inputs[ rand() % 44 ];
    }
    d.insert( addPos, input );
    ++cnt;
    if ( cnt == 1000 ) {
      cnt = 0;
      d.showClock();
      cout << "size:" << d.string.size() << endl;
      d.compare();
    }

    ///Copy the text and remove some part, to make sure that the tree stays consistent
    Text f( d.flexText );
    if ( d.string.size() != 0 ) {
      uint remSize = rand() % 5;
      uint remPos = rand() % d.string.size();
      if ( remPos + remSize > d.string.size() )
        remSize = d.string.size() - remPos;
      f.remove( remPos, remSize );
    }
    //d.compare();
  }
  //cout << "\n" << d.flexText.dumpTree() << "\n";

}

void verifySumTree() {

  try {
    SumTree::Map tree;
    tree.insertRange( 0, 10, 1 );
    for( int a = 0; a < 10; a++ ) {
      DYN_VERIFY_SAME( tree.indexValue( 0 ), 1 );
      DYN_VERIFY_SAME( tree.index( a ), a );
      SumTree::IndexAndSum r;
      tree.indexAndSum( a, r );
      DYN_VERIFY_SAME( r.index, a );
      DYN_VERIFY_SAME( r.sum, a );
      cout << a << ":" << tree.indexValue( a ) << ""<< tree.sum( a ) << endl;
    }
    DYN_VERIFY_SAME( tree.sum( 10 ), 10 );
    DYN_VERIFY_SAME( tree.sum( 11 ), 10 );
    tree.setIndexValue( 1, 0 );
    for( int a = 0; a < 10; a++ )
      cout << a << ":" << tree.indexValue( a ) << "" << tree.sum( a ) << endl;
    DYN_VERIFY_SAME( tree.sum( 10 ), 9 );
    DYN_VERIFY_SAME( tree.sum( 11 ), 9 );
    DYN_VERIFY_SAME( tree.indexValue( 1 ), 0 );
    DYN_VERIFY_SAME( tree.indexValue( 2 ), 1 );
    tree.changeIndexValue( 1, 1 );
    DYN_VERIFY_SAME( tree.indexValue( 1 ), 1 );
    DYN_VERIFY_SAME( tree.indexValue( 2 ), 1 );
    tree.insertIndex( 3, 0 );
    DYN_VERIFY_SAME( tree.indexValue( 3 ), 0 );
    DYN_VERIFY_SAME( tree.sum( 5 ), 4 );
    tree.removeIndex( 0 );
    tree.removeIndex( 0 );
    tree.removeIndex( 0 );
    DYN_VERIFY_SAME( tree.indexValue( 0 ), 0 );

  } catch ( DynamicTextError err ) {
  cout << "remove-index-test failed:" << err.what() << endl;
    terminate();
  }

  int cycles = 3;
  for ( int a = 0; a < cycles; a++ ) {
    cout << "testing cycle" << a + 1 << endl;
    std::vector<int > summands;
    //srand ( time(NULL) );
    int count = rand() % 20000;
    summands.resize( count );

    for ( int a = 0; a < count; a++ ) {
      summands[ a ] = rand() % 2000 + 1;
    }

    try {
      SumTree::Map tree( summands );
      int sum = tree.sum( 5 );
/*      int nsum = tree.sum( 6 );*/
      DYN_VERIFY_SAME( tree.index( sum ), 5 );
      tree.setIndexValue( 5, 0 );
      //      DYN_VERIFY_SAME( tree.index( sum ), 6 ); ///Reactivate in time!
    } catch ( DynamicTextError err ) {
      cout << "custom test failed:" << err.what() << endl;
      terminate();
    }

    //uint t = timeGetTime();
    try {
      SumTree::Map tree( summands );
      DYN_VERIFY_SAME( tree.indexCount(), count );
    } catch ( DynamicTextError err ) {
      cout << "count-invariance failed:" << err.what() << endl;
      terminate();
    }
    //cout << "count-invariance elapsed time:" << timeGetTime() - t << endl;

    int a = 0;
    try {
      SumTree::Map tree( summands );
      int sum = 0;
      for ( a = 0; a < count; a++ ) {
        int s = tree.sum( a );
        DYN_VERIFY_SAME( s, sum );
        sum += summands[ a ];
        int ind = tree.index( s );
        DYN_VERIFY_SAME( ind, a );
      }

    } catch ( DynamicTextError err ) {
      cout << "sum-test failed:" << err.what() << endl;
      terminate();
    }

    //cout << "sum-test elapsed time:" << time(NULL) - t << endl;

    try {
      SumTree::Map tree( summands );
/*      int sum = 0;*/
      for ( int a = 0; a < count; a++ ) {
        int s = tree.indexValue( a );
        DYN_VERIFY_SAME( s, summands[ a ] );
      }

    } catch ( DynamicTextError err ) {
      cout << "index-value failed:" << err.what() << endl;
      terminate();
    }

    try {
      SumTree::Map tree( summands );
      for ( int a = 0; a < count; a++ ) {
        tree.setIndexValue( a, a );
        DYN_VERIFY_SAME( a, tree.indexValue( a ) );
      }

    } catch ( DynamicTextError err ) {
      cout << "set-index-value failed:" << err.what() << endl;
      terminate();
    }

    try {
      SumTree::Map tree( summands );
      for ( int a = 0; a < count; a++ ) {
        int v = tree.indexValue( a );
        DYN_VERIFY_SAME( v + a, tree.changeIndexValue( a, a ) );
      }

    } catch ( DynamicTextError err ) {
      cout << "change-index-value failed:" << err.what() << endl;
      terminate();
    }

    try {
      SumTree::Map tree( summands );
      for ( int a = 0; a < count; a += 2 ) {
        int v = tree.indexValue( a );
        tree.insertIndex( a, a );
        DYN_VERIFY_SAME( tree.indexValue( a ), a );
        DYN_VERIFY_SAME( tree.indexValue( a + 1 ), v );
      }
    } catch ( DynamicTextError err ) {
      cout << "insert-index-test failed:" << err.what() << endl;
      terminate();
    }
    try {
      SumTree::Map tree( summands );
      for ( int a = count / 10; a >= 0; a-- ) {
        int r = tree.removeIndex( a * 9 );
        DYN_VERIFY_SAME( r, summands[ a * 9 ] );
      }
    } catch ( DynamicTextError err ) {
      cout << "remove-index-test failed:" << err.what() << endl;
      terminate();
    }
  }


}

///@todo create so automatic testing-algorithm that tests the system more intensively

void verifyDynamicText() {
  cout << "testing replacement-system" << endl;

  try {
    DynamicText t;
    cout << "test 0.1: \n";
    VectorTimestamp a5 = t.insert( 0, SimpleReplacement( 0, "a", "" ) );
    VectorTimestamp a1 = t.insert( 0, SimpleReplacement( 1, "a", "" ) );
    VectorTimestamp b1 = t.insert( 1, SimpleReplacement( 2, "b", "" ) );
    VectorTimestamp a2 = t.insert( 0, SimpleReplacement( 3, "a", "" ) );

    cout << t.state().print() << "text:" << t.text() << endl;

    VectorTimestamp v( t.state() );
    v.setStamp( 0, 0 );
    t.changeState( v );
    cout << t.state().print() << v << "text:" << t.text() << endl;
    DYN_VERIFY_SAME( t.text().text(), "b" );

    v.setStamp( 0, 1 );
    t.changeState( v );
    cout << t.state().print() << v << "text:" << t.text() << endl;
    DYN_VERIFY_SAME( t.text().text(), "ab" );

    v.setStamp( 0, 2 );
    t.changeState( v );
    cout << t.state().print() << v << "text:" << t.text() << endl;
    DYN_VERIFY_SAME( t.text().text(), "aab" );

    v.setStamp( 0, 3 );
    t.changeState( v );
    DYN_VERIFY_SAME( t.text().text(), "aaba" );

  } catch ( DynamicTextError err ) {
    cout << "error:" << err.what();
    terminate();
  }

  try {
    DynamicText t;
    cout << "test 4: \n";
    VectorTimestamp a1 = t.insert( 0, SimpleReplacement( 0, "i", "" ) );
    VectorTimestamp b1 = t.insert( 1, SimpleReplacement( 0, "d", "" ) );
    VectorTimestamp a2 = t.insert( 0, SimpleReplacement( 0, "v", "" ) ); ///!
    VectorTimestamp b2 = t.insert( 1, SimpleReplacement( 0, "d", "" ) );
    VectorTimestamp a3 = t.insert( 0, SimpleReplacement( t.text().length(), "r", "" ) );
    //VectorTimestamp a4 = t.insert( 0, SimpleReplacement( 0, " ", "" ) );
    VectorTimestamp a5 = t.insert( 0, SimpleReplacement( 0, "q", "" ) );  ///!
    VectorTimestamp a6 = t.insert( 0, SimpleReplacement( 1, "v", "" ) );

    cout << t.state().print() << "text:" << t.text() << endl;
    VectorTimestamp v( t.state() );
    v.setStamp( 1, 0 );
    t.changeState( v );
    cout << t.state().print() << v << "text:" << t.text() << endl;

    v = t.state();
    v.setStamp( 0, 1 );
    t.changeState( v );
    cout << t.state().print() << v << "text:" << t.text() << endl;

  } catch ( DynamicTextError err ) {
    cout << "error:" << err.what();
    terminate();
  }

  try {
    DynamicText t;
    cout << "test 2: \n";
    VectorTimestamp a1 = t.insert( 0, SimpleReplacement( 0, "a", "" ) );
    VectorTimestamp a2 = t.insert( 0, SimpleReplacement( 1, "b", "" ) );
    VectorTimestamp b1 = t.insert( 1, SimpleReplacement( 2, "c", "" ) );
    VectorTimestamp b2 = t.insert( 1, SimpleReplacement( 3, "d", "" ) );
    VectorTimestamp a3 = t.insert( 0, SimpleReplacement( 4, "e", "" ) );

    cout << t.state().print() << "text:" << t.text() << endl;
    VectorTimestamp v( t.state() );
    v.setStamp( 1, 1 );
    t.changeState( v );
    cout << t.state().print() << "text:" << t.text() << endl;

  } catch ( DynamicTextError err ) {
    cout << "error:" << err.what();
    terminate();
  }

  try {
    DynamicText t;
    cout << "test 1.1: \n";
    t.insert( 0, SimpleReplacement( 0, "a", "" ) );
    t.insert( 0, SimpleReplacement( 1, "b", "" ) );
    t.insert( 1, SimpleReplacement( 1, "q", "" ) );
    t.insert( 1, SimpleReplacement( 2, "q", "" ) );

    cout << t.state().print() << "text:" << t.text() << endl;
    VectorTimestamp v( t.state() );
    v.setStamp( 0, 1 );
    t.changeState( v );
    cout << t.state().print() << v << "text:" << t.text() << endl;
    /*
    v = t.state();
    v.setStamp( 0, 1 );
    t.changeState( v );
    cout << t.state().print() << v << "text:" << t.text() << endl;*/

  } catch ( DynamicTextError err ) {
    cout << "error:" << err.what();
    terminate();
  }

  try {
    cout << "test:\n";
    DynamicText t;
    VectorTimestamp start = t.insert( 0, SimpleReplacement( 0, "HalloichbinderDavid", "" ) );
    VectorTimestamp u0r1 = t.insert( 0, SimpleReplacement( 0, "Ahoiiiiii", "Hallo" ) );
    VectorTimestamp u1r1( 1, start );
    t.insert( u1r1, SimpleReplacement( 5, "ja", "ich" ) );
    VectorTimestamp u2r1( 2, start );
    t.insert( u2r1, SimpleReplacement( 8, "jsem", "bin" ) );
    t.changeState();
    cout << t.state().print() << "text:" << t.text() << endl;
    VectorTimestamp u1r2( 1, t.state() );
    t.insert( u1r2, SimpleReplacement( 18, "Davidek", "David" ) );
    t.changeState();
    VectorTimestamp mix = t.state();
    mix.setStamp( 0, 1 );
    t.changeState( mix );
    cout << t.state().print() << "text:" << t.text() << endl;
    t.changeState( u1r1 );
    cout << t.state().print() << "text:" << t.text() << endl;
    t.changeState( start );
    ReplacementPointer r = t.replacement( 0, 2 );
    assert( r );
    r->setEnabled( false );
    r = t.replacement( 2, 1 );
    assert( r );
    r->setEnabled( false );
    t.changeState();

    cout << t.state().print() << ", some disabled, text:" << t.text() << endl;

  } catch ( DynamicTextError err ) {
    cout << "error:" << err.what();
  }

  {
    cout << "test:\n";
    std::string text = "hallo";
    std::string text2 = text;
    OffsetMap om;
    ReplacementPointer r = new Replacement( VectorTimestamp(), SimpleReplacement( 1, "BLA", "al" ) );
    ReplacementPointer r2 = new Replacement( VectorTimestamp(), SimpleReplacement( 4, "HeHe", "lo" ) );
    ReplacementPointer r3 = new Replacement( VectorTimestamp(), SimpleReplacement( 2, "Ne nee neee", "LAHe" ) );
    //r->setEnabled( false );
    r->apply( text2, OffsetMap(), om );
    r2->apply( text2, OffsetMap(), om );
    r3->apply( text2, OffsetMap(), om );

    std::string text3 = text2;
    om.printMap();
    cout << endl;

    r3->unApply( text3, OffsetMap(), om );
    r2->unApply( text3, OffsetMap(), om );
    r->unApply( text3, OffsetMap(), om );
    cout << "before apply: \"" << text << "\", after: \"" << text2 << "\" after unapply: \"" << text3 << "\"" << endl;
    cout << "offset-map: ";
    om.printMap();
    cout << endl;
  }

  cout << "testing document-manager" << endl;
  VectorTimestamp ts( 0, VectorTimestamp() );

  try {
    DynamicText t;
    cout << "test 1.2: \n";
    VectorTimestamp a1 = t.insert( 0, SimpleReplacement( 0, "a", "" ) );
    VectorTimestamp c1 = t.insert( 2, SimpleReplacement( 0, "w", "" ) );
    VectorTimestamp a2 = t.insert( 0, SimpleReplacement( 1, "b", "" ) );
    VectorTimestamp b1 = t.insert( 1, SimpleReplacement( 1, "q", "" ) );

#define VERIFY_OFFSET( index, from, to )    \
    {  OffsetMap om = t.offset( to, from ); \
      cout << "verifying offset from" << from.print() << "to" << to.print() << "at index" << index << "map is:" << om.print() << cout;  \
      t.changeState( from );  \
      cout << "text in state" << from.print() << ":" << t.text() << endl;   \
      char c = t.text()[index];   \
      t.changeState( to );    \
      cout << "text in state" << to.print() << ":" << t.text() << endl;   \
      cout << "index:" << index << "mapped index:" << om(index) << "values:" << c << "" << t.text()[ om( index ) ] << endl;   \
      DYN_VERIFY_SAME( c, t.text()[ om( index ) ] );    \
      OffsetMap om2 = t.offset( from, to ); DYN_VERIFY_SAME( om2( om( index ) ), index );   \
  }   \

    VERIFY_OFFSET( 2, a2, a1 );

    VERIFY_OFFSET( 2, b1, a2 );

    t.changeState();
    cout << t.state().print() << "text:" << t.text() << endl;
    VectorTimestamp v( t.state() );
    v.setStamp( 2, 0 );
    t.changeState( v );
    cout << t.state().print() << v << "text:" << t.text() << endl;

    v.setStamp( 0, 1 );
    t.changeState( v );
    cout << t.state().print() << v << "text:" << t.text() << endl;

  } catch ( DynamicTextError err ) {
    cout << "error:" << err.what();
    terminate();
  }

  DynamicText t;

  t.insert( ts, SimpleReplacement( 0, "aabb", "" ) );
  t.changeState( ts );


  VectorTimestamp ts1( 1, ts );
  VectorTimestamp ts2( 0, ts );
  t.insert( ts1, SimpleReplacement( 1, "a_b", "ab" ) );
  t.insert( ts2, SimpleReplacement( 3, "cc", "b" ) );
  VectorTimestamp v( 1, ts1 );
  v.maximize( ts2 );

  t.insert( v, SimpleReplacement( 0, "dd", "a" ) );
  t.changeState();


  cout << "text:" << t.text() << endl;
  VectorTimestamp ss( 0 );
  ss.setStamp( 0, 1 );
  ss.setStamp( 1, 2 );
  t.changeState( ss );
  cout << "text on 1,2:" << t.text() << endl;

  t.changeState();
  /*VectorTimestamp nv( 0, t.state() );
  t.insert( nv, SimpleReplacement( 0, "ey", "dda" ) );
  t.changeState();*/

  cout << "end text:" << t.text() << endl;

  VectorTimestamp ns( 0, t.state() );
  t.insert( ns, SimpleReplacement( 5, "hihi", "c" ) );
  t.changeState();

  cout << "whole end text:" << t.text() << endl;
  /*
  ns.setStamp( 1, 0 );

  t.changeState( ns );*/

  cout << "text edited by 1:" << t.text() << endl;

  ss.setStamp( 1, 0 );

  cout << "ready" << endl;
}


void verifyOffsets() {
  {
    //srand ( time( NULL ) );
    OffsetMap o;
    o %= OffsetMap( 3, 1 );
    o %= OffsetMap( 6, 2 );
    assert( o( 0 ) == 0 );
    assert( o( 1 ) == 1 );
    assert( o( 2 ) == 2 );
    assert( o( 3 ) == 4 );
    assert( o( 6 ) == 9 );
    assert( o( 7 ) == 10 );


    cout << "verifying offset-map\\m";

    for ( int b = 0; b < 1000; b++ ) {
      try {
        {
          int pos1 = rand() % 10;
          int off1 = rand() % 10;
          if( off1 < -pos1 ) off1 = pos1;

          int pos2 = rand() % 10;
          int off2 = rand() % 10;
          if( off2 < -pos2 ) off2 = pos2;

          OffsetMap om( pos1, off1);
          OffsetMap om2( pos2, off2 );
          OffsetMap om3 = om2 % om;
          OffsetMap om3Inv = ~om3;
          for ( int a = 0; a < 10; a++ ) {
            DYN_VERIFY_SAME( om2( om( a ) ), om3( a ) );
            /*int om3A = om3(a);
            int om3Invom3A = om3Inv( om3A );
            int result = om3( om3Invom3A );
            DYN_VERIFY_SAME( result, om3A );*/
          }
        }
      } catch ( const DynamicTextError & error ) {
        cout << "verifyOffsets error:" << error.what() << endl;
        terminate();
      }
    }
    cout << "success\n";
  }


  cout << "building map 1" << endl;
  OffsetMap o;
  OffsetMap o2;
  for ( int a = 0; a < SMALLCNT; a++ ) {
    float f = ( float( rand() ) / RAND_MAX ) * CNT;
    int of = (int)( ( float( rand() ) / RAND_MAX ) * 100 ) - 50;
    if ( of ) {
      OffsetMap in( ( uint ) f, of );
      if ( o.isCompatible( in ) )
        o %= in;
    }
  }
  cout << "ready" << endl;

  cout << "building map 2" << endl;
  for ( int a = 0; a < SMALLCNT; a++ ) {
    float f = ( float( rand() ) / RAND_MAX ) * CNT;
    int of = (int)( ( float( rand() ) / RAND_MAX ) * 100 ) - 50;
    if ( of ) {
      OffsetMap in( ( uint ) f, of );
      if ( o.isCompatible( in ) )
        o %= in;
      /*      else
              cout << "incompatible:" << (uint)f << "" << of << endl;*/
    }
  }
  cout << "ready" << endl;
  /*
  o %= OffsetMap( 3, 1 );
  o %= OffsetMap( 5, 1 );
  o %= OffsetMap( 10, -2 );
  o %= OffsetMap( 15, 3 );

  o2 %= OffsetMap( 1, 1 );
  o2 %= OffsetMap( 8, -2 );
//   o2 % OffsetMap( 5, 1 );
//   o2 % OffsetMap( 10, -2 );
//   o2 % OffsetMap( 15, 3 );

  //cout << "map 1: ";
  o.printMap();
  //cout << "map 2: ";
  o2.printMap();
  if ( !o.isCompatible( o2 ) ) {
    cout << "map 2 is not compatible" << endl;
    return ;
  }*/
  OffsetMap o3 = o % o2;
  //cout << "combined: ";
  o3.printMap();

  /*  cout << "inversion-test: ";
    OffsetMap oInv(o);
    ~oInv;
    oInv.printMap();
    cout << endl;
    for( int a = 5; a < CNT; a++ ) {
      int oa = o(a);
      cout << a << "==" << oInv( oa );
      cout << "(" << oa << ")" << endl;
      assert( oInv( o(a) ) == a );
    }
    cout << "success "<< endl;*/


  cout << "testing o(o2(x)) == o3(x) ( with o3 = o % o2 )" << endl;
  for ( int a = 0; a < CNT; a++ ) {
    if ( o2( a ) < 0 || o( a ) < 0 )
      continue;
    if ( !o( o2( a ) ) == o3( a ) )
      cout << "error for a" << a << ":" << o( a ) << "" << o2( a ) << ":" << o( o2( a ) ) << "!=" << o3( a ) << endl;
    assert( o( o2( a ) ) == o3( a ) );
  }
  cout << "success" << endl;

  o3 = o3 % ( ~o2 );

  cout << "testing (o % o2) % (~o2)  == o2" << endl;
  for ( int a = 0; a < CNT; a++ ) {
    if ( o2( a ) < 0 || o( a ) < 0 )
      continue;
    int val1 = o3( a );
    int val2 = o( a );
    if ( val1 != val2 )
      cout << "error for a" << a << ":" << o( a ) << "" << o3( a ) << ":" << val1 << "!=" << val2 << endl;
    assert( val1 == val2 );
  }
  cout << "success" << endl;

}


int main( int /* argc */, char /**argv[]*/ ) {
  verifyDynamicText();
  verifySumTree();
  verifyFlexibleText();
  verifyOffsets();

  //printf("testing\n");
  //  for( int q = 0; q < CYCLES; q++ ) {
  //  }


  return EXIT_SUCCESS;
}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
