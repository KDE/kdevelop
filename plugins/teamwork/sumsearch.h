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



#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

///For higher orders to work, the sum-type has to be signed
#define DEFAULTSUMTYPE int
#define DEFAULTORDER 2


///State-based-build can only nearly complete with extreme compiler-parameters set, although it should theoretically be faster, if the compile would optimize it the right way.

//#define STATEBASEDBUILD

namespace FuzzySearch {

/**
 * How should all the differences be weighed together to a compound difference?
 *
 * The weighting still needs some work.
 *
 * */

template < class Compare1, class Compare2, class Then, class Else >
class IfEqual {
public:
  typedef Else Result;
};

template < class Compare, class Then, class Else >
class IfEqual<Compare, Compare, Then, Else> {
public:
  typedef Then Result;
};

template <class Item>
class PrimitiveVector {
    Item* data_;
    uint size_;
  public:
    PrimitiveVector() : data_( 0 ), size_( 0 ) {}
    ;

    ~PrimitiveVector() {
      if ( data_ )
        delete[] data_;
    }

    ///All data is lost when this is called
    void rawResize( unsigned int i ) {
      if ( size_ == i )
        return ;
      size_ = i;
      if ( data_ )
        delete[] data_;
      if ( !i ) {
        data_ = 0;
      } else {
        data_ = new Item[ i ];
      }
    };

    Item& operator [] ( uint index ) {
      return data_[ index ];
    }

    const Item& operator [] ( uint index ) const {
      return data_[ index ];
    }

    uint size() const {
      return size_;
    }
};

///unsigned int is ok for document-sizes of up to about 16 Megabytes, 64-bit integers create problems here
using namespace std;
template <int maxDepth, class SumType = DEFAULTSUMTYPE>
struct SumGroup {
  SumType sums[ maxDepth ];

  SumGroup( SumType* _sums = 0 ) {
    if ( _sums ) {
      for ( int a = 0; a < maxDepth; a++ ) {
        sums[ a ] = _sums[ a ];
      }
    } else {
      for ( int a = 0; a < maxDepth; a++ )
        sums[ a ] = 0;
    }
  }

  /*
  SumType sumDifference( const SumGroup& rhs ) const {
    SumType ret = 0;
    for( int a = 0; a < maxDepth; a++ ) {
      SumType dif;
      if( sums[a] > rhs.sums[a] )
        dif = sums[a] - rhs.sums[a];
      else
        dif = rhs.sums[a] - sums[a];

      ret += ( dif * (maxDepth - a ) ) >> a;
    }
    return ret;
  };*/

  struct VecSumGetter {
    SumType** sumVec;
    VecSumGetter() : sumVec( 0 ) {}
    VecSumGetter( SumType** vec ) : sumVec( vec ) {}

    inline bool sumValid( const uint num ) const {
      return ( bool ) sumVec[ num ];
    }


    inline SumType sum( const uint num ) const {
      return * ( sumVec[ num ] );
    }
  };

  struct ZeroGetter {
    inline bool sumValid( const uint num ) const {
      return false;
    }
    inline SumType sum( const uint num ) const {
      return 0;
    }
  };

  struct GroupSumGetter {
    SumGroup& group;
    GroupSumGetter( SumGroup& grp ) : group( grp ) {}

    inline bool sumValid( const uint num ) const {
      return true;
    }


    inline SumType sum( const uint num ) const {
      return group.sums[ num ];
    }
  };

  template <class SumGetter>
  SumType sumDifference( const SumGetter& getter ) const {
    SumType ret = 0;
    for ( int a = 0; a < maxDepth; a++ ) {
      SumType dif;
      if ( getter.sumValid( a ) ) {
        if ( sums[ a ] > getter.sum( a ) )
          dif = sums[ a ] - getter.sum( a );
        else
          dif = getter.sum( a ) - sums[ a ];
      } else {
        dif = sums[ a ];
      }

      ///The weighting is a bit primitive at the moment
      ret += ( dif * ( ( maxDepth - a ) + 1 ) ) /* >> a*/;
    }
    return ret;
  }

