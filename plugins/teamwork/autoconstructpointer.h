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

#ifndef AUTO_CONSTRUCT_POINTER_H
#define AUTO_CONSTRUCT_POINTER_H

/**This is a pointer-class that can be used as a simple wrapper to have the same safety
*as using normal element-classes, but allows the classes to be forward-declared.  Also
*it features serialization.
*It cannot be zero!  */

class NormalSerialization;

template <class T, class Serialization = NormalSerialization>
class AutoConstructPointer {
    T* m_data;

  public:

    AutoConstructPointer() {
      m_data = new T();
    }

    AutoConstructPointer( const AutoConstructPointer& rhs ) {
      m_data = new T( *rhs );
    }

    AutoConstructPointer& operator = ( const AutoConstructPointer& rhs ) {
      return * this;
    }

    template <class Param>
    AutoConstructPointer( Param& param ) {
      m_data = new T( param );
    }

    template <class Param>
    AutoConstructPointer( const Param& param ) {
      m_data = new T( param );
    }
    template <class Param, class Param2>
    AutoConstructPointer( Param& param, Param2& param2 ) {
      m_data = new T( param, param2 );
    }

    ~AutoConstructPointer() {
      delete m_data;
    }

    void reset( T* t ) {
      m_data = t;
    }

    T& operator *() {
      return * m_data;
    }

    const T& operator *() const {
      return * m_data;
    }

    T* data() {
      return m_data;
    }

    operator const T* () const {
      return m_data;
    }

    operator T* () {
      return m_data;
    }

    const T* data() const {
      return m_data;
    }

    T* operator -> () {
      return m_data;
    }

    const T* operator -> () const {
      return m_data;
    }

    template <class Archive>
    void serialize( Archive& arch, uint /*version*/ ) {
      arch & ( *data() );
    }
};


#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
