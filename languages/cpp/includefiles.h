#ifndef INCLUDE_FILES_H
#define INCLUDE_FILES_H

#include<ksharedptr.h>
#include<hashedstring.h>

class HashedStringSetData;

///This is a reference-counting string-set optimized for fast lookup of hashed strings
class HashedStringSet {
public:
  HashedStringSet();
  
  ~HashedStringSet();
  
  ///Constructs a string-set from one single file
  HashedStringSet( const HashedString& file );
  
  HashedStringSet( const HashedStringSet& rhs );
  
  HashedStringSet& operator = ( const HashedStringSet& rhs );
  ///@return whether the given file-name was included
  bool operator[] ( const HashedString& rhs ) const;

  void insert( const HashedString& str );

  ///Returns true if all files that are part of this set are also part of the given set
  HashedStringSet& operator +=( const HashedStringSet& );
  
  ///intersection-test
  bool operator <= ( const HashedStringSet& rhs ) const;

  bool operator == ( const HashedStringSet& rhs ) const;
private:
  void makeDataPrivate();
  KSharedPtr<HashedStringSetData> m_data; //this implies some additional cost because KShared's destructor is virtual. Maybe change that by copying KShared without the virtual destructor.
  friend HashedStringSet operator + ( const HashedStringSet& lhs, const HashedStringSet& rhs );
};

HashedStringSet operator + ( const HashedStringSet& lhs, const HashedStringSet& rhs );

typedef HashedStringSet IncludeFiles;

#endif
