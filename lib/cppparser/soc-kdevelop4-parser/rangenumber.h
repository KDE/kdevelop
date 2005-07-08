/* This file is part of KDevelop
    Copyright (C) 2005 John Tapsell <john.tapsell@kdemail.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef RANGENUMBER_H
#define RANGENUMBER_H

#include <iostream>

template <class T>
class SingleRange
{
  public:
    /** Note start <= end always */
    T m_start;
    /** Note start <= end always */
    T m_end;
    /** If this is false, then start and end have undefined values. */
    bool m_valid;

  public:
    SingleRange<T>() { m_valid = false; }
    SingleRange<T>(const T& start, const T& end, bool valid=true) {
      m_valid = valid;
      if(start < end) { 
        m_start = start;
        m_end = end;
      } else { 
        m_start = end;
        m_end = start;
      }
    }    
    
    SingleRange operator = (const SingleRange &range) {
      m_valid = range.m_valid;
      m_start = range.m_start;
      m_end = range.m_end;
      return *this;
    }
    
    /** Check whether this and another range are equal.
     */
    SingleRange<bool> operator == (const SingleRange &range) const {
      if(!m_valid || !range.m_valid) return SingleRange<bool>();

      if(m_start == m_end && m_start == range.m_start && m_start == range.m_end) 
        return SingleRange<bool>(true, true);
      else if(m_end < range.m_start || range.m_end < m_start) //can never be true
        return SingleRange<bool>(false, false);
      else
        return SingleRange<bool>(false, true);
    }
    
    SingleRange<bool> operator != (const SingleRange &range) const {
      if(!m_valid || !range.m_valid) return SingleRange<bool>();

      if(m_start == m_end && m_start == range.m_start && m_start == range.m_end) 
        return SingleRange<bool>(false, false);
      else if(m_end < range.m_start || range.m_end < m_start) //can never be true
        return SingleRange<bool>(true, true);
      else
        return SingleRange<bool>(false, true);
    }

    SingleRange<bool> operator > (const SingleRange &range) const {
      if(!m_valid || !range.m_valid) return SingleRange<bool>();

      if(m_start > range.m_end ) 
        return SingleRange<bool>(true, true);
      else if(m_end <= range.m_start)
        return SingleRange<bool>(false, false);
      else
        return SingleRange<bool>(false, true);
    }
 
    SingleRange<bool> operator < (const SingleRange &range) const {
      if(!m_valid || !range.m_valid) return SingleRange<bool>();

      if(m_end < range.m_start ) 
        return SingleRange<bool>(true, true);
      else if(m_start >= range.m_end)
        return SingleRange<bool>(false, false);
      else
        return SingleRange<bool>(false, true);
   }
    
   SingleRange<bool> operator >= (const SingleRange &range) const {
      if(!m_valid || !range.m_valid) return SingleRange<bool>();

      if(m_start >= range.m_end ) 
        return SingleRange<bool>(true, true);
      else if(m_end < range.m_start)
        return SingleRange<bool>(false, false);
      else
        return SingleRange<bool>(false, true);
    }
 
    SingleRange<bool> operator <= (const SingleRange &range) const {
      if(!m_valid || !range.m_valid) return SingleRange<bool>();

      if(m_end <= range.m_start ) 
        return SingleRange<bool>(true, true);
      else if(m_start > range.m_end)
        return SingleRange<bool>(false, false);
      else
        return SingleRange<bool>(false, true);
    }

    SingleRange<bool> operator || (const SingleRange &range) const {
      if(!m_valid || !range.m_valid) return SingleRange<bool>();
      
      bool canbefalse = ( m_start <= 0 && m_end >= 0 ) && (range.m_start <= 0 && range.m_end >= 0);
      bool canbetrue  = !( m_start == 0 && m_end == 0 && range.m_start == 0 && range.m_end == 0);
      
      return SingleRange<bool>( !canbefalse, canbetrue );
      
    }
    
    SingleRange<bool> operator && (const SingleRange &range) const {
      if(!m_valid || !range.m_valid) return SingleRange<bool>();
      
      bool canbefalse = ( m_start <= 0 && m_end >= 0 ) || (range.m_start <= 0 && range.m_end >= 0);      
      bool canbetrue  = ( m_start != 0 || m_end != 0 ) && (range.m_start != 0 || range.m_end != 0);
      
      return SingleRange<bool>( !canbefalse, canbetrue );
      
    }
   
    SingleRange<bool> operator ! () const {
      if(!m_valid) return SingleRange<bool>();
      
      if(m_start == m_end)
        return SingleRange<bool>( !m_start, !m_start );
      else if(m_start <= 0 && m_end >= 0)
        return SingleRange<bool>( false, true );
      else
        return SingleRange<bool>( false, false);
      
    }
 
    template <class T2>
    SingleRange<T2> doif (const SingleRange<T2> &range1, const SingleRange<T2> &range2) const {
      if(!m_valid) return SingleRange<T2>();
      
      if(m_start == m_end && m_start)
        return range1;
      else if(m_start == m_end)
        return range2;
      else if(m_start <= 0 && m_end >= 0)
        return range1.rangeunion(range2);
      else
        return range2;
      
    }
 
    SingleRange rangeunion(const SingleRange &range) const {
      if(!m_valid || !range.m_valid) return SingleRange();

      return  SingleRange( (m_start < range.m_start)?m_start:range.m_start, m_end > range.m_end?m_end:range.m_end);
    }
    
    SingleRange operator +(const SingleRange &range) const {
      if(!m_valid || !range.m_valid) return SingleRange();
      
      return SingleRange(m_start+range.m_start, m_end+range.m_end);
    }
 
    SingleRange operator - (const SingleRange &range) const {
      if(!m_valid || !range.m_valid) return SingleRange();
      
      return SingleRange(m_start-range.m_end, m_end-range.m_start);
    }

    SingleRange operator * (const SingleRange &range) const {
      if(!m_valid || !range.m_valid) return SingleRange();
      
      T min = m_start*range.m_start;
      T max = m_start*range.m_start;

      T temp = m_start*range.m_end;
      if(temp < min) min = temp;
      if(temp > max) max = temp;
      temp = m_end*range.m_start;
      if(temp < min) min = temp;
      if(temp > max) max = temp;
      temp = m_end*range.m_end;
      if(temp < min) min = temp;
      if(temp > max) max = temp;

      return SingleRange(min, max);
    }
  
    SingleRange operator / (const SingleRange &range) const {
      if(!m_valid || !range.m_valid) return SingleRange();

      if(range.m_start <= 0 && range.m_end >= 0) return SingleRange();
 
      T min = m_start/range.m_start;
      T max = m_start/range.m_start;

      T temp = m_start/range.m_end;
      if(temp < min) min = temp;
      if(temp > max) max = temp;
      temp = m_end/range.m_start;
      if(temp < min) min = temp;
      if(temp > max) max = temp;
      temp = m_end/range.m_end;
      if(temp < min) min = temp;
      if(temp > max) max = temp;

      return SingleRange(min, max);
     
    }

    SingleRange operator % (const SingleRange &range) const {
      if(!m_valid || !range.m_valid) return SingleRange();

      if(range.m_start <= 0 && range.m_end >= 0) return SingleRange();

      if(m_end < range.m_start) return SingleRange(m_start, m_end);

      if(range.m_start == range.m_end && (m_end - m_start) < range.m_start ) { 
        T min = m_start % range.m_start;
        T max = m_end % range.m_start;

        if(min <= max) return SingleRange(min, max); 
      }
      
      return SingleRange((T)0, range.m_end - (T)1); 
    }

    SingleRange operator += (const SingleRange &range) {
      *this = *this + range;
      return *this;
    }
    SingleRange operator -= (const SingleRange &range) {
      *this = *this - range;
      return *this;
    }
    SingleRange operator *= (const SingleRange &range) {
      *this = *this * range;
      return *this;
    }
    SingleRange operator /= (const SingleRange &range) {
      *this = *this / range;
      return *this;
    }
    SingleRange operator %= (const SingleRange &range) {
      *this = *this % range;
      return *this;
    }

    //FIXME Not sure how to code these.  Will come back to them.. maybe