  template <class Archive>
  void serialize( Archive& arch, const uint /*version*/ ) {
    arch & sums;
  }
};

template < int Condition, class Then, class Else >
struct IfThen {
  typedef Then Result;
};

template < class Then, class Else >
struct IfThen <0, Then, Else> {
  typedef Else Result;
};


template <int maxDepth, typename SumType, int order, bool dummy>
class DifferenceSumReference {
  public:
    template <class Archive>
    void serialize( Archive& /*arch*/, const uint /*version*/ ) {}
}
;

template <int maxDepth, class SumType = DEFAULTSUMTYPE, int order = DEFAULTORDER>
class SumReference {
    bool valid_;
  public:
    SumGroup<maxDepth, SumType> leftMarker;
    SumGroup<maxDepth, SumType> rightMarker;

    DifferenceSumReference < maxDepth, SumType, order - 1, ( order - 1 ) == 0 > nextOrder_;

    SumReference() : valid_( false ) {}

    template <class Archive>
    void serialize( Archive& arch, const uint /*version*/ ) {
      arch & valid_ & leftMarker & rightMarker;
      arch & nextOrder_;
    }

    void setValid( bool valid ) {
      valid_ = valid;
    }

    bool isValid() const {
      return valid_;
    }
};


template <int maxDepth, typename SumType, int order>
struct DifferenceSumReference<maxDepth, SumType, order, false> {
  SumReference<maxDepth, SumType, order> reference_;
public:
  template <class Archive>
  void serialize( Archive& arch, const uint /*version*/ ) {
    arch & reference_;
  }
};

/** The problem with a single sum-search: 123456| is detected as exactly same as 124356|.
 * For that reason a parallel difference-sum-search can be used, which would look like 111111| in the first, and 112(-1)21| in the second case.
 * SumType must be signed for that to work.
 * */


template <int maxDepth, typename SumType, int order, bool dummy>
class DifferenceSumSearch {
  public:
    DifferenceSumSearch( const string& /*text*/ ) {}
    template <class Type>
    void fillReference( Type& /*t*/, int /*position*/ ) {}

    template <class PrevType, class Evaluator>
    struct FindRefProgress {
      FindRefProgress( PrevType& /*t*/, const SumReference < maxDepth, SumType, order + 1 > & /*_ref*/, Evaluator& /*_eval*/ ) {}
      SumType leftDiff() {
        return 0;
      }

      SumType rightDiff() {
        return 0;
      }

      void next() {}
    }
    ;
};


template <class SumType, int Depth, int EndDepth>
struct SumBuildState : public SumBuildState < SumType, Depth + 1, EndDepth > {
  typedef SumBuildState< SumType, Depth, EndDepth> Self;
  typedef SumBuildState < SumType, Depth + 1, EndDepth > Next;
  typedef SumBuildState< SumType, 0, EndDepth> Head;
  SumType* v;

  uint tempSumC;
  SumType tempSum;
  const char* text;
  const char* ctext;
  uint offset;

  //uint maxCount = 1<<b;

  SumBuildState( PrimitiveVector<SumType>* sumVecs_, const char* tx ) : Next( sumVecs_, tx ), tempSumC( 0 ), tempSum( 0 ), text( tx ), ctext( tx ), offset( 0 ) {
    v = &( sumVecs_[ Depth ][ 0 ] );
  }

  inline void go1( register const char* ctex, register const uint offset ) {
    Next::go1( ctex, offset );
    {
      if ( tempSumC == ( uint ) ( 1 << Depth ) ) {
        ///remove the first summand
        tempSum -= *( ctex - ( 1 << Depth ) );
      } else {
        tempSumC ++;
      }

      tempSum += *ctex;
      ( * ( v + offset ) ) = tempSum;
    }
  }

  /*  inline void go2( register uint offset ) {
      Next::go2( offset );
      //++v;
  }*/

  inline void go() {
    go1( ctext, offset );

    //go2( offset );

    ++ctext;
    ++offset;
  }
};

template <class SumType, int Depth>
struct SumBuildState<SumType, Depth, Depth> {
  SumBuildState( PrimitiveVector<SumType>*, const char* ) {}
  ;

  inline void go1( register const char* ctex, register const uint offset ) {}
  //  inline void go2() {
  //  }
}
;


template <int maxDepth, typename SumType = DEFAULTSUMTYPE, int order = DEFAULTORDER>
class SumSearch {
    typedef SumSearch<maxDepth, SumType, order> SelfSumSearch;
    typedef PrimitiveVector< SumType > SumVector;
    string text_;
    SumVector sumVecs_[ maxDepth ];
    typedef DifferenceSumSearch < maxDepth, SumType, order - 1, ( order - 1 ) == 0 > NextOrderType;

#ifdef STATEBASEDBUILD

