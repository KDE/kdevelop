/*
   Copyright (C) 1998, 1999 Jochen Wilhelmy
                            digisnap@cs.tu-berlin.de

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

#ifndef _KWTEXTLINE_H_
#define _KWTEXTLINE_H_

#include <stdlib.h>

#include <qstring.h>
#include <qvaluelist.h>

#include <ksharedptr.h>

#define FASTLIST 1
#ifdef FASTLIST
/**
  FastValueList: QValueList, but with a faster at() like QList
  FVPrivate is needed so that "const" functions can change the
  current position
*/
template<class T>
class FVPrivate
{
public:
    int curpos;
    typedef QValueListConstIterator<T> Iterator;
    Iterator curit;

    FVPrivate() { curpos=-1; };
};

template<class T>
class FastValueList : public QValueList<T>
{
public:
    typedef QValueListIterator<T> Iterator;
    typedef QValueListConstIterator<T> ConstIterator;
protected:
    FVPrivate<T> *fvp;

    Iterator fastat( uint i ) {
        uint num=count();
        if (i>=num) {return end();}
        if (fvp->curpos<0) { fvp->curpos=0; fvp->curit=begin(); }
        uint curpos=(uint) fvp->curpos;
        Iterator curit(fvp->curit.node);
        if (curpos==i) return curit;

        int diff=i-curpos;
        bool forward;
        if (diff<0) diff=-diff;
        if (((uint)diff < i) && ((uint)diff < num-i)) { // start from current node
                forward=i > (uint)curpos;
        } else if (i < num - i) { // start from first node
                curit=begin(); diff=i; forward=TRUE;
        } else {                  // start from last node
                curit=fromLast(); diff=num - i - 1;
                if (diff<0) diff=0;
                forward=FALSE;
        }
        if (forward) {
                while(diff--) curit++;
        } else {
                while(diff--) curit--;
        }
        fvp->curpos=i; fvp->curit=curit;
        return curit;
    }
    ConstIterator fastat( uint i ) const {
        uint num=count();
        if (i>=num) {return end();}
        if (fvp->curpos<0) { fvp->curpos=0; fvp->curit=begin(); }
        uint curpos=(uint) fvp->curpos;
        ConstIterator curit=fvp->curit;
        if (curpos==i) return curit;

        int diff=i-curpos;
        bool forward;
        if (diff<0) diff=-diff;
        if (((uint)diff < i) && ((uint)diff < num-i)) { // start from current node
                forward=i > (uint)curpos;
        } else if (i < num - i) { // start from first node
                curit=begin(); diff=i; forward=TRUE;
        } else {                  // start from last node
                curit=fromLast(); diff=num - i - 1;
                if (diff<0) diff=0;
                forward=FALSE;
        }
        if (forward) {
                while(diff--) curit++;
        } else {
                while(diff--) curit--;
        }
        fvp->curpos=i; fvp->curit=curit;
        return curit;
    }

public:
    FastValueList() : QValueList<T>()
    { fvp=new FVPrivate<T>(); }
    FastValueList(const FastValueList<T>& l) : QValueList<T>(l)
    { fvp=new FVPrivate<T>(); }
    ~FastValueList() { delete fvp; }

    Iterator insert( Iterator it, const T& x ) {
      fvp->curpos=-1; return QValueList<T>::insert(it, x);
    }

    Iterator append( const T& x ) {
      fvp->curpos=-1; return QValueList<T>::append( x );
    }
    Iterator prepend( const T& x ) {
      fvp->curpos=-1; return QValueList<T>::prepend( x );
    }

    Iterator remove( Iterator it ) {
      fvp->curpos=-1; return QValueList<T>::remove( it );
    }
    void remove( const T& x ) {
      fvp->curpos=-1; QValueList<T>::remove( x );
    }

    T& operator[] ( uint i ) { detach(); return fastat(i); }
    const T& operator[] ( uint i ) const { return *fastat(i); }
    Iterator at( uint i ) { detach(); return fastat(i); }
    ConstIterator at( uint i ) const { return ConstIterator( fastat(i) ); }
};
#endif


/**
  The TextLine represents a line of text. A text line that contains the
  text, an attribute for each character, an attribute for the free space
  behind the last character and a context number for the syntax highlight.
  The attribute stores the index to a table that contains fonts and colors
  and also if a character is selected.
*/
class TextLine : public KShared
{
public:
    typedef KSharedPtr<TextLine> Ptr;
#ifdef FASTLIST
    typedef FastValueList<Ptr> List;
#else
    typedef QValueList<Ptr> List;
#endif
public:
    /**
      Creates an empty text line with given attribute and syntax highlight
      context
    */
    TextLine(int attribute = 0, int context = 0);
    ~TextLine();