/*    SingleRange operator &&= (const SingleRange &range) {            
      SingleRange<bool> result = *this && range;
      m_valid = result.m_valid;
      m_start = result.m_start;
      m_end = result.m_end;
      return *this;
    }

    SingleRange operator ||= (const SingleRange &range) {
      SingleRange<bool> result = *this || range;
      m_valid = result.m_valid;
      m_start = result.m_start;
      m_end = result.m_end;
      return *this;
    }*/

    
    void toString() {
        if(!m_valid)
            std::cout << "(invalid)";
        else if(m_start == m_end)
            std::cout << m_start;
        else
            std::cout << "(" << m_start << ", " << m_end << ")";
    }
   
};

/**
 *  This class 
 */
class RangeNumber 
{
  typedef enum { Bool, Int, Double } Type; //Put in order so that (a*b)  ends up the type with the highest numerical value
  private:
    Type m_type;
    
    SingleRange<int>  m_int_value;
    SingleRange<bool> m_bool_value;
    SingleRange<double> m_double_value;

  public:
    RangeNumber() : m_type(Int), m_bool_value(0,0, false) { };
    RangeNumber(const SingleRange<bool> &value) : m_type(Bool), m_bool_value(value) { };
    RangeNumber(const SingleRange<int> &value) : m_type(Int), m_int_value(value) { };
    RangeNumber(const SingleRange<double> &value) : m_type(Double), m_double_value(value) { };
    
