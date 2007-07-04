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

#ifndef FLEXIBLETEXT_H
#define FLEXIBLETEXT_H

#include <boost/serialization/extended_type_info_typeid.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/string.hpp>
#include <iostream>
#include <list>
#include <sstream>
#include <deque>

#include "sumtree.h"
#include "flexibletextnotifier.h"

namespace SumTree {

template<class Tp>
class TemporaryExchange {
  public:
    TemporaryExchange( Tp& target, Tp newValue ) : target_(target), oldValue_(target) {
      target_ = newValue;
    }
    ~TemporaryExchange() {
      target_ = oldValue_;
    }
  private:
    Tp& target_;
    Tp oldValue_;
};
///This is just a little helper that autmatically deleted unremoved members of a string-list on exceptions etc.
///If they should be used on elsewhere, they must be removed from this list.
template <class StringList>
struct ManagedStringList {
  ManagedStringList( const StringList& rhs ) : list( rhs ) {}

  void delete_front() {
    delete list.front();
    list.pop_front();
  }

  ~ManagedStringList() {
    for ( typename StringList::iterator it = list.begin(); it != list.end(); ++it ) {
      delete *it;
    }
  }

  StringList* operator -> () {
    return & list;
  }
  StringList list;
};

/**This is a class that simulates a normal string, but is much more flexible and allows access using line/column- and index-information.
 * For big documents that are edited it is much more efficient than a string, because much less allocation has to be done, since the
 * document is internally stored within smaller units(divided into lines).
 *
 * Any function in this class may throw DynamicTextError on error!
 * */

template <class String, class Char = char, Char lineBreak = '\n'>
class FlexibleText {
  typedef std::deque<String*> StringVector;
  public:
    typedef std::list<String*> StringList;

    FlexibleText( const String& text = "" ) : notify_(0) {
      build( text );
    }

    ~FlexibleText() {
      deleteText();
    }

//     template <class StringListType>
//     FlexibleText( const StringListType& text ) {
//       build( text );
//     }

    FlexibleText( const FlexibleText& rhs ) : notify_(0) {
      operator=( rhs );
    }

    FlexibleText& operator = ( const FlexibleText& rhs ) {
      deleteText();
      for( typename StringVector::const_iterator it = rhs.text_.begin(); it != rhs.text_.end(); ++it ) {
        text_.push_back( new String( **it ) );
      }
      lineColMap_ = rhs.lineColMap_;
      return *this;
    }

    void linearToLineColumn( int position, int& line, int& column ) const {
      SumTree::IndexAndSum s;
      lineColMap_.indexAndSum( position, s );
      line = s.index;
      column = position - s.sum;
    }

    int lineColumnToLinear( int line, int column ) const {
      return lineColMap_.sum( line ) + column;
    }

    String substr( int position, int count ) const {
      SumTree::IndexAndSum s;
      lineColMap_.indexAndSum( position, s );
      return substr( s.index, position - s.sum, count );
    }

    
    void replace( int position, int length, const String& replacement )
    {
      ///Maybe implement this more efficiently as a single action
      if( notify_ )
        notify_->notifyFlexibleTextReplace( position, length, replacement );
      TemporaryExchange< FlexibleTextNotifier<String>* > hideNotify( notify_, 0 );
      
      SumTree::IndexAndSum s;
      lineColMap_.indexAndSum( position, s );
      int column = position - s.sum;
      erase( s.index, column, length );
      insert( s.index, column, replacement );
    }

    void insert( int position, const String& text )
    {
      if( notify_ )
        notify_->notifyFlexibleTextInsert( position, text );
      TemporaryExchange< FlexibleTextNotifier<String>* > hideNotify( notify_, 0 );
      
      SumTree::IndexAndSum s;
      lineColMap_.indexAndSum( position, s );
      insert( s.index, position - s.sum, text );
    }

    ///Remove 'length' charateres at position 'position'. remove and erase do the same, they are provided for compatibility with Qt and STL.
    void remove( int position, int length ) {
      return erase( position, length );
    }
    
    ///Remove 'length' charateres at position 'position'. remove and erase do the same, they are provided for compatibility with Qt and STL.
    void erase( int position, int length )
    {
      if( notify_ )
        notify_->notifyFlexibleTextErase( position, length );
      TemporaryExchange< FlexibleTextNotifier<String>* > hideNotify( notify_, 0 );
      
      SumTree::IndexAndSum s;
      lineColMap_.indexAndSum( position, s );
      erase ( s.index, position - s.sum, length );
    }

    String substr( int line, int column, int count ) const {
      String ret;
      int currentColumn = column;
      int currentLine = line;
      uint size = text_.size();
      while ( count > 0 ) {
        if ( currentLine >= (int)size )
          return ret;
        int get
          = min( text_[ currentLine ] ->length() - column, count );
        ret += text_[ currentLine ] ->substr( column, get
                                              );
        count -= get
                   ;
        if ( count != 0 ) {
          ret.append( "\n" );
          count -= 1;
        }
        currentLine++;
        currentColumn = 0;
      }
      return ret;
    }