    void buildSumGroups() {
      for ( int a = 0; a < maxDepth; a++ ) {
        sumVecs_[ a ].rawResize( text_.size() );
      }

      SumBuildState< SumType, 0, maxDepth > state( sumVecs_, text_.c_str() );

      uint textSize = text_.size();

      for ( uint a = 0; a < textSize; a++ ) {
        state.go();
      }
    }
#else
    void buildSumGroups() {
      for ( int a = 0; a < maxDepth; a++ ) {
        sumVecs_[ a ].rawResize( text_.size() );
      }

      const char* text = text_.c_str();
      uint textSize = text_.size();

      for ( uint b = 0; b < maxDepth; b++ ) {

        SumType* v = &( sumVecs_[ b ][ 0 ] );
        uint tempSumC = 0;
        SumType tempSum = 0;
        uint maxCount = 1 << b;

        const char* ctext = text;

        for ( uint a = 0; a < textSize; a++ ) {
          if ( tempSumC == maxCount ) {
            ///remove the first summand
            tempSum -= *( ctext - maxCount );
          } else {
            tempSumC ++;
          }

          tempSum += ( *ctext );

          ( *v ) = tempSum;
          ++v;
          ++ctext;
        }
      }
    }
#endif

  public:
    NextOrderType nextOrder_;

    SumSearch( const string& text ) : nextOrder_( text ) {
      text_ = text;
      buildSumGroups();
    }

    ///position != 0
    SumReference<maxDepth, SumType, order> getReference( int position ) {
      SumReference<maxDepth, SumType, order> ret;
      fillReference( ret, position );

      nextOrder_.fillReference( ret, position );
      return ret;
    }


    ///Should not be used from outside
    void fillReference( SumReference<maxDepth, SumType, order>& ret, int position ) {
      int leftPos = position - 1;
      SumType leftSums[ maxDepth ];
      SumType rightSums[ maxDepth ];

      if ( leftPos >= 0 ) {
        for ( int a = 0; a < maxDepth; a++ )
          leftSums[ a ] = sumVecs_[ a ] [ leftPos ];
      } else {
        for ( int a = 0; a < maxDepth; a++ )
          leftSums[ a ] = 0;
      }

      for ( int a = 0; a < maxDepth; a++ ) {
        uint pos = leftPos + ( 1 << a );
        if ( pos < sumVecs_[ 0 ].size() )
          rightSums[ a ] = sumVecs_[ a ][ pos ];
        else
          rightSums[ a ] = 0;
      }

      ret.leftMarker = SumGroup<maxDepth, SumType>( leftSums );
      ret.rightMarker = SumGroup<maxDepth, SumType>( rightSums );
      ret.setValid( true );
    }


    struct SimpleEvaluator {
      SumType minDif;
      uint minDifPos;

      SimpleEvaluator() : minDif( 100000 ), minDifPos( -1 ) {}
      /**This should return a new compound difference-value. In the end, the position with the lowest difference will be returned,
        *so this may be used to implement some weighting, maybe according to the position */
      inline void operator () ( const SumType leftDifference, const SumType rightDifference, const uint position ) {
        SumType dif = ( leftDifference + 1 ) * ( rightDifference + 1 );

        if ( dif < minDif ) {
          //cout << "best dif: " << dif << endl;
          minDif = dif;
          minDifPos = position;
        }
      }

      uint result() {
        return minDifPos;
      }
    };

    SumVector* sumVecs() {
      return sumVecs_;
    }

    template <class Evaluator>
    struct FindReferenceProgress {
      SumReference<maxDepth, SumType, order>& ref;
      Evaluator& eval;

      typename NextOrderType::template FindRefProgress<SelfSumSearch, Evaluator>
      nextOrder_;

      SumType* leftOffset[ maxDepth ];
      SumType* rightOffset[ maxDepth ];

      SumType* firstItem;
      SumType* finalRight[ maxDepth ];

      SumVector* sumVecs_;

      typename SumGroup<maxDepth, SumType>::VecSumGetter leftGetter;
      typename SumGroup<maxDepth, SumType>::VecSumGetter rightGetter;
      typedef typename SumGroup<maxDepth, SumType>::ZeroGetter ZeroGetter;

