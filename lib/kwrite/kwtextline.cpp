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

#include "kwtextline.h"

TextLine::TextLine(int attribute, int context)
  : len(0), size(0), text(0L), attribs(0L), attr(attribute), ctx(context) {
}

TextLine::~TextLine() {
  delete [] text;
  delete [] attribs;
}

void TextLine::replace(int pos, int delLen, const QChar *insText, int insLen,
  uchar *insAttribs) {

  int newLen, i, z;
  uchar newAttr;
  QChar *newText;
  uchar *newAttribs;

  //find new length
  newLen = len - delLen;
  if (newLen < pos) newLen = pos;
  newLen += insLen;
//printf("len %d, pos %d, delLen %d, insLen %d, newLen %d\n",
//  len, pos, delLen, insLen, newLen);

  newAttr = (pos < len) ? attribs[pos] : attr;

  if (newLen > size) {
    //reallocate data
    size = size*3 >> 1;
    if (size < newLen) size = newLen;
    size = (size + 15) & (~15);

    newText = new QChar[size];
    newAttribs = new uchar[size];
  ASSERT(newText);
  ASSERT(newAttribs);

    i = QMIN(len, pos);
    for (z = 0; z < i; z++) {
      newText[z] = text[z];
      newAttribs[z] = attribs[z];
    }
  } else {
    newText = text;
    newAttribs = attribs;
  }

  //fill up with spaces and attribute
  for (z = len; z < pos; z++) {
    newText[z] = ' ';
    newAttribs[z] = attr;
  }

  i = (insLen - delLen);

  // realloc has gone - no idea why it was here
  if (i != 0) {
    if (i <= 0) {
      //text to replace longer than new text
      for (z = pos + delLen; z < len; z++) {
        newText[z + i] = text[z];
        newAttribs[z + i] = attribs[z];
      }
    } else {
      //text to replace shorter than new text
      for (z = len -1; z >= pos + delLen; z--) {
        newText[z + i] = text[z];
        newAttribs[z + i] = attribs[z];
      }
    }
  }

  if (newText != text) { //delete old stuff on realloc
    delete [] text;
    delete [] attribs;
    text = newText;
    attribs = newAttribs;
  }

  if (insAttribs == 0L) {
    for (z = 0; z < insLen; z++) {
      text[pos + z] = insText[z];
      attribs[pos + z] = newAttr;
    }
  } else {
    for (z = 0; z < insLen; z++) {
      text[pos + z] = insText[z];
      attribs[pos + z] = insAttribs[z];
    }
  }
  len = newLen;
}

void TextLine::wrap(TextLine::Ptr nextLine, int pos) {
  int l;

  l = len - pos;
  if (l > 0) {
    nextLine->replace(0, 0, &text[pos], l, &attribs[pos]);
    attr = attribs[pos];
    len = pos;
  }
}

void TextLine::unWrap(int pos, TextLine::Ptr nextLine, int len) {

  replace(pos, 0, nextLine->text, len, nextLine->attribs);
  attr = nextLine->getRawAttr(len);
  nextLine->replace(0, len, 0L, 0);
}

int TextLine::firstChar() const {
  int z = 0;

  while (z < len && text[z].isSpace()) z++;
  return (z < len) ? z : -1;
}

int TextLine::lastChar() const {
  int z = len;

  while (z > 0 && text[z - 1].isSpace()) z--;
  return z;
}

void TextLine::removeSpaces() {

  while (len > 0 && text[len - 1].isSpace()) len--;
}

QChar TextLine::getChar(int pos) const {
  if (pos < len) return text[pos];
  return ' ';
}
const QChar *TextLine::firstNonSpace()
{
  const QChar *ptr=getString();
  int first=firstChar();
  return (first > -1) ? ptr+first : ptr;
}


const QChar* TextLine::getString() {
  QChar ch = QChar('\0');  // this hack makes the string null terminated
  replace(len, 0, &ch, 1); //  only syntax highlight needs it
  len--;
//  resize(len+1);
//  text[len] = '\0';
  return text;
}