    RangeNumber(bool start, bool end, bool valid=true) : m_type(Bool), m_bool_value(start, end, valid) { };
    RangeNumber(int start, int end, bool valid=true) : m_type(Int), m_int_value(start, end, valid) { };
    RangeNumber(double start, double end, bool valid=true) : m_type(Double), m_double_value(start, end, valid) { };
    
    RangeNumber cast_to_highest(const RangeNumber &range) const {
      if(m_type >= range.m_type) return *this;

      if(range.m_type == Double && m_type == Int) return RangeNumber((double) m_int_value.m_start, (double) m_int_value.m_end);
      if(range.m_type == Double /*must be bool*/) return RangeNumber((double) m_bool_value.m_start, (double) m_bool_value.m_end);
      //Must now be range.m_type == Int and m_type == Bool
      return RangeNumber((int) m_bool_value.m_start, (int) m_bool_value.m_end);
    }

    RangeNumber cast_to_bool() const {
      if(m_type == Bool) return *this;
      if(m_type == Int) return RangeNumber((bool) !(m_int_value.m_start <=0 && m_int_value.m_end >= 0) /* false if it can be false */, (bool) (m_int_value.m_start != 0 || m_int_value.m_end != 0) /* can it be true*/);
      /*m_type == Double*/
      return RangeNumber((bool) !(m_double_value.m_start <=0 && m_double_value.m_end >= 0) /* false if it can be false */, (bool) (m_double_value.m_start != 0 || m_double_value.m_end != 0) /* can it be true*/);

    }

    RangeNumber operator +(const RangeNumber &range) const {
            RangeNumber r1 = cast_to_highest(range);
            RangeNumber r2 = range.cast_to_highest(*this);
            if(r1.m_type == Bool) 
              return RangeNumber( r1.m_bool_value + r2.m_bool_value );
            else if(r1.m_type == Int)
              return RangeNumber( r1.m_int_value + r2.m_int_value );
            else
              return RangeNumber( r1.m_double_value + r2.m_double_value );
                    
    }
    RangeNumber operator -(const RangeNumber &range) const {
            RangeNumber r1 = cast_to_highest(r1);
            RangeNumber r2 = range.cast_to_highest(*this);
            if(r1.m_type == Bool) 
              return RangeNumber( r1.m_bool_value - r2.m_bool_value );
            else if(r1.m_type == Int)
              return RangeNumber( r1.m_int_value - r2.m_int_value );
            else
              return RangeNumber( r1.m_double_value - r2.m_double_value );
                    
    }
    RangeNumber operator /(const RangeNumber &range) const {
            RangeNumber r1 = cast_to_highest(r1);
            RangeNumber r2 = range.cast_to_highest(*this);
            if(r1.m_type == Bool) 
              return RangeNumber( r1.m_bool_value / r2.m_bool_value );
            else if(r1.m_type == Int)
              return RangeNumber( r1.m_int_value / r2.m_int_value );
            else
              return RangeNumber( r1.m_double_value / r2.m_double_value );
                    
    }
    RangeNumber operator *(const RangeNumber &range) const { 
            RangeNumber r1 = cast_to_highest(r1);
            RangeNumber r2 = range.cast_to_highest(*this);
            if(r1.m_type == Bool) 
              return RangeNumber( r1.m_bool_value * r2.m_bool_value );
            else if(r1.m_type == Int)
              return RangeNumber( r1.m_int_value * r2.m_int_value );
            else
              return RangeNumber( r1.m_double_value * r2.m_double_value );
                    
    }
    RangeNumber operator %(const RangeNumber &range) const {
            RangeNumber r1 = cast_to_highest(r1);
            RangeNumber r2 = range.cast_to_highest(*this);
            if(r1.m_type == Bool) 
              return RangeNumber( r1.m_bool_value % r2.m_bool_value );
            else if(r1.m_type == Int)
              return RangeNumber( r1.m_int_value % r2.m_int_value );
            else
              return RangeNumber(); //You can't use doubles on % 
                    
    }
//    RangeNumber operator !() const { return RangeNumber( !m_value ); }
//    RangeNumber operator !=(const RangeNumber &range) const { return RangeNumber( m_value != range.m_value ); }
//    RangeNumber operator ==(const RangeNumber &range) const { return RangeNumber( m_value == range.m_value ); }

//    RangeNumber operator <=(const RangeNumber &range) const { return RangeNumber( m_value <= range.m_value; ); }
//    RangeNumber operator >=(const RangeNumber &range) const { return RangeNumber( m_value >= range.m_value; ); }
//    RangeNumber operator <(const RangeNumber &range) const { return RangeNumber( m_value < range.m_value; ); }
//    RangeNumber operator >(const RangeNumber &range) const { return RangeNumber( m_value > range.m_value; ); }