    int length() const {
      int s = lineColMap_.sum();
      DYN_VERIFY_SMALLER( 0, s );
      return s - 1;
    }

    ///SHould be called time by time(for efficiency), especially if many insertions have been done.
    void rebuildTree() {
      //lineColMap_ = lineColMap_.rebuild();
    }

    void checkConsistency() {
#ifdef EXTREMEDEBUG
      int sum = 0;
      int num = 0;
      int sz = text_.size();
      for ( typename StringVector::iterator it = text_.begin(); it != text_.end(); ++it ) {
        int lineColSum = lineColMap_.sum( num );
        if( lineColMap_.indexValue( num ) != (*it)->length() + 1 || sum != lineColSum ) {
          cout << "consistency-fail: at line " << num << " text: \n" << dump() << "offsets-tree: " << lineColMap_.dump() << endl;
        }
        
        DYN_VERIFY_SAME( lineColMap_.indexValue( num ), (*it)->length() + 1 );
        DYN_VERIFY_SAME( sum, lineColSum );
        num++;
        sum += ( *it ) ->length() + 1;
      }
#endif
    };
    
    ///Returns the whole text as the normal string-representation
    String text() const {
      return substr( 0, 0, length() );
    }

    /*operator String() const {
      return substr( 0, 0, length() );
    }*/

    std::string dump() const {
      String ret;
      int line = 0;
      for( typename StringVector::const_iterator it = text_.begin(); it != text_.end(); ++it ) {
        std::ostringstream os; os << "line " << line << "(len " << (*it)->length() << "): \"" << **it << "\"\n";
        ret += os.str();
        line++;
      }
      return ret;
    }

    std::string dumpTree() const {
      return lineColMap_.dump();
    }

    template<class Archive>
    void save( Archive& arch, const uint ) const {
      int lines = text_.size();
      arch << lines;
      for( typename StringVector::const_iterator it = text_.begin(); it != text_.end(); ++it ) {
        arch << **it;
      }
    }

    template<class Archive>
    void load( Archive& arch, const uint ) {
      deleteText();

      int lines;
      arch >> lines;

      for( int a = 0; a < lines; a++ ) {
        String* s = new String();
        arch >> *s;
        text_.push_back( s );
      }
      rebuildTree();
    }

    ///Very inefficient, ranges should be used whenever possible
    Char operator [] ( uint offset ) const {
      IndexAndSum s;
      lineColMap_.indexAndSum( offset, s );
      return (*text_[s.index])[offset - s.sum];
    }

    /** Register a notifier that will be notified with ANY change to the text, exactly before the change is applied */
    void registerNotifier( FlexibleTextNotifier<String>* notifier ) const {
      notify_ = notifier;
    };