bool TextLine::startingWith(QString& match) {

  int matchLen = match.length();

  // Get the first chars of the textline
  QString firstChars = QString(text, len).left(matchLen);

  return (firstChars == match);
}

bool TextLine::endingWith(QString& match) {

  int matchLen = match.length();

  // Get the last chars of the textline
  QString lastChars = QString(text, len).right(matchLen);

  return (lastChars == match);
}

int TextLine::cursorX(int pos, int tabChars) const {
  int l, x, z;

  l = (pos < len) ? pos : len;
  x = 0;
  for (z = 0; z < l; z++) {
    if (text[z] == '\t') x += tabChars - (x % tabChars); else x++;
  }
  x += pos - l;
  return x;
}

void TextLine::setAttribs(int attribute, int start, int end) {
  int z;

  if (end > len) end = len;
  for (z = start; z < end; z++) attribs[z] = (attribs[z] & taSelectMask) | attribute;
}

void TextLine::setAttr(int attribute) {
  attr = (attr & taSelectMask) | attribute;
}

int TextLine::getAttr(int pos) const {
  if (pos < len) return attribs[pos] & taAttrMask;
  return attr & taAttrMask;
}

int TextLine::getAttr() const {
  return attr & taAttrMask;
}

int TextLine::getRawAttr(int pos) const {
  if (pos < len) return attribs[pos];
  return (attr & taSelectMask) ? attr : attr | 256;
}

int TextLine::getRawAttr() const {
  return attr;
}

void TextLine::setContext(int context) {
  ctx = context;
}

int TextLine::getContext() const {
  return ctx;
}


void TextLine::select(bool sel, int start, int end) {
  int z;

  if (end > len) end = len;
  if (sel) {
    for (z = start; z < end; z++) attribs[z] |= taSelected;
  } else {
    for (z = start; z < end; z++) attribs[z] &= ~taSelected;
  }
}

void TextLine::selectEol(bool sel, int pos) {
  int z;

  if (sel) {
    for (z = pos; z < len; z++) attribs[z] |= taSelected;
    attr |= taSelected;
  } else {
    for (z = pos; z < len; z++) attribs[z] &= ~taSelected;
    attr &= ~taSelected;
  }
}


void TextLine::toggleSelect(int start, int end) {
  int z;

  if (end > len) end = len;
  for (z = start; z < end; z++) attribs[z] = attribs[z] ^ taSelected;
}


void TextLine::toggleSelectEol(int pos) {
  int z;

  for (z = pos; z < len; z++) attribs[z] = attribs[z] ^ taSelected;
  attr = attr ^ taSelected;
}


int TextLine::numSelected() const {
  int z, n;

  n = 0;
  for (z = 0; z < len; z++) if (attribs[z] & taSelected) n++;
  return n;
}

bool TextLine::isSelected(int pos) const {
  if (pos < len) return (attribs[pos] & taSelected);
  return (attr & taSelected);
}

bool TextLine::isSelected() const {
  return (attr & taSelected);
}

int TextLine::findSelected(int pos) const {
  while (pos < len && attribs[pos] & taSelected) pos++;
  return pos;
}

int TextLine::findUnselected(int pos) const {
  while (pos < len && !(attribs[pos] & taSelected)) pos++;
  return pos;
}

int TextLine::findRevSelected(int pos) const {
  while (pos > 0 && attribs[pos - 1] & taSelected) pos--;
  return pos;
}

int TextLine::findRevUnselected(int pos) const {
  while (pos > 0 && !(attribs[pos - 1] & taSelected)) pos--;
  return pos;
}

void TextLine::markFound(int pos, int l) {
  int z;

  l += pos;
  if (l > len) l = len;
  for (z = pos; z < l; z++) attribs[z] |= taFound;
}

void TextLine::unmarkFound() {
  int z;
  for (z = 0; z < len; z++) attribs[z] &= ~taFound;
}