    RangeNumber operator =(const RangeNumber &range) {
      
      if(m_type == Bool) {    
        if(range.m_type == Bool) { m_bool_value.m_start = range.m_bool_value.m_start; m_bool_value.m_end = range.m_bool_value.m_end; m_bool_value.m_valid = range.m_bool_value.m_valid; }
        if(range.m_type == Int) { m_bool_value.m_start = range.m_int_value.m_start; m_bool_value.m_end = range.m_int_value.m_end; m_bool_value.m_valid = range.m_int_value.m_valid; }
        if(range.m_type == Double) { m_bool_value.m_start = range.m_double_value.m_start; m_bool_value.m_end = range.m_double_value.m_end; m_bool_value.m_valid = range.m_double_value.m_valid; }
      } else if(m_type == Int) {
        if(range.m_type == Bool) { m_int_value.m_start = range.m_bool_value.m_start; m_int_value.m_end = range.m_bool_value.m_end; m_int_value.m_valid = range.m_bool_value.m_valid;}
        if(range.m_type == Int) { m_int_value.m_start = range.m_int_value.m_start; m_int_value.m_end = range.m_int_value.m_end; m_int_value.m_valid = range.m_int_value.m_valid;}
        if(range.m_type == Double) { m_int_value.m_start = (int)range.m_double_value.m_start; m_int_value.m_end = (int)range.m_double_value.m_end; m_int_value.m_valid = range.m_double_value.m_valid; }
      } else if(m_type == Double) {
        if(range.m_type == Bool) { m_double_value.m_start = (double)range.m_bool_value.m_start; m_double_value.m_end = (double)range.m_bool_value.m_end; m_double_value.m_valid = range.m_bool_value.m_valid; }
        if(range.m_type == Int) { m_double_value.m_start = (double)range.m_int_value.m_start; m_double_value.m_end = (double)range.m_int_value.m_end; m_double_value.m_valid = range.m_int_value.m_valid; }
        if(range.m_type == Double) { m_double_value.m_start = range.m_double_value.m_start; m_double_value.m_end = range.m_double_value.m_end; m_double_value.m_valid = range.m_double_value.m_valid; }
      }
      return *this;
    }
    void display() { 
      if(m_type == Bool) m_bool_value.toString();
      else if (m_type == Int) m_int_value.toString();
      else m_double_value.toString();
    }
    RangeNumber doif(const RangeNumber &range1, const RangeNumber &range2) const { 

      RangeNumber r1 = cast_to_bool();
      RangeNumber r2 = range1.cast_to_highest(range2);
      RangeNumber r3 = range2.cast_to_highest(range1);
      
      if(r2.m_type == Bool) return RangeNumber( r1.m_bool_value.doif(r2.m_bool_value, r3.m_bool_value));
      if(r2.m_type == Int) return RangeNumber( r1.m_bool_value.doif(r2.m_int_value, r3.m_int_value));
      /*r2.m_type == Double)*/ return RangeNumber( r1.m_bool_value.doif(r2.m_double_value, r3.m_double_value));
    }
};



#endif // RANGENUMBER_H

// vim:ts=4:et
//