    /**
      Returns the length
    */
    int length() const {return len;}
    /**
      Universal text manipulation method. It can be used to insert, delete
      or replace text.
    */
    void replace(int pos, int delLen, const QChar *insText, int insLen,
      uchar *insAttribs = 0L);

    /**
      Appends a string of length l to the textline
    */
    void append(const QChar *s, int l) {replace(len, 0, s, l);}
    /**
      Wraps the text from the given position to the end to the next line
    */
    void wrap(TextLine::Ptr nextLine, int pos);
    /**
      Wraps the text of given length from the beginning of the next line to
      this line at the given position
    */
    void unWrap(int pos, TextLine::Ptr nextLine, int len);
    /**
      Truncates the textline to the new length
    */
    void truncate(int newLen) {if (newLen < len) len = newLen;}
    /**
      Returns the position of the first character which is not a white space
    */
    int firstChar() const;
    /**
      Returns the position of the last character which is not a white space
    */
    int lastChar() const;
    /**
      Removes trailing spaces
    */
    void removeSpaces();
    /**
      Gets the char at the given position
    */
    QChar getChar(int pos) const;
    /**
      Gets the text. WARNING: it is not null terminated
    */
    QChar *getText() const {return text;}
    /**
      Gets a C-like null terminated string
    */
    const QChar *getString();

    /*
      Gets a null terminated pointer to first non space char
    */
    const QChar *firstNonSpace();
    /**
      Returns the x position of the cursor at the given position, which
      depends on the number of tab characters
    */
    int cursorX(int pos, int tabChars) const;
    /**
      Is the line starting with the given string
    */
    bool startingWith(QString& match);
    /**
      Is the line ending with the given string
    */
    bool endingWith(QString& match);

    /**
      Sets the attributes from start to end -1
    */
    void setAttribs(int attribute, int start, int end);
    /**
      Sets the attribute for the free space behind the last character
    */
    void setAttr(int attribute);
    /**
      Gets the attribute at the given position
    */
    int getAttr(int pos) const;
    /**
      Gets the attribute for the free space behind the last character
    */
    int getAttr() const;
    /**
      Gets the attribute, including the select state, at the given position
    */
    int getRawAttr(int pos) const;
    /**
      Gets the attribute, including the select state, for the free space
      behind the last character
    */
    int getRawAttr() const;

    /**
      Sets the syntax highlight context number
    */
    void setContext(int context);
    /**
      Gets the syntax highlight context number
    */
    int getContext() const;

    /**
      Sets the select state from start to end -1
    */
    void select(bool sel, int start, int end);
    /**
      Sets the select state from the given position to the end, including
      the free space behind the last character
    */
    void selectEol(bool sel, int pos);
    /**
      Toggles the select state from start to end -1
    */
    void toggleSelect(int start, int end);
    /**
      Toggles the select state from the given position to the end, including
      the free space behind the last character
    */
    void toggleSelectEol(int pos);
    /**
      Returns the number of selected characters
    */
    int numSelected() const;
    /**
      Returns if the character at the given position is selected
    */
    bool isSelected(int pos) const;
    /**
      Returns true if the free space behind the last character is selected
    */
    bool isSelected() const;
    /**
      Finds the next selected character, starting at the given position
    */
    int findSelected(int pos) const;
    /**
      Finds the next unselected character, starting at the given position
    */
    int findUnselected(int pos) const;
    /**
      Finds the previous selected character, starting at the given position
    */
    int findRevSelected(int pos) const;
    /**
      Finds the previous unselected character, starting at the given position
    */
    int findRevUnselected(int pos) const;

    /**
      Marks the text from the given position and length as found
    */
    void markFound(int pos, int l);
    /**
      Removes the found marks
    */
    void unmarkFound();

  protected:
    /**
      Length of the text line
    */
    int len;
    /**
      Memory Size of the text line
    */
    int size;
    /**
      The text
    */
    QChar *text;
    /**
      The attributes
    */
    uchar *attribs;
    /**
      The attribute of the free space behind the end
    */
    uchar attr;
    /**
      The syntax highlight context
    */
    int ctx;
};

//text attribute constants
const int taSelected = 0x40;
const int taFound = 0x80;
const int taSelectMask = taSelected | taFound;
const int taAttrMask = ~taSelectMask & 0xFF;
const int taShift = 6;

#endif //KWTEXTLINE_H