      FindReferenceProgress( SelfSumSearch& search, const SumReference<maxDepth, SumType, order>& _ref, Evaluator& _eval ) : ref( const_cast<SumReference<maxDepth, SumType, order>&>( _ref ) ), eval( _eval ), nextOrder_( search, ref, _eval ), sumVecs_( search.sumVecs() ) {
        for ( uint a = 0; a < maxDepth; a++ ) {
          leftOffset[ a ] = &( sumVecs_[ a ][ 0 - 1 ] );

          if ( ( uint ) ( 1 << a ) < ( uint ) sumVecs_[ 0 ].size() ) {
            rightOffset[ a ] = &( sumVecs_[ a ][ ( 1 << a ) - 1 ] );
          } else {
            rightOffset[ a ] = 0;
          }
        }

        firstItem = &sumVecs_[ 0 ][ 0 ];

        for ( int a = 0; a < maxDepth; a++ )
          finalRight[ a ] = &( sumVecs_[ a ][ sumVecs_[ a ].size() - 1 ] );

        leftGetter = typename SumGroup<maxDepth, SumType>::VecSumGetter( leftOffset );
        rightGetter = typename SumGroup<maxDepth, SumType>::VecSumGetter( rightOffset );
      }

      operator bool() {
        return ( bool ) rightOffset[ 0 ];
      }

      void next() {
        for ( int a = 0; a < maxDepth; a++ ) {
          ++leftOffset[ a ];
          if ( !rightOffset[ a ] )
            break;
          ++rightOffset[ a ];
          if ( rightOffset[ a ] > finalRight[ a ] )
            rightOffset[ a ] = 0;
        }
        nextOrder_.next();
      }

      SumType leftDiff() {
        if ( leftOffset[ 0 ] < firstItem ) {
          return ref.leftMarker.sumDifference( ZeroGetter() ) + nextOrder_.leftDiff();
        }
        return ref.leftMarker.sumDifference( leftGetter ) + nextOrder_.leftDiff();
      }

      SumType rightDiff() {
        return ref.rightMarker.sumDifference( rightGetter ) + nextOrder_.rightDiff();
      }

      uint difPos() {
        return ( ((unsigned long)rightOffset[ 0 ] ) - (unsigned long)firstItem ) / sizeof( uint* );
      }
    };

    template <class Evaluator>
    int findReference( const SumReference<maxDepth, SumType, order>& ref, Evaluator& eval ) {
      if ( !ref.isValid() )
        return -1;
      if ( sumVecs_[ 0 ].size() < 2 )
        return -1;

      FindReferenceProgress<Evaluator> prog( *this, ref, eval );

      while ( prog ) {
        SumType leftDif = 0; //prog.leftDiff();
        SumType rightDif = prog.rightDiff();
        uint difPos = prog.difPos();

        eval( leftDif, rightDif, difPos );

        prog.next();
      }
      return eval.result();
    }

    ///This uses the default-evaluator
    inline int findReference( const SumReference<maxDepth, SumType, order>& ref ) {
      SimpleEvaluator eval;
      return findReference( ref, eval );
    }
};


template <int maxDepth, typename SumType, int order>
class DifferenceSumSearch<maxDepth, SumType, order, false> {
    typedef SumSearch<maxDepth, SumType, order> MySearch;
    typedef SumSearch < maxDepth, SumType, order + 1 > MyPrevSearch;
    MySearch search_;

    static string makeDifference( const string& str ) {
      string ret = str;
      uint len = ret.length();
      char last = 0;
      if ( !ret.empty() )
        last = ret[ 0 ];

      for ( uint n = 0; n < len; n++ ) {
        char r = ret[ n ];
        ret[ n ] = r - last;
        last = r;
      }
      return ret;
    }

  public:
    DifferenceSumSearch( const string& text ) : search_( makeDifference( text ) ) {}

    template <class Type>
    void fillReference( Type& t, int position ) {
      search_.fillReference( t.nextOrder_.reference_, position );
    }

    template <class PrevSearch, class Evaluator>
    struct FindRefProgress {
      typename MySearch:: template FindReferenceProgress<Evaluator>
      find;

      FindRefProgress( PrevSearch& search, const SumReference < maxDepth, SumType, order + 1 > & _ref, Evaluator& _eval ) : find( search.nextOrder_.search_, _ref.nextOrder_.reference_, _eval )
      {}

      SumType leftDiff() {
        return find.leftDiff();
      }
      SumType rightDiff() {
        return find.rightDiff();
      }

      void next() {
        find.next();
      }
    };
};


} //FuzzySearch

using namespace FuzzySearch;


// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