    /** unregister a previously registered notifier */
    void unregisterNotifier() const {
      notify_ = 0;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
  private:

    ///Erase and insert could be public, the only problem is that the change-notification would become more complicated(currently notification is done in linear measure)
    
    void insert( int line, int column, const String& text ) {
#ifdef EXTREMDEBUG
      cout << "insertion at " << line << ":" << column << " length " << text.length() << " of \"" << text << "\"" << endl;
      checkConsistency();
#endif
      uint len = text.length();
      if( len == 0 ) return;
      int currentStart = 0;
      bool nobreak = true;
      String endAppend; ///Needs to be appended to the last line inserted
      int l = line + 1;

      for ( uint a = 0; a < len; a++ ) {
        if ( text[ a ] == lineBreak ) {
          if( nobreak ) {
            ///It is the first line, put it into the insertion-line and break.
            int insertLength = a - currentStart;
            String& curLine( *text_[line] );
            int removeLength = curLine.length() - column;
            DYN_VERIFY_SMALLERSAME( 0, removeLength );
            ///Copy the last part of the line it is inserted to, and append it to the last line inserted.
            endAppend = curLine.substr( column, removeLength );

            ///Replace the removed part of the inserted-to line with the first inserted line
            lineColMap_.changeIndexValue( line, insertLength - removeLength );
            text_[ line ] ->replace( column, removeLength, text, currentStart, insertLength );
          } else {
            ///Now insert all other lines behind "line"
            text_.insert( text_.begin() + l, new String( text, currentStart, a - currentStart ) );
            lineColMap_.insertIndex( l, a - currentStart + 1 );
            l++;
          }
          
          currentStart = a + 1;
          nobreak = false;
        }
      }
       
      if( nobreak ) {
        text_[ line ] ->insert( column, text );
        lineColMap_.changeIndexValue( line, len );
#ifdef EXTREMDEBUG
        checkConsistency();
#endif
      } else {
        ///insert the last part
        String* s = new String( text, currentStart, len - currentStart );
        s->append( endAppend );
        text_.insert( text_.begin() + l, s );
        lineColMap_.insertIndex( l, s->length() + 1 );
      }

#ifdef EXTREMEDEBUG
      cout << "text after: \n" << dump() << "offsets after: " << lineColMap_.dump() << endl;
      checkConsistency();
#endif
    }
        
    ///Remove 'length' charateres in line 'line', starting at column 'column'. If the length is longer then the rest of the line, this will continue deleting in the next line.
    void erase( int line, int column, int length ) {
#ifdef EXTREMEDEBUG
      cout << "erasing " << line << ":" << column << " length " << length << endl;
      cout << "text before: \n" << dump() << "offsets before: " << lineColMap_.dump() << endl;
      checkConsistency();
#endif

      if ( length == 0 )
        return ;

#ifdef EXTREMEDEBUG
      DYN_VERIFY_SMALLERSAME( 0, removeLength );
      DYN_VERIFY_SAME( text_[ line ] ->length() + 1, lineColMap_.indexValue( line ) );
      DYN_VERIFY_SMALLER( 0, lineColMap_.indexValue( line ) );
#endif
      int removeLength = min( length, text_[ line ] ->length() - column );
      int lengthLeft = length - removeLength;
      lineColMap_.changeIndexValue( line, -removeLength );
      text_[ line ] ->erase( column, removeLength );
      bool broken = true;

      if ( lengthLeft )
        lengthLeft -= 1; ///Count one character for the linebreak, because it is removed and the next line pushed up
      else
        return ;

      ///First find out what exactly has to be removed
      int currentLine = line;

      while ( lengthLeft > 0 ) {
        currentLine++;
        lengthLeft -= text_[ currentLine ] ->length();
        if ( lengthLeft > 0 ) {
          broken = true;
          lengthLeft -= 1; ///If the whole line is removed, also count the linebreak.
        } else {
          broken = false;
        }
      }

      ///Add the rest of the line that caused the stop to the beginning line
      if ( lengthLeft != 0 ) {
        ///Add the last -lengthLeft characters to text_[line]
        text_[ line ] ->append( text_[ currentLine ] ->substr( text_[ currentLine ] ->length() + lengthLeft, -lengthLeft ) );
        lineColMap_.changeIndexValue( line, -lengthLeft );
      } else if ( broken ) {
        ///Move  the next unaffected line up
        if ( (int)text_.size() > currentLine + 1 ) {
          currentLine++;
          uint len = text_[ currentLine ] ->length();
          text_[ line ] ->append( *text_[ currentLine ] );
          lineColMap_.changeIndexValue( line, len );
        }
      }

      ///Remove all unneeded lines
      for ( int a = line + 1; a <= currentLine; a++ ) {
        delete text_[ a ];
        lineColMap_.removeIndex( line + 1 );
      }

      text_.erase( text_.begin() + line + 1, text_.begin() + currentLine + 1 );

#ifdef EXTREMDEBUG
      checkConsistency();
#endif
    }
    
    void deleteText() {
      for( typename StringVector::iterator it = text_.begin(); it != text_.end(); ++it ) {
        delete *it;
      }
      text_.clear();
    }
    StringList split( const String& text ) {
      StringList ret;
      uint len = text.length();
      int currentStart = 0;

      for ( uint a = 0; a < len; a++ ) {
        if ( text[ a ] == lineBreak ) {
          ret.push_back( new String( text.substr( currentStart, a - currentStart ) ) );
          currentStart = a + 1;
        }
      }
      //if ( currentStart != len )
        ret.push_back( new String( text.substr( currentStart, len - currentStart ) ) );

      return ret;
    }
    template <class StringListType>
    void build( const StringListType& lines ) {
      std::vector<int> length;
      int size = lines.size();
      length.resize( size );

      int a = 0;
      for ( typename StringListType::const_iterator i = lines.begin(); i != lines.end(); i++ ) {
        length[ a ] = i->length() + 1;  ///Plus one for the next-line character
        text_.push_back( new String( *i ) );
        a++;
      }

      //length[ size - 1 ] -= 1; ///The last line should not get the linebreak.
      lineColMap_ = SumTree::Map( length );
    }

    void build( const String& text ) {
      std::vector<int> length;
      ManagedStringList<StringList> lines( split( text ) );
      int size = lines->size();
      length.resize( size );

      int a = 0;
      while( !lines->empty() ) {
        length[ a ] = lines->front()->length() + 1;  ///Plus one for the next-line character
        text_.push_back( lines->front() );
        lines->pop_front();
        a++;
      }

      //length[ size - 1 ] -= 1; ///The last line should not get the linebreak.
      lineColMap_ = SumTree::Map( length );
    }

    inline static int min( int a, int b ) {
      return a < b ? a : b;
    }

    mutable FlexibleTextNotifier<String>* notify_;
    StringVector text_; ///Not the most intelligent solution(some combination of maps and offset-maps would be perfect)
    SumTree::Map lineColMap_;
};
}

template <class String, class Char, Char lineBreak>
std::ostream& operator << ( std::ostream& o, const SumTree::FlexibleText<String, Char, lineBreak>& rhs ) {
  o << rhs.text();
  return o;
}



#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
