/*
  $Id$

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

#include <sys/time.h>
#include <unistd.h>

#include <stdio.h>

#include <list>

#include <qobject.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qfont.h>
#include <qpainter.h>

#include <kcharsets.h>

#include "kwview.h"
#include "kwdoc.h"

//text attribute constants
const int taSelected = 0x40;
const int taFound = 0x80;
const int taSelectMask = taSelected | taFound;
const int taAttrMask = ~taSelectMask & 0xFF;
const int taShift = 6;

TextLine::TextLine(int attribute, int context)
  : len(0), size(0), text(0L), attribs(0L), attr(attribute), ctx(context) {
}

TextLine::~TextLine() {
  delete text;
  delete attribs;
}

void TextLine::replace(int pos, int delLen, const QChar *insText, int insLen,
  uchar *insAttribs) {

  int newLen, i, z;
  uchar newAttr;
  bool realloc;
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
    realloc = true;
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
    realloc = false;
    newText = text;
    newAttribs = attribs;
  }

  //fill up with spaces and attribute
  for (z = len; z < pos; z++) {
    newText[z] = ' ';
    newAttribs[z] = attr;
  }

  i = (insLen - delLen);
  if (i != 0 || realloc) {
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

  if (realloc) {
    delete text;
    delete attribs;
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

void TextLine::wrap(TextLine *nextLine, int pos) {
  int l;

  l = len - pos;
  if (l > 0) {
    nextLine->replace(0, 0, &text[pos], l, &attribs[pos]);
    attr = attribs[pos];
    len = pos;
  }
}

void TextLine::unWrap(int pos, TextLine *nextLine, int len) {

  replace(pos, 0, nextLine->text, len, nextLine->attribs);
  attr = nextLine->getRawAttr(len);
  nextLine->replace(0, len, 0L, 0);
}

void TextLine::removeSpaces() {

  while (len > 0 && text[len - 1] == ' ') len--;
}

int TextLine::firstChar() const {
  int z;

  z = 0;
  while (z < len && text[z].isSpace()) z++;
  return (z < len) ? z : -1;
}

QChar TextLine::getChar(int pos) const {
  if (pos < len) return text[pos];
  return ' ';
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

const QChar* TextLine::getString() {
  QChar ch = QChar('\0');
  replace(len, 0, &ch, 1);
  len--;
//  resize(len+1);
//  text[len] = '\0';
  return text;
}

const QChar* TextLine::getText() const {
  return text;
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


int TextLine::cursorX(int pos, int tabChars) const {
  int l, x, z;

  l = (pos < len) ? pos : len;
  x = 0;
  for (z = 0; z < l; z++) {
    if (text[z] == '\t') x += tabChars - (x % tabChars); else x++;
  }
  if (pos > len) x += pos - len;
  return x;
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

Attribute::Attribute() : font(), fm(font) {
}

void Attribute::setFont(const QFont &f) {
  font = f;
  fm = QFontMetrics(f);
//workaround for slow QFontMetrics::width(), QFont::fixedPitch() doesn't seem to work
  if ((fontWidth = fm.width('W')) != fm.width('i')) fontWidth = -1;
}


KWAction::KWAction(Action a, PointStruc &cursor, int len, const QString &text)
  : action(a), cursor(cursor), len(len), text(text) {
}
/*
void KWAction ::setData(int aLen, const QChar *aText, int aTextLen) {

  len = aLen;
  if (aTextLen > 0) {
    text = new QChar[aTextLen];
    memcpy((QChar *) text, aText, aTextLen*sizeof(QChar));
    textLen = aTextLen;
  } else {
    text = 0L;
    textLen = 0;
  }
} */

KWActionGroup::KWActionGroup(PointStruc &aStart, int type)
  : start(aStart), action(0L), undoType(type) {
}

KWActionGroup::~KWActionGroup() {
  KWAction *current, *next;

  current = action;
  while (current) {
    next = current->next;
    delete current;
    current = next;
  }
}

void KWActionGroup::insertAction(KWAction *a) {
  a->next = action;
  action = a;
}

QString KWActionGroup::typeName(int type)
{
  // return a short text description of the given undo group type suitable for a menu
  // not the lack of i18n's, the caller is expected to handle translation
  switch (type) {
  case ugPaste : return "Paste Text";
  case ugDelBlock : return "Selection Overwrite";
  case ugIndent : return "Indent";
  case ugUnindent : return "Unindent";
  case ugReplace : return "Text Replace";
  case ugSpell : return "Spell Check";
  case ugInsChar : return "Typing";
  case ugDelChar : return "Delete Text";
  case ugInsLine : return "New Line";
  case ugDelLine : return "Delete Line";
  }
  return "";
}

KWriteDoc::KWriteDoc(HlManager *hlManager, const QString &path)
  : QObject(0L), hlManager(hlManager), fName(path) {

  contents.setAutoDelete(true);

  colors[0] = white;
  colors[1] = darkBlue;
  colors[2] = black;
  colors[3] = black;
  colors[4] = white;

  highlight = 0L;
  tabChars = 8;

  newDocGeometry = false;
  readOnly = false;

  modified = false;

  undoList.setAutoDelete(true);
  undoState = 0;
  undoSteps = 50;

//  recordReset();

  pseudoModal = 0L;
  clear();
  clearFileName();

  setHighlight(0); //calls updateFontData()
  connect(hlManager,SIGNAL(changed()),SLOT(hlChanged()));

  newDocGeometry = false;
}

KWriteDoc::~KWriteDoc() {
  highlight->release();
}

TextLine *KWriteDoc::textLine(int line) {
//  if (line < 0) line = 0;
//  if (line >= (int) contents.count()) line = contents.count() -1;
  return contents.at(line);
}

int KWriteDoc::textLength(int line) {
  return contents.at(line)->length();
}

void KWriteDoc::tagLineRange(int line, int x1, int x2) {
  int z;

  for (z = 0; z < (int) views.count(); z++) {
    views.at(z)->tagLines(line, line, x1, x2);
  }
}

void KWriteDoc::tagLines(int start, int end) {
  int z;

  for (z = 0; z < (int) views.count(); z++) {
    views.at(z)->tagLines(start, end, 0, 0xffffff);
  }
}

void KWriteDoc::tagAll() {
  int z;

  for (z = 0; z < (int) views.count(); z++) {
    views.at(z)->tagAll();
  }
}

void KWriteDoc::readConfig(KConfig *config) {
  int z;
  char s[16];

  setTabWidth(config->readNumEntry("TabWidth",8));
  setUndoSteps(config->readNumEntry("UndoSteps",50));
  for (z = 0; z < 5; z++) {
    sprintf(s,"Color%d",z);
    colors[z] = config->readColorEntry(s,&colors[z]);
  }
}

void KWriteDoc::writeConfig(KConfig *config) {
  int z;
  char s[16];

  config->writeEntry("TabWidth",tabChars);
  config->writeEntry("UndoSteps",undoSteps);
  for (z = 0; z < 5; z++) {
    sprintf(s,"Color%d",z);
    config->writeEntry(s,colors[z]);
  }
}

void KWriteDoc::readSessionConfig(KConfig *config) {

  readConfig(config);
  fName = config->readEntry("URL");
  setHighlight(hlManager->nameFind(config->readEntry("Highlight")));
}

void KWriteDoc::writeSessionConfig(KConfig *config) {

  writeConfig(config);
  config->writeEntry("URL",fName);
  config->writeEntry("Highlight",highlight->name());
}

void KWriteDoc::registerView(KWriteView *view) {
  views.append(view);
}

void KWriteDoc::removeView(KWriteView *view) {
//  if (undoView == view) recordReset();
  views.remove(view);
}


int KWriteDoc::currentColumn(PointStruc &cursor) {
  return contents.at(cursor.y)->cursorX(cursor.x,tabChars);
}

void KWriteDoc::insert(VConfig &c, const QString &s) {
//  char b[256];
  int pos;
  QChar ch;
  QString buf;

  if (s.isEmpty()) return;

  recordStart(c, KWActionGroup::ugPaste);

  pos = 0;
  if (!(c.flags & cfVerticalSelect)) {
    do {
      ch = s[pos];
      if (ch.isPrint() || ch == '\t') {
        buf += ch;
      } else if (ch == '\n') {
        recordAction(KWAction::newLine, c.cursor);
        recordInsert(c.cursor, buf);
        c.cursor.x += buf.length();
        buf.truncate(0);
        c.cursor.y++;
        c.cursor.x = 0;
      }
      pos++;
/*      if ((unsigned char) *s >= 32 || *s == '\t') {
        b[pos] = *s;
        pos++;
        if (pos >= 256) {
          recordReplace(c.cursor, 0, b, pos);
          c.cursor.x += pos;
          pos = 0;
        }
      } else if (*s == '\n') {
        recordAction(KWAction::newLine, c.cursor);
        recordReplace(c.cursor, 0, b, pos);
        c.cursor.y++;
        c.cursor.x = 0;
        pos = 0;
      }
      s++;
      len--;
    } while (*s != 0 && len != 0);*/
    } while (pos < (int) s.length());
  } else {
    int xPos;

    xPos = textWidth(c.cursor);
    do {
      ch = s[pos];
      if (ch.isPrint() || ch == '\t') {
        buf += ch;
      } else if (ch == '\n') {
        recordInsert(c.cursor, buf);
        c.cursor.x += buf.length();
        buf.truncate(0);
        c.cursor.y++;
        if (c.cursor.y >= numLines())
          recordAction(KWAction::insLine, c.cursor);
        c.cursor.x = textPos(contents.at(c.cursor.y), xPos);
      }
      pos++;
/*
      if ((unsigned char) *s >= 32 || *s == '\t') {
        b[pos] = *s;
        pos++;
        if (pos >= 256) {
          recordReplace(c.cursor, 0, b, pos);
          c.cursor.x += pos;
          pos = 0;
        }
      } else if (*s == '\n') {
        recordReplace(c.cursor, 0, b, pos);
        c.cursor.y++;
        if (c.cursor.y >= numLines())
          recordAction(KWAction::insLine, c.cursor);
        c.cursor.x = textPos(contents.at(c.cursor.y), xPos);
        pos = 0;
      }
      s++;
      len--;
    } while (*s != 0 && len != 0);*/
    } while (pos < (int) s.length());
  }
  recordInsert(c.cursor, buf);
  c.cursor.x += buf.length();
/*  if (pos > 0) {
    recordReplace(c.cursor, 0, b, pos);
    c.cursor.x += pos;
  }*/
  recordEnd(c);
}

void KWriteDoc::insertFile(VConfig &c, QIODevice &dev) {
  char block[256];
  int len;
  char *s;
  QChar ch;
  QString buf;
  char last = '\0';

  recordStart(c, KWActionGroup::ugPaste);

  do {
    len = dev.readBlock(block, 256);
    s = block;
    while (len > 0) {
      ch = *s;
      if (ch.isPrint() || *s == '\t') {
        buf += ch;
/*        b[pos] = *s;
        pos++;
        if (pos >= 256) {
          recordReplace(c.cursor, 0, b, pos);
          c.cursor.x += pos;
          pos = 0;
        }*/
      } else if (*s == '\n' || *s == '\r') {
        if (last != '\r' || *s != '\n') {
          recordAction(KWAction::newLine, c.cursor);
          recordInsert(c.cursor, buf);
          buf.truncate(0);
//          recordReplace(c.cursor, 0, b, pos);
          c.cursor.y++;
          c.cursor.x = 0;
//          pos = 0;
        }
        last = *s;
      }
      s++;
      len--;
    }
  } while (s != block);
  recordInsert(c.cursor, buf);
  recordEnd(c);
}

void KWriteDoc::loadFile(QIODevice &dev) {
  TextLine *textLine;
  char block[256];
  int len;
  char *s;
  QChar ch;
  char last = '\0';

  clear();

  textLine = contents.getFirst();
//  pos = 0;
  do {
    len = dev.readBlock(block, 256);
    s = block;
    while (len > 0) {
      ch = *s;
      if (ch.isPrint() || *s == '\t') {
        textLine->append(&ch, 1);
/*        b[pos] = ch;
        pos++;
        if (pos >= 256) {
          textLine->append(b, pos);
          pos = 0;
        }*/
      } else if (*s == '\n' || *s == '\r') {
        if (last != '\r' || *s != '\n') {
//          textLine->append(b, pos);
//          pos = 0;
          textLine = new TextLine();
          contents.append(textLine);
          if (*s == '\r') eolMode = eolMacintosh;
        } else eolMode = eolDos;
        last = *s;
      }
      s++;
      len--;
    }
  } while (s != block);
//  textLine->append(b, pos);

//  updateLines();
}

void KWriteDoc::writeFile(QIODevice &dev) {
  TextLine *textLine;

  textLine = contents.first();
  do {
    QConstString str((QChar *) textLine->getText(), textLine->length());
    dev.writeBlock(str.string().latin1(), textLine->length());
//    dev.writeBlock(QString(textLine->getText(),
//      textLine->length()).latin1(), textLine->length());
    textLine = contents.next();
    if (!textLine) break;
    if (eolMode != eolUnix) dev.putch('\r');
    if (eolMode != eolMacintosh) dev.putch('\n');
  } while (true);
}

bool KWriteDoc::insertChars(VConfig &c, const QString &chars) {
  TextLine *textLine;
  int z, pos, l;
  bool onlySpaces;
  QChar ch;
  QString buf;

  textLine = contents.at(c.cursor.y);

  pos = 0;
  onlySpaces = true;
  for (z = 0; z < (int) chars.length(); z++) {
    ch = chars[z];
    if (ch == '\t' && c.flags & cfReplaceTabs) {
      l = tabChars - (textLine->cursorX(c.cursor.x, tabChars) % tabChars);
      while (l > 0) {
        buf.insert(pos, ' ');
        pos++;
        l--;
      }
    } else if (ch.isPrint() || ch == '\t') {
      buf.insert(pos, ch);
      pos++;
      if (ch != ' ') onlySpaces = false;
      if (c.flags & cfAutoBrackets) {
        if (ch == '(') buf.insert(pos, ')');
        if (ch == '[') buf.insert(pos, ']');
        if (ch == '{') buf.insert(pos, '}');
      }
      /*
      if (ch == 'ä') {
        strcpy(buf,"&auml;");
        l = z = 6;
      }
      if (ch == 'ö') {
        strcpy(buf,"&ouml;");
        l = z = 6;
      }
      if (ch == 'ü') {
        strcpy(buf,"&uuml;");
        l = z = 6;
      }
      if (ch == 'Ä') {
        strcpy(buf,"&Auml;");
        l = z = 6;
      }
      if (ch == 'Ö') {
        strcpy(buf,"&Ouml;");
        l = z = 6;
      }
      if (ch == 'Ü') {
        strcpy(buf,"&Uuml;");
        l = z = 6;
      }*/
      /*
      if (ch == 'ä') {
        strcpy(buf,"\"a");
        l = z = 2;
      }
      if (ch == 'ö') {
        strcpy(buf,"\"o");
        l = z = 2;
      }
      if (ch == 'ü') {
        strcpy(buf,"\"u");
        l = z = 2;
      }
      if (ch == 'Ä') {
        strcpy(buf,"\"A");
        l = z = 2;
      }
      if (ch == 'Ö') {
        strcpy(buf,"\"O");
        l = z = 2;
      }
      if (ch == 'Ü') {
        strcpy(buf,"\"U");
        l = z = 2;
      }
      if (ch == 'ß') {
        strcpy(buf,"\"s");
        l = z = 2;
      }*/
    }
  }
  //pos = cursor increment

  //return false if nothing has to be inserted
  if (buf.isEmpty()) return false;

  //auto deletion of the marked text occurs not very often and can therefore
  //  be recorded separately
  if (c.flags & cfDelOnInput) delMarkedText(c);

  //do nothing if spaces will be removed
  if (onlySpaces && c.flags & cfRemoveSpaces && c.cursor.x >= textLine->length()) {
    //no trailing space, but move the cursor

    if (c.flags & cfGroupUndo) {
      //modify last undo step that it includes the cursor motion
      KWActionGroup *g = undoList.getLast();
      if (undoCount < 1024 && g != 0L && g->undoType == KWActionGroup::ugInsChar
        && g->end.x == c.cursor.x && g->end.y == c.cursor.y) {

        g->end.x += pos;
      }
    }

    c.cursor.x += pos;
    c.view->updateCursor(c.cursor);
    return true;
  }

  recordStart(c, KWActionGroup::ugInsChar);
  recordReplace(c.cursor, (c.flags & cfOvr) ? buf.length() : 0, buf);
  c.cursor.x += pos;

  if (c.flags & cfWordWrap && c.wrapAt > 0) {
    int line;
    const QChar *s;
//    int pos;
    PointStruc actionCursor;

    line = c.cursor.y;
    do {
      textLine = contents.at(line);
      s = textLine->getText();
      l = textLine->length();
      for (z = c.wrapAt; z < l; z++) if (!s[z].isSpace()) break; //search for text to wrap
      if (z >= l) break; // nothing more to wrap
      pos = c.wrapAt;
      for (; z >= 0; z--) { //find wrap position
        if (s[z].isSpace()) {
          pos = z + 1;
          break;
        }
      }
      //pos = wrap position

      if (line == c.cursor.y && pos <= c.cursor.x) {
        //wrap cursor
        c.cursor.y++;
        c.cursor.x -= pos;
      }

      if (textLine == contents.getLast() || contents.next()->length() == 0) {
        //at end of doc: create new line
        actionCursor.x = pos;
        actionCursor.y = line;
        recordAction(KWAction::newLine,actionCursor);
      } else {
        //wrap
        actionCursor.y = line + 1;
        if (!s[l - 1].isSpace()) { //add space in next line if necessary
          actionCursor.x = 0;
//          recordReplace(actionCursor, 0, " ", 1);
          recordInsert(actionCursor, " ");
        }
        actionCursor.x = textLine->length() - pos;
        recordAction(KWAction::wordWrap, actionCursor);
      }
      line++;
    } while (true);
  }
  recordEnd(c);
  return true;
}

void KWriteDoc::newLine(VConfig &c) {

  //auto deletion of marked text is done by the view to have a more
  // "low level" KWriteDoc::newLine method
  recordStart(c, KWActionGroup::ugInsLine);

  if (!(c.flags & cfAutoIndent)) {
    recordAction(KWAction::newLine,c.cursor);
    c.cursor.y++;
    c.cursor.x = 0;
  } else {
    TextLine *textLine;
    int pos;

    textLine = contents.at(c.cursor.y);
    pos = textLine->firstChar();
    if (pos > c.cursor.x) c.cursor.x = pos;

    do {
      pos = textLine->firstChar();
      if (pos >= 0) break;
      textLine = contents.prev();
    } while (textLine);
    recordAction(KWAction::newLine,c.cursor);
    c.cursor.y++;
    c.cursor.x = 0;
    if (pos > 0) {
      recordInsert(c.cursor, QString(textLine->getText(), pos));
      c.cursor.x = pos;
    }
  }

  recordEnd(c);
}

void KWriteDoc::killLine(VConfig &c) {

  recordStart(c, KWActionGroup::ugDelLine);
  c.cursor.x = 0;
  recordDelete(c.cursor, 0xffffff);
  if (c.cursor.y < (int) contents.count() -1) {
    recordAction(KWAction::killLine, c.cursor);
  }
  recordEnd(c);
}

void KWriteDoc::backspace(VConfig &c) {

  if (c.cursor.x <= 0 && c.cursor.y <= 0) return;

  if (c.cursor.x > 0) {
    recordStart(c, KWActionGroup::ugDelChar);
    if (!(c.flags & cfBackspaceIndents)) {
      c.cursor.x--;
      recordDelete(c.cursor, 1);
    } else {
      TextLine *textLine;
      int pos, l;

      textLine = contents.at(c.cursor.y);
      pos = textLine->firstChar();
      if (pos >= 0 && pos < c.cursor.x) pos = 0;
      while ((textLine = contents.prev()) && pos != 0) {
        pos = textLine->firstChar();
        if (pos >= 0 && pos < c.cursor.x) {
          l = c.cursor.x - pos;
          goto found;
        }
      }
      l = 1;//del one char
      found:
      c.cursor.x -= l;
      recordDelete(c.cursor, l);
    }
  } else {
    recordStart(c, KWActionGroup::ugDelLine);
    c.cursor.y--;
    c.cursor.x = contents.at(c.cursor.y)->length();
    recordAction(KWAction::delLine,c.cursor);
  }
  recordEnd(c);
}


void KWriteDoc::del(VConfig &c) {

  if (c.cursor.x < contents.at(c.cursor.y)->length()) {
    recordStart(c, KWActionGroup::ugDelChar);
    recordDelete(c.cursor, 1);
    recordEnd(c);
  } else {
    if (c.cursor.y < (int) contents.count() -1) {
      recordStart(c, KWActionGroup::ugDelLine);
      recordAction(KWAction::delLine,c.cursor);
      recordEnd(c);
    }
  }
}

void KWriteDoc::clipboardChanged() { //slot
#if defined(_WS_X11_)
  disconnect(QApplication::clipboard(),SIGNAL(dataChanged()),
    this,SLOT(clipboardChanged()));
  deselectAll();
  updateViews();
#endif
}

void KWriteDoc::hlChanged() { //slot
  makeAttribs();
  updateViews();
}

void KWriteDoc::setHighlight(int n) {
  Highlight *h;

//  hlNumber = n;

  h = hlManager->getHl(n);
  if (h == highlight) {
    updateLines();
  } else {
    if (highlight) highlight->release();
    h->use();
    highlight = h;
    makeAttribs();
  }
}

void KWriteDoc::makeAttribs() {

  hlManager->makeAttribs(highlight,attribs,nAttribs);
  updateFontData();
  updateLines();
}
/*
void KWriteDoc::makeAttribs() {
  ItemDataList list;
  ItemData *itemData;
  DefItemStyle *defItemStyle;
  int z;
  QFont font;

  list.setAutoDelete(true);
  highlight->getItemDataList(list);
  for (z = 0; z < (int) list.count(); z++) {
    itemData = list.at(z);
    if (itemData->defStyle) {
      defItemStyle = defItemStyleList->at(itemData->defStyleNum);
      attribs[z].col = defItemStyle->col;
      attribs[z].selCol = defItemStyle->selCol;
      font.setBold(defItemStyle->bold);
      font.setItalic(defItemStyle->italic);
    } else {
      attribs[z].col = itemData->col;
      attribs[z].selCol = itemData->selCol;
      font.setBold(itemData->bold);
      font.setItalic(itemData->italic);
    }
    if (itemData->defFont) {
      font.setFamily(defFont->family);
      font.setPointSize(defFont->size);
      KCharset(defFont->charset).setQFont(font);
    } else {
      font.setFamily(itemData->family);
      font.setPointSize(itemData->size);
      KCharset(itemData->charset).setQFont(font);
    }
    attribs[z].setFont(font);
  }
  for (; z < nAttribs; z++) {
    attribs[z].col = black;
    attribs[z].selCol = black;
    attribs[z].setFont(font);
  }

  updateFontData();
  updateLines();
}
*/
void KWriteDoc::updateFontData() {
  int maxAscent, maxDescent;
  int minTabWidth, maxTabWidth;
  int i, z;
  KWriteView *view;

  maxAscent = 0;
  maxDescent = 0;
  minTabWidth = 0xffffff;
  maxTabWidth = 0;

  for (z = 0; z < nAttribs; z++) {
    i = attribs[z].fm.ascent();
    if (i > maxAscent) maxAscent = i;
    i = attribs[z].fm.descent();
    if (i > maxDescent) maxDescent = i;
    i = attribs[z].fm.width('x');
    if (i < minTabWidth) minTabWidth = i;
    if (i > maxTabWidth) maxTabWidth = i;
  }

  fontHeight = maxAscent + maxDescent + 1;
  fontAscent = maxAscent;
  tabWidth = tabChars*(maxTabWidth + minTabWidth)/2;

  for (view = views.first(); view != 0L; view = views.next()) {
    resizeBuffer(view,view->width(),fontHeight);
    view->tagAll();
    view->updateCursor();
  }
}

void KWriteDoc::setTabWidth(int chars) {
  TextLine *textLine;
  int len;

  if (tabChars == chars) return;
  if (chars < 1) chars = 1;
  if (chars > 16) chars = 16;
  tabChars = chars;
  updateFontData();

  maxLength = -1;
  for (textLine = contents.first(); textLine != 0L; textLine = contents.next()) {
    len = textWidth(textLine,textLine->length());
    if (len > maxLength) {
      maxLength = len;
      longestLine = textLine;
    }
  }
//  tagAll();
}


void KWriteDoc::updateLines(int startLine, int endLine, int flags) {
  TextLine *textLine;
  int line, lastLine;
  int ctxNum, endCtx;

  lastLine = (int) contents.count() -1;
  if (endLine >= lastLine) endLine = lastLine;

  line = startLine;
  ctxNum = 0;
  if (line > 0) ctxNum = contents.at(line - 1)->getContext();
  do {
    textLine = contents.at(line);
    if (line <= endLine) {
      if (flags & cfRemoveSpaces) textLine->removeSpaces();
      updateMaxLength(textLine);
    }
    endCtx = textLine->getContext();
    ctxNum = highlight->doHighlight(ctxNum,textLine);
    textLine->setContext(ctxNum);
    line++;
  } while (line <= lastLine && (line <= endLine || endCtx != ctxNum));
  tagLines(startLine, line - 1);
}


void KWriteDoc::updateMaxLength(TextLine *textLine) {
  int len;

  len = textWidth(textLine,textLine->length());

  if (len > maxLength) {
    longestLine = textLine;
    maxLength = len;
    newDocGeometry = true;
  } else {
    if (!longestLine || (textLine == longestLine && len <= maxLength*3/4)) {
      maxLength = -1;
      for (textLine = contents.first(); textLine != 0L; textLine = contents.next()) {
        len = textWidth(textLine,textLine->length());
        if (len > maxLength) {
          maxLength = len;
          longestLine = textLine;
        }
      }
      newDocGeometry = true;
    }
  }
}


void KWriteDoc::updateViews(KWriteView *exclude) {
  KWriteView *view;
  int flags;
  bool markState = hasMarkedText();

  flags = (newDocGeometry) ? ufDocGeometry : 0;
  for (view = views.first(); view != 0L; view = views.next()) {
    if (view != exclude) view->updateView(flags);

    // notify every view about the changed mark state....
    if (oldMarkState != markState) emit view->kWrite->newMarkStatus();
  }
  oldMarkState = markState;
  newDocGeometry = false;
}


int KWriteDoc::textWidth(TextLine *textLine, int cursorX) {
  int x;
  int z;
  char ch;
  Attribute *a;

  x = 0;
  for (z = 0; z < cursorX; z++) {
    ch = textLine->getChar(z);
    a = &attribs[textLine->getAttr(z)];
    x += (ch == '\t') ? tabWidth - (x % tabWidth) : a->width(ch);//a->fm.width(ch);
  }
  return x;
}

int KWriteDoc::textWidth(PointStruc &cursor) {
  if (cursor.x < 0) cursor.x = 0;
  if (cursor.y < 0) cursor.y = 0;
  if (cursor.y >= (int) contents.count()) cursor.y = (int) contents.count() -1;
  return textWidth(contents.at(cursor.y),cursor.x);
}

int KWriteDoc::textWidth(bool wrapCursor, PointStruc &cursor, int xPos) {
  TextLine *textLine;
  int len;
  int x, oldX;
  int z;
  char ch;
  Attribute *a;

  if (cursor.y < 0) cursor.y = 0;
  if (cursor.y >= (int) contents.count()) cursor.y = (int) contents.count() -1;
  textLine = contents.at(cursor.y);
  len = textLine->length();

  x = oldX = z = 0;
  while (x < xPos && (!wrapCursor || z < len)) {
    oldX = x;
    ch = textLine->getChar(z);
    a = &attribs[textLine->getAttr(z)];
    x += (ch == '\t') ? tabWidth - (x % tabWidth) : a->width(ch);//a->fm.width(ch);
    z++;
  }
  if (xPos - oldX < x - xPos && z > 0) {
    z--;
    x = oldX;
  }
  cursor.x = z;
  return x;
}


int KWriteDoc::textPos(TextLine *textLine, int xPos) {
/*  int newXPos;

  return textPos(textLine, xPos, newXPos);
}

int KWriteDoc::textPos(TextLine *textLine, int xPos, int &newXPos) {
*/
//  int len;
  int x, oldX;
  int z;
  char ch;
  Attribute *a;

//  len = textLine->length();

  x = oldX = z = 0;
  while (x < xPos) { // && z < len) {
    oldX = x;
    ch = textLine->getChar(z);
    a = &attribs[textLine->getAttr(z)];
    x += (ch == '\t') ? tabWidth - (x % tabWidth) : a->width(ch);//a->fm.width(ch);
    z++;
  }
  if (xPos - oldX < x - xPos && z > 0) {
    z--;
   // newXPos = oldX;
  }// else newXPos = x;
  return z;
}

int KWriteDoc::textWidth() {
  return maxLength + 8;
}

int KWriteDoc::textHeight() {
  return contents.count()*fontHeight;
}

void KWriteDoc::toggleRect(int start, int end, int x1, int x2) {
  int z, line;
  bool t;

  if (x1 > x2) {
    z = x1;
    x1 = x2;
    x2 = z;
  }
  if (start > end) {
    z = start;
    start = end;
    end = z;
  }

  t = false;
  for (line = start; line < end; line++) {
    int x, oldX, s, e, newX1, newX2;
    TextLine *textLine;
    char ch;
    Attribute *a;

    textLine = contents.at(line);

    //--- speed optimization
    //s = textPos(textLine, x1, newX1);
    x = oldX = z = 0;
    while (x < x1) { // && z < len) {
      oldX = x;
      ch = textLine->getChar(z);
      a = &attribs[textLine->getAttr(z)];
      x += (ch == '\t') ? tabWidth - (x % tabWidth) : a->width(ch);//a->fm.width(ch);
      z++;
    }
    s = z;
    if (x1 - oldX < x - x1 && z > 0) {
      s--;
      newX1 = oldX;
    } else newX1 = x;
    //e = textPos(textLine, x2, newX2);
    while (x < x2) { // && z < len) {
      oldX = x;
      ch = textLine->getChar(z);
      a = &attribs[textLine->getAttr(z)];
      x += (ch == '\t') ? tabWidth - (x % tabWidth) : a->width(ch);//a->fm.width(ch);
      z++;
    }
    e = z;
    if (x2 - oldX < x - x2 && z > 0) {
      e--;
      newX2 = oldX;
    } else newX2 = x;
    //---

    if (e > s) {
      textLine->toggleSelect(s, e);
      tagLineRange(line, newX1, newX2);
      t = true;
    }
  }
  if (t) {
    end--;
//    tagLines(start, end);

    if (start < selectStart) selectStart = start;
    if (end > selectEnd) selectEnd = end;
  }
}

void KWriteDoc::selectTo(VConfig &c, PointStruc &cursor, int cXPos) {
  //c.cursor = old cursor position
  //cursor = new cursor position

  if (c.cursor.x != select.x || c.cursor.y != select.y) {
    //new selection

    if (!(c.flags & cfKeepSelection)) deselectAll();
//      else recordReset();

    anchor = c.cursor;
    aXPos = c.cXPos;
  }

  if (!(c.flags & cfVerticalSelect)) {
    //horizontal selections
    TextLine *textLine;
    int x, y, sXPos;
    int ex, ey, eXPos;
    bool sel;

    if (cursor.y > c.cursor.y || (cursor.y == c.cursor.y && cursor.x > c.cursor.x)) {
      x = c.cursor.x;
      y = c.cursor.y;
      sXPos = c.cXPos;
      ex = cursor.x;
      ey = cursor.y;
      eXPos = cXPos;
      sel = true;
    } else {
      x = cursor.x;
      y = cursor.y;
      sXPos = cXPos;
      ex = c.cursor.x;
      ey = c.cursor.y;
      eXPos = c.cXPos;
      sel = false;
    }

//    tagLines(y, ye);
    if (y < ey) {
      //tagLineRange(y, sXPos, 0xffffff);
      tagLines(y, ey -1);
      tagLineRange(ey, 0, eXPos);
    } else tagLineRange(y, sXPos, eXPos);

    if (y < selectStart) selectStart = y;
    if (ey > selectEnd) selectEnd = ey;

    textLine = contents.at(y);

    if (c.flags & cfXorSelect) {
      //xor selection with old selection
      while (y < ey) {
        textLine->toggleSelectEol(x);
        x = 0;
        y++;
        textLine = contents.at(y);
      }
      textLine->toggleSelect(x, ex);
    } else {
      //set selection over old selection

      if (anchor.y > y || (anchor.y == y && anchor.x > x)) {
        if (anchor.y < ey || (anchor.y == ey && anchor.x < ex)) {
          sel = !sel;
          while (y < anchor.y) {
            textLine->selectEol(sel, x);
            x = 0;
            y++;
            textLine = contents.at(y);
          }
          textLine->select(sel, x, anchor.x);
          x = anchor.x;
        }
        sel = !sel;
      }
      while (y < ey) {
        textLine->selectEol(sel, x);
        x = 0;
        y++;
        textLine = contents.at(y);
      }
      textLine->select(sel, x, ex);
    }
  } else {
    //vertical (block) selections
//    int ax, sx, ex;

//    ax = textWidth(anchor);
//    sx = textWidth(start);
//    ex = textWidth(end);

    toggleRect(c.cursor.y + 1, cursor.y + 1, aXPos, c.cXPos);
    toggleRect(anchor.y, cursor.y + 1, c.cXPos, cXPos);
  }
  select = cursor;
  optimizeSelection();
}

void KWriteDoc::clear() {
  PointStruc cursor;
  KWriteView *view;

  setPseudoModal(0L);
  cursor.x = cursor.y = 0;
  for (view = views.first(); view != 0L; view = views.next()) {
    view->updateCursor(cursor);
    view->tagAll();
  }

  eolMode = eolUnix;

  contents.clear();

  contents.append(longestLine = new TextLine());
  maxLength = 0;

  select.x = -1;

  selectStart = 0xffffff;
  selectEnd = 0;
  oldMarkState = false;

  foundLine = -1;

  setModified(false);

  undoList.clear();
  currentUndo = 0;
//  recordReset();
  newUndo();
}


void KWriteDoc::copy(int flags) {

  if (selectEnd < selectStart) return;

  QString s = markedText(flags);
  if (!s.isEmpty()) {
#if defined(_WS_X11_)
    disconnect(QApplication::clipboard(),SIGNAL(dataChanged()),this,0);
#endif
    QApplication::clipboard()->setText(s);
#if defined(_WS_X11_)
    connect(QApplication::clipboard(),SIGNAL(dataChanged()),
      this,SLOT(clipboardChanged()));
#endif
  }
}

void KWriteDoc::paste(VConfig &c) {
  QString s = QApplication::clipboard()->text();
  if (!s.isEmpty()) {
//    unmarkFound();
    insert(c, s);
  }
}


void KWriteDoc::cut(VConfig &c) {

  if (selectEnd < selectStart) return;

//  unmarkFound();
  copy(c.flags);
  delMarkedText(c);
}

void KWriteDoc::selectAll() {
  int z;
  TextLine *textLine;

  select.x = -1;

  unmarkFound();
//  if (selectStart != 0 || selectEnd != lastLine()) recordReset();

  selectStart = 0;
  selectEnd = lastLine();

  tagLines(selectStart,selectEnd);

  for (z = selectStart; z < selectEnd; z++) {
    textLine = contents.at(z);
    textLine->selectEol(true,0);
  }
  textLine = contents.at(z);
  textLine->select(true,0,textLine->length());
}

void KWriteDoc::deselectAll() {
  int z;
  TextLine *textLine;

  select.x = -1;
  if (selectEnd < selectStart) return;

  unmarkFound();
//  recordReset();

  tagLines(selectStart,selectEnd);

  for (z = selectStart; z <= selectEnd; z++) {
    textLine = contents.at(z);
    textLine->selectEol(false,0);
  }
  selectStart = 0xffffff;
  selectEnd = 0;
}

void KWriteDoc::invertSelection() {
  int z;
  TextLine *textLine;

  select.x = -1;

  unmarkFound();
//  if (selectStart != 0 || selectEnd != lastLine()) recordReset();

  selectStart = 0;
  selectEnd = lastLine();

  tagLines(selectStart,selectEnd);

  for (z = selectStart; z < selectEnd; z++) {
    textLine = contents.at(z);
    textLine->toggleSelectEol(0);
  }
  textLine = contents.at(z);
  textLine->toggleSelect(0,textLine->length());
  optimizeSelection();
}

void KWriteDoc::selectWord(PointStruc &cursor, int flags) {
  TextLine *textLine;
  int start, end, len;

  textLine = contents.at(cursor.y);
  len = textLine->length();
  start = end = cursor.x;
  while (start > 0 && highlight->isInWord(textLine->getChar(start - 1))) start--;
  while (end < len && highlight->isInWord(textLine->getChar(end))) end++;
  if (end <= start) return;
  if (!(flags & cfKeepSelection)) deselectAll();
//    else recordReset();

  textLine->select(true, start, end);

  anchor.x = start;
  select.x = end;
  anchor.y = select.y = cursor.y;
  tagLines(cursor.y, cursor.y);
  if (cursor.y < selectStart) selectStart = cursor.y;
  if (cursor.y > selectEnd) selectEnd = cursor.y;
}

QString KWriteDoc::text() {
  TextLine *textLine;
  QString s;

  textLine = contents.first();
  do {
    s.insert(s.length(), textLine->getText(), textLine->length());
    textLine = contents.next();
    if (!textLine) break;
    s.append('\n');
  } while (true);
  return s;
}

QString KWriteDoc::getWord(PointStruc &cursor) {
  TextLine *textLine;
  int start, end, len;

  textLine = contents.at(cursor.y);
  len = textLine->length();
  start = end = cursor.x;
  while (start > 0 && highlight->isInWord(textLine->getChar(start - 1))) start--;
  while (end < len && highlight->isInWord(textLine->getChar(end))) end++;
  len = end - start;
  return QString(&textLine->getText()[start], len);
}

void KWriteDoc::setText(const QString &s) {
  TextLine *textLine;
  int pos;
  QChar ch;

  clear();

  textLine = contents.getFirst();
  for (pos = 0; pos <= (int) s.length(); pos++) {
    ch = s[pos];
    if (ch.isPrint() || ch == '\t') {
      textLine->append(&ch, 1);
    } else if (ch == '\n') {
      textLine = new TextLine();
      contents.append(textLine);
    }
  }
  updateLines();
}


QString KWriteDoc::markedText(int flags) {
  TextLine *textLine;
  int len, z, start, end, i;

  len = 1;
  if (!(flags & cfVerticalSelect)) {
    for (z = selectStart; z <= selectEnd; z++) {
      textLine = contents.at(z);
      len += textLine->numSelected();
      if (textLine->isSelected()) len++;
    }
    QString s;
    len = 0;
    for (z = selectStart; z <= selectEnd; z++) {
      textLine = contents.at(z);
      end = 0;
      do {
        start = textLine->findUnselected(end);
        end = textLine->findSelected(start);
        for (i = start; i < end; i++) {
          s[len] = textLine->getChar(i);
          len++;
        }
      } while (start < end);
      if (textLine->isSelected()) {
        s[len] = '\n';
        len++;
      }
    }
    s[len] = '\0';
    return s;
  } else {
    for (z = selectStart; z <= selectEnd; z++) {
      textLine = contents.at(z);
      len += textLine->numSelected() + 1;
    }
    QString s;
    len = 0;
    for (z = selectStart; z <= selectEnd; z++) {
      textLine = contents.at(z);
      end = 0;
      do {
        start = textLine->findUnselected(end);
        end = textLine->findSelected(start);
        for (i = start; i < end; i++) {
          s[len] = textLine->getChar(i);
          len++;
        }
      } while (start < end);
      s[len] = '\n';
      len++;
    }
    s[len] = '\0';
    return s;
  }
}

void KWriteDoc::delMarkedText(VConfig &c/*, bool undo*/) {
  TextLine *textLine;
  int end = 0;

  if (selectEnd < selectStart) return;

  // the caller may have already started an undo record for the current action
//  if (undo)

  //auto deletion of the marked text occurs not very often and can therefore
  //  be recorded separately
  recordStart(c, KWActionGroup::ugDelBlock);

  for (c.cursor.y = selectEnd; c.cursor.y >= selectStart; c.cursor.y--) {
    textLine = contents.at(c.cursor.y);

    c.cursor.x = textLine->length();
    do {
      end = textLine->findRevUnselected(c.cursor.x);
      if (end == 0) break;
      c.cursor.x = textLine->findRevSelected(end);
      recordDelete(c.cursor, end - c.cursor.x);
    } while (true);
    end = c.cursor.x;
    c.cursor.x = textLine->length();
    if (textLine->isSelected()) recordAction(KWAction::delLine,c.cursor);
  }
  c.cursor.y++;
  /*if (end < c.cursor.x)*/ c.cursor.x = end;

  selectEnd = -1;
  select.x = -1;

  /*if (undo)*/ recordEnd(c);
}

QColor &KWriteDoc::cursorCol(int x, int y) {
  TextLine *textLine;
  int attr;
  Attribute *a;

  textLine = contents.at(y);
  attr = textLine->getRawAttr(x);
  a = &attribs[attr & taAttrMask];
  if (attr & taSelectMask) return a->selCol; else return a->col;
}


void KWriteDoc::paintTextLine(QPainter &paint, int line, int xStart, int xEnd) {
  int y;
  TextLine *textLine;
  int len;
  const QChar *s;
  int z, x;
  char ch;
  Attribute *a = 0L;
  int attr, nextAttr;
  int xs;
  int xc, zc;

//  struct timeval tv1, tv2, tv3; //for profiling
//  struct timezone tz;

  y = 0;//line*fontHeight - yPos;
  if (line >= (int) contents.count()) {
    paint.fillRect(0, y, xEnd - xStart,fontHeight, colors[4]);
    return;
  }

  textLine = contents.at(line);
  len = textLine->length();
  s = textLine->getText();

  // skip to first visible character
  x = 0;
  z = 0;
  do {
    xc = x;
    zc = z;
    if (z == len) break;
    ch = s[z];//textLine->getChar(z);
    if (ch == '\t') {
      x += tabWidth - (x % tabWidth);
    } else {
      a = &attribs[textLine->getAttr(z)];
      x += a->width(ch);//a->fm.width(ch);
    }
    z++;
  } while (x <= xStart);

//gettimeofday(&tv1, &tz);

  // draw background
  xs = xStart;
  attr = textLine->getRawAttr(zc);
  while (x < xEnd) {
    nextAttr = textLine->getRawAttr(z);
    if ((nextAttr ^ attr) & (taSelectMask | 256)) {
      paint.fillRect(xs - xStart, y, x - xs, fontHeight, colors[attr >> taShift]);
      xs = x;
      attr = nextAttr;
    }
    if (z == len) break;
    ch = s[z];//textLine->getChar(z);
    if (ch == '\t') {
      x += tabWidth - (x % tabWidth);
    } else {
      a = &attribs[attr & taAttrMask];
      x += a->width(ch);//a->fm.width(ch);
    }
    z++;
  }
  paint.fillRect(xs - xStart, y, xEnd - xs, fontHeight, colors[attr >> taShift]);
  len = z; //reduce length to visible length

//gettimeofday(&tv2, &tz);

  // draw text
  x = xc;
  z = zc;
  y += fontAscent -1;
  attr = -1;
  while (z < len) {
    ch = s[z];//textLine->getChar(z);
    if (ch == '\t') {
      if (z > zc) {
        //this should cause no copy at all
        QConstString str((QChar *) &s[zc], z - zc /*+1*/);
        QString s = str.string();
        paint.drawText(x - xStart, y, s);
        x += a->width(s);//a->fm.width(str);//&s[zc], z - zc);
      }
      zc = z +1;
      x += tabWidth - (x % tabWidth);
    } else {
      nextAttr = textLine->getRawAttr(z);
      if (nextAttr != attr) {
        if (z > zc) {
          QConstString str((QChar *) &s[zc], z - zc /*+1*/);
          QString s = str.string();
          paint.drawText(x - xStart, y, s);
          x += a->width(s);//a->fm.width(str);//&s[zc], z - zc);
          zc = z;
        }
        attr = nextAttr;
        a = &attribs[attr & taAttrMask];

        if (attr & taSelectMask) paint.setPen(a->selCol);
          else paint.setPen(a->col);
        paint.setFont(a->font);
      }
    }
    z++;
  }
  if (z > zc) {
    QConstString str((QChar *) &s[zc], z - zc /*+1*/);
    paint.drawText(x - xStart, y, str.string());
  }
//gettimeofday(&tv3, &tz);
//printf(" %d %d\n", tv2.tv_usec - tv1.tv_usec, tv3.tv_usec - tv2.tv_usec);
}


void KWriteDoc::printTextLine(QPainter &paint, int line, int xEnd, int y) {
  TextLine *textLine;
  int z, x;
  Attribute *a = 0L;
  int attr, nextAttr;
  char ch;
  char buf[256];
  int bufp;

  if (line >= (int) contents.count()) return;
  textLine = contents.at(line);

  z = 0;
  x = 0;
  y += fontAscent -1;
  attr = -1;
  bufp = 0;
  while (x < xEnd && z < textLine->length()) {
    ch = textLine->getChar(z);
    if (ch == '\t') {
      if (bufp > 0) {
        paint.drawText(x, y, buf, bufp);
        x += paint.fontMetrics().width(buf, bufp);
        bufp = 0;
      }
      x += tabWidth - (x % tabWidth);
    } else {
      nextAttr = textLine->getAttr(z);
      if (nextAttr != attr || bufp >= 256) {
        if (bufp > 0) {
          paint.drawText(x, y, buf, bufp);
          x += paint.fontMetrics().width(buf,bufp);
          bufp = 0;
        }
        attr = nextAttr;
        a = &attribs[attr];
        paint.setFont(a->font);
      }
      buf[bufp] = ch;
      bufp++;
    }
    z++;
  }
  if (bufp > 0) paint.drawText(x, y, buf, bufp);
}

void KWriteDoc::setReadOnly(bool m) {
  KWriteView *view;

  if (m != readOnly) {
    readOnly = m;
//    if (readOnly) recordReset();
    for (view = views.first(); view != 0L; view = views.next()) {
      emit view->kWrite->newStatus();
    }
  }
}

bool KWriteDoc::isReadOnly() {
  return readOnly;
}

void KWriteDoc::setModified(bool m) {
  KWriteView *view;

  if (m != modified) {
    modified = m;
    for (view = views.first(); view != 0L; view = views.next()) {
      emit view->kWrite->newStatus();
    }
  }
}

bool KWriteDoc::isModified() {
  return modified;
}

bool KWriteDoc::isLastView(int numViews) {
  return ((int) views.count() == numViews);
}


bool KWriteDoc::hasFileName() {
  return fName.findRev('/') +1 < (int) fName.length();
}

const QString KWriteDoc::fileName() {
  return fName;
}

void KWriteDoc::setFileName( const QString& s ) {
  int pos, hl;
  KWriteView *view;

  fName = s;
  for (view = views.first(); view != 0L; view = views.next()) {
    emit view->kWrite->fileChanged();
  }

  //highlight detection
  pos = fName.findRev('/') +1;
  if (pos >= (int) fName.length()) return; //no filename
  hl = hlManager->wildcardFind((pos) ? s.right(pos) : QString("")); // this is a WORKAROUND
                                                                    //  QT-2.00 QRegExp doesn´t permit a null-string

  if (hl == -1) {
    // fill the detection buffer with the contents of the text
    const int HOWMANY = 1024;
    char buf[HOWMANY];
    int bufpos = 0, len;
    TextLine *textLine;

    for (textLine = contents.first(); textLine != 0L; textLine = contents.next()) {
      len = textLine->length();
      if (bufpos + len > HOWMANY) len = HOWMANY - bufpos;
      memcpy(&buf[bufpos], textLine->getText(), len);
      bufpos += len;
      if (bufpos >= HOWMANY) break;
    }
    hl = hlManager->mimeFind(buf, bufpos, s.right(pos));
  }
  setHighlight(hl);
  updateViews();
}

void KWriteDoc::clearFileName() {
  KWriteView *view;

  fName.truncate(fName.findRev('/') +1);
  for (view = views.first(); view != 0L; view = views.next()) {
    emit view->kWrite->fileChanged();
  }
}



bool KWriteDoc::doSearch(SConfig &sc, const QString &searchFor) {
  int line, col;
  int searchEnd;
//  int slen, blen, tlen;
//  char *s, *b, *t;
  QString str;
  int slen, smlen, bufLen, tlen;
  const QChar *s;
  QChar *t;
  TextLine *textLine;
  int z, pos, newPos;

  if (searchFor.isEmpty()) return false;
  str = (sc.flags & sfCaseSensitive) ? searchFor : searchFor.lower();

  s = str.unicode();
  slen = str.length();
  smlen = slen*sizeof(QChar);
  bufLen = 0;
  t = 0L;

  line = sc.cursor.y;
  col = sc.cursor.x;
  if (!(sc.flags & sfBackward)) {
    //forward search
    if (sc.flags & sfSelected) {
      if (line < selectStart) {
        line = selectStart;
        col = 0;
      }
      searchEnd = selectEnd;
    } else searchEnd = lastLine();

    while (line <= searchEnd) {
      textLine = contents.at(line);
      tlen = textLine->length();
      if (tlen > bufLen) {
        delete t;
        bufLen = (tlen + 255) & (~255);
        t = new QChar[bufLen];
      }
      memcpy(t, textLine->getText(), tlen*sizeof(QChar));
      if (sc.flags & sfSelected) {
        pos = 0;
        do {
          pos = textLine->findSelected(pos);
          newPos = textLine->findUnselected(pos);
          memset(&t[pos], 0, (newPos - pos)*sizeof(QChar));
          pos = newPos;
        } while (pos < tlen);
      }
      if (!(sc.flags & sfCaseSensitive)) for (z = 0; z < tlen; z++) t[z] = t[z].lower();

      tlen -= slen;
      if (sc.flags & sfWholeWords && tlen > 0) {
        //whole word search
        if (col == 0) {
          if (!highlight->isInWord(t[slen])) {
            if (memcmp(t, s, smlen) == 0) goto found;
          }
          col++;
        }
        while (col < tlen) {
          if (!highlight->isInWord(t[col -1]) && !highlight->isInWord(t[col + slen])) {
            if (memcmp(&t[col], s, smlen) == 0) goto found;
          }
          col++;
        }
        if (!highlight->isInWord(t[col -1]) && memcmp(&t[col], s, smlen) == 0)
          goto found;
      } else {
        //normal search
        while (col <= tlen) {
          if (memcmp(&t[col], s, smlen) == 0) goto found;
          col++;
        }
      }
      col = 0;
      line++;
    }
  } else {
    // backward search
    if (sc.flags & sfSelected) {
      if (line > selectEnd) {
        line = selectEnd;
        col = -1;
      }
      searchEnd = selectStart;
    } else searchEnd = 0;;

    while (line >= searchEnd) {
      textLine = contents.at(line);
      tlen = textLine->length();
      if (tlen > bufLen) {
        delete t;
        bufLen = (tlen + 255) & (~255);
        t = new QChar[bufLen];
      }
      memcpy(t, textLine->getText(), tlen*sizeof(QChar));
      if (sc.flags & sfSelected) {
        pos = 0;
        do {
          pos = textLine->findSelected(pos);
          newPos = textLine->findUnselected(pos);
          memset(&t[pos], 0, (newPos - pos)*sizeof(QChar));
          pos = newPos;
        } while (pos < tlen);
      }
      if (!(sc.flags & sfCaseSensitive)) for (z = 0; z < tlen; z++) t[z] = t[z].lower();

      if (col < 0 || col > tlen) col = tlen;
      col -= slen;
      if (sc.flags & sfWholeWords && tlen > slen) {
        //whole word search
        if (col + slen == tlen) {
          if (!highlight->isInWord(t[col -1])) {
            if (memcmp(&t[col], s, smlen) == 0) goto found;
          }
          col--;
        }
        while (col > 0) {
          if (!highlight->isInWord(t[col -1]) && !highlight->isInWord(t[col+slen])) {
            if (memcmp(&t[col],s,smlen) == 0) goto found;
          }
          col--;
        }
        if (!highlight->isInWord(t[slen]) && memcmp(t, s, smlen) == 0)
          goto found;
      } else {
        //normal search
        while (col >= 0) {
          if (memcmp(&t[col],s,slen) == 0) goto found;
          col--;
        }
      }
      line--;
    }
  }
  sc.flags |= sfWrapped;
  return false;
found:
  if (sc.flags & sfWrapped) {
    if ((line > sc.startCursor.y || (line == sc.startCursor.y && col >= sc.startCursor.x))
      ^ ((sc.flags & sfBackward) != 0)) return false;
  }
  sc.cursor.x = col;
  sc.cursor.y = line;
  return true;
}
/*
void downcase(char *s, int len) {
  while (len > 0) {
    if (*s >= 'A' && *s <= 'Z') *s += 32;
    s++;
    len--;
  }
}

bool KWriteDoc::doSearch(SConfig &sc, const char *searchFor) {
  int line, col;
  int searchEnd;
  int slen, blen, tlen;
  char *s, *b, *t;
  TextLine *textLine;
  int pos, newPos;

  slen = strlen(searchFor);
  if (slen == 0) return 0;
  s = new char[slen];
  memcpy(s,searchFor,slen);
  if (!(sc.flags & sfCaseSensitive)) downcase(s,slen);

  blen = -2;
  b = 0;
  t = 0;

  line = sc.cursor.y;
  col = sc.cursor.x;
  if (!(sc.flags & sfBackward)) {
    //forward search

    if (sc.flags & sfSelected) {
      if (line < selectStart) {
        line = selectStart;
        col = 0;
      }
      searchEnd = selectEnd;
    } else searchEnd = lastLine();

    while (line <= searchEnd) {
      textLine = contents.at(line);
      tlen = textLine->length();
      if (tlen > blen) {
        delete b;
        blen = (tlen + 257) & (~255);
        b = new char[blen];
        blen -= 2;
        b[0] = 0;
        t = &b[1];
      }
      memcpy(t,textLine->getText(),tlen);
      t[tlen] = 0;
      if (sc.flags & sfSelected) {
        pos = 0;
        do {
          pos = textLine->findSelected(pos);
          newPos = textLine->findUnselected(pos);
          memset(&t[pos],0,newPos - pos);
          pos = newPos;
        } while (pos < tlen);
      }
      if (!(sc.flags & sfCaseSensitive)) downcase(t,tlen);

      tlen -= slen;
      if (sc.flags & sfWholeWords) {
        while (col <= tlen) {
          if (!highlight->isInWord(t[col-1]) && !highlight->isInWord(t[col+slen])) {
            if (memcmp(&t[col],s,slen) == 0) goto found;
          }
          col++;
        }
      } else {
        while (col <= tlen) {
          if (memcmp(&t[col],s,slen) == 0) goto found;
          col++;
        }
      }
      col = 0;
      line++;
    }
  } else {
    // backward search

    if (sc.flags & sfSelected) {
      if (line > selectEnd) {
        line = selectEnd;
        col = -1;
      }
      searchEnd = selectStart;
    } else searchEnd = 0;;

    while (line >= searchEnd) {
      textLine = contents.at(line);
      tlen = textLine->length();
      if (tlen > blen) {
        delete b;
        blen = (tlen + 257) & (~255);
        b = new char[blen];
        blen -= 2;
        b[0] = 0;
        t = &b[1];
      }
      memcpy(t,textLine->getText(),tlen);
      t[tlen] = 0;
      if (sc.flags & sfSelected) {
        pos = 0;
        do {
          pos = textLine->findSelected(pos);
          newPos = textLine->findUnselected(pos);
          memset(&t[pos],0,newPos - pos);
          pos = newPos;
        } while (pos < tlen);
      }
      if (!(sc.flags & sfCaseSensitive)) downcase(t,tlen);

      if (col < 0 || col > tlen) col = tlen;
      col -= slen;
      if (sc.flags & sfWholeWords) {
        while (col >= 0) {
          if (!highlight->isInWord(t[col-1]) && !highlight->isInWord(t[col+slen])) {
            if (memcmp(&t[col],s,slen) == 0) goto found;
          }
          col--;
        }
      } else {
        while (col >= 0) {
          if (memcmp(&t[col],s,slen) == 0) goto found;
          col--;
        }
      }
      line--;
    }
  }
  sc.flags |= sfWrapped;
  return false;
found:
  if (sc.flags & sfWrapped) {
    if ((line > sc.startCursor.y || (line == sc.startCursor.y && col >= sc.startCursor.x))
      ^ ((sc.flags & sfBackward) != 0)) return false;
  }
  sc.cursor.x = col;
  sc.cursor.y = line;
  return true;
} */

void KWriteDoc::unmarkFound() {
  if (pseudoModal) return;
  if (foundLine != -1) {
    contents.at(foundLine)->unmarkFound();
    tagLines(foundLine,foundLine);
    foundLine = -1;
  }
}

void KWriteDoc::markFound(PointStruc &cursor, int len) {
//  unmarkFound();
//  recordReset();
  if (foundLine != -1) {
    contents.at(foundLine)->unmarkFound();
    tagLines(foundLine,foundLine);
  }
  contents.at(cursor.y)->markFound(cursor.x,len);
  foundLine = cursor.y;
  tagLines(foundLine,foundLine);
}


void KWriteDoc::tagLine(int line) {

  if (tagStart > line) tagStart = line;
  if (tagEnd < line) tagEnd = line;
}

void KWriteDoc::insLine(int line) {
  KWriteView *view;

  if (selectStart >= line) selectStart++;
  if (selectEnd >= line) selectEnd++;
  if (tagStart >= line) tagStart++;
  if (tagEnd >= line) tagEnd++;

  newDocGeometry = true;
  for (view = views.first(); view != 0L; view = views.next()) {
    view->insLine(line);
  }
}

void KWriteDoc::delLine(int line) {
  KWriteView *view;

  if (selectStart >= line && selectStart > 0) selectStart--;
  if (selectEnd >= line) selectEnd--;
  if (tagStart >= line && tagStart > 0) tagStart--;
  if (tagEnd >= line) tagEnd--;

  newDocGeometry = true;
  for (view = views.first(); view != 0L; view = views.next()) {
    view->delLine(line);
  }
}

void KWriteDoc::optimizeSelection() {
  TextLine *textLine;

  while (selectStart <= selectEnd) {
    textLine = contents.at(selectStart);
    if (textLine->isSelected() || textLine->numSelected() > 0) break;
    selectStart++;
  }
  while (selectEnd >= selectStart) {
    textLine = contents.at(selectEnd);
    if (textLine->isSelected() || textLine->numSelected() > 0) break;
    selectEnd--;
  }
  if (selectStart > selectEnd) {
    selectStart = 0xffffff;
    selectEnd = 0;
  }
}

void KWriteDoc::doAction(KWAction *a) {

  switch (a->action) {
    case KWAction::replace:
      doReplace(a);
      break;
    case KWAction::wordWrap:
      doWordWrap(a);
      break;
    case KWAction::wordUnWrap:
      doWordUnWrap(a);
      break;
    case KWAction::newLine:
      doNewLine(a);
      break;
    case KWAction::delLine:
      doDelLine(a);
      break;
    case KWAction::insLine:
      doInsLine(a);
      break;
    case KWAction::killLine:
      doKillLine(a);
      break;
/*    case KWAction::doubleLine:
      break;
    case KWAction::removeLine:
      break;*/
  }
}

void KWriteDoc::doReplace(KWAction *a) {
  TextLine *textLine;
  int l;

  //exchange current text with stored text in KWAction *a

  textLine = contents.at(a->cursor.y);
  l = textLine->length() - a->cursor.x;
  if (l > a->len) l = a->len;

  QString oldText(&textLine->getText()[a->cursor.x], (l < 0) ? 0 : l);
  textLine->replace(a->cursor.x, a->len, a->text.unicode(), a->text.length());

  a->len = a->text.length();
  a->text = oldText;

  tagLine(a->cursor.y);
}

void KWriteDoc::doWordWrap(KWAction *a) {
  TextLine *textLine;

  textLine = contents.at(a->cursor.y - 1);
  a->len = textLine->length() - a->cursor.x;
  textLine->wrap(contents.next(),a->len);

  tagLine(a->cursor.y - 1);
  tagLine(a->cursor.y);
  if (selectEnd == a->cursor.y - 1) selectEnd++;

  a->action = KWAction::wordUnWrap;
}

void KWriteDoc::doWordUnWrap(KWAction *a) {
  TextLine *textLine;

  textLine = contents.at(a->cursor.y - 1);
//  textLine->setLength(a->len);
  textLine->unWrap(a->len, contents.next(),a->cursor.x);

  tagLine(a->cursor.y - 1);
  tagLine(a->cursor.y);

  a->action = KWAction::wordWrap;
}

void KWriteDoc::doNewLine(KWAction *a) {
  TextLine *textLine, *newLine;

  textLine = contents.at(a->cursor.y);
  newLine = new TextLine(textLine->getRawAttr(), textLine->getContext());
  textLine->wrap(newLine,a->cursor.x);
  contents.insert(a->cursor.y + 1,newLine);

  insLine(a->cursor.y + 1);
  tagLine(a->cursor.y);
  tagLine(a->cursor.y + 1);
  if (selectEnd == a->cursor.y) selectEnd++;//addSelection(a->cursor.y + 1);

  a->action = KWAction::delLine;
}

void KWriteDoc::doDelLine(KWAction *a) {
  TextLine *textLine, *nextLine;

  textLine = contents.at(a->cursor.y);
  nextLine = contents.next();
//  textLine->setLength(a->cursor.x);
  textLine->unWrap(a->cursor.x, nextLine,nextLine->length());
  textLine->setContext(nextLine->getContext());
  if (longestLine == nextLine) longestLine = 0L;
  contents.remove();

  tagLine(a->cursor.y);
  delLine(a->cursor.y + 1);

  a->action = KWAction::newLine;
}

void KWriteDoc::doInsLine(KWAction *a) {

  contents.insert(a->cursor.y,new TextLine());

  insLine(a->cursor.y);

  a->action = KWAction::killLine;
}

void KWriteDoc::doKillLine(KWAction *a) {
  TextLine *textLine;

  textLine = contents.at(a->cursor.y);
  if (longestLine == textLine) longestLine = 0L;
  contents.remove();

  delLine(a->cursor.y);
  tagLine(a->cursor.y);

  a->action = KWAction::insLine;
}

void KWriteDoc::newUndo() {
  KWriteView *view;
  int state;

  state = 0;
  if (currentUndo > 0) state |= 1;
  if (currentUndo < (int) undoList.count()) state |= 2;
  undoState = state;
  for (view = views.first(); view != 0L; view = views.next()) {
    emit view->kWrite->newUndo();
  }
}

void KWriteDoc::recordStart(VConfig &c, int newUndoType) {
  recordStart(c.view, c.cursor, c.flags, newUndoType);
}

void KWriteDoc::recordStart(KWriteView *, PointStruc &cursor, int flags,
  int newUndoType, bool keepModal, bool mergeUndo) {

  KWActionGroup *g;

//  if (newUndoType == KWActionGroup::ugNone) {
    // only a bug would cause this
//why should someone do this? we can't prevent all programming errors :) (jochen whilhelmy)
//    debug("KWriteDoc::recordStart() called with no undo group type!");
//    return;
//  }

  if (!keepModal) setPseudoModal(0L);

  //i optimized the group undo stuff a bit (jochen wilhelmy)
  //  recordReset() is not needed any more
  g = undoList.getLast();
  if (g != 0L && ((undoCount < 1024 && flags & cfGroupUndo
    && g->end.x == cursor.x && g->end.y == cursor.y) || mergeUndo)) {

    //undo grouping : same actions are put into one undo step
    //precondition : new action starts where old stops or mergeUndo flag
    if (g->undoType == newUndoType
      || (g->undoType == KWActionGroup::ugInsChar
        && newUndoType == KWActionGroup::ugInsLine)
      || (g->undoType == KWActionGroup::ugDelChar
        && newUndoType == KWActionGroup::ugDelLine)) {

      undoCount++;
      if (g->undoType != newUndoType) undoCount = 0xffffff;
      return;
    }
  }
  undoCount = 0;
/*
  if (undoView != view) {
    // always kill the current undo group if the editing view changes
    recordReset();
    undoType = newUndoType;
  } else if (newUndoType == undoType) {
printf("bla!!!\n");
    // same as current type, keep using it
    return;
  } else if  ( (undoType == KWActionGroup::ugInsChar && newUndoType == KWActionGroup::ugInsLine) ||
               (undoType == KWActionGroup::ugDelChar && newUndoType == KWActionGroup::ugDelLine) ) {
    // some type combinations can run together...
    undoType += 1000;
    return;
  } else {
    recordReset();
    undoType = newUndoType;
  }

  undoView = view;
*/
  while ((int) undoList.count() > currentUndo) undoList.removeLast();
  while ((int) undoList.count() > undoSteps) {
    undoList.removeFirst();
    currentUndo--;
  }

  g = new KWActionGroup(cursor, newUndoType);
  undoList.append(g);
//  currentUndo++;

  unmarkFound();
  tagEnd = 0;
  tagStart = 0xffffff;
}

void KWriteDoc::recordAction(KWAction::Action action, PointStruc &cursor) {
  KWAction *a;

  a = new KWAction(action, cursor);
  doAction(a);
  undoList.getLast()->insertAction(a);
}

void KWriteDoc::recordInsert(PointStruc &cursor, const QString &text) {
  recordReplace(cursor, 0, text);
}

void KWriteDoc::recordDelete(PointStruc &cursor, int len) {
  recordReplace(cursor, len, QString::null);
}

void KWriteDoc::recordReplace(PointStruc &cursor, int len, const QString &text) {
  KWAction *a;
  TextLine *textLine;
  int l;

  if (len == 0 && text.isEmpty()) return;

  //try to append to last replace action
  a = undoList.getLast()->action;
  if (a == 0L || a->action != KWAction::replace
    || a->cursor.x + a->len != cursor.x || a->cursor.y != cursor.y) {

//if (a != 0L) printf("new %d %d\n", a->cursor.x + a->len, cursor.x);
    a = new KWAction(KWAction::replace, cursor);
    undoList.getLast()->insertAction(a);
  }

  //replace
  textLine = contents.at(cursor.y);
  l = textLine->length() - cursor.x;
  if (l > len) l = len;
  a->text.insert(a->text.length(), &textLine->getText()[cursor.x], (l < 0) ? 0 : l);
  textLine->replace(cursor.x, len, text.unicode(), text.length());
  a->len += text.length();

  tagLine(a->cursor.y);
}

void KWriteDoc::recordEnd(VConfig &c) {
  recordEnd(c.view, c.cursor, c.flags);
}

void KWriteDoc::recordEnd(KWriteView *view, PointStruc &cursor, int flags) {
  KWActionGroup *g;

  // clear selection if option "persistent selections" is off
//  if (!(flags & cfPersistent)) deselectAll();

  g = undoList.getLast();
  if (g->action == 0L) {
    // no action has been done: remove empty undo record
    undoList.removeLast();
    return;
  }
  // store end cursor position for redo
  g->end = cursor;
  currentUndo = undoList.count();

  view->updateCursor(cursor, flags);

  if (tagStart <= tagEnd) {
    optimizeSelection();
    updateLines(tagStart,tagEnd,flags);
    setModified(true);
  }

//  newUndo();
/*
  undoCount++;
  // we limit the number of individual undo operations for sanity - is 1K reasonable?
  // this is also where we handle non-group undo preference
  // if the undo type is singlular, we always finish it now
  if ( undoType == KWActionGroup::ugPaste ||
       undoType == KWActionGroup::ugDelBlock ||
       undoType > 1000 ||
       undoCount > 1024 || !(flags & cfGroupUndo) ) {
printf("recordend %d %d\n", undoType, undoCount);
    recordReset();
  }
*/

  // this should keep the flood of signals down a little...
  if (undoCount == 0) newUndo();
}
/*
void KWriteDoc::recordReset()
{
  if (pseudoModal)
    return;

  // forces the next call of recordStart() to begin a new undo group
  // not used in normal editing, but used by markFound(), etc.
  undoType = KWActionGroup::ugNone;
  undoCount = 0;
  undoView = NULL;
  undoReported = false;
printf("recordreset\n");
}
*/

/*
void KWriteDoc::recordDel(PointStruc &cursor, TextLine *textLine, int l) {
  int len;

  len = textLine->length() - cursor.x;
  if (len > l) len = l;
  if (len > 0) {
    insertUndo(new KWAction(KWAction::replace,cursor,&textLine->getText()[cursor.x],len));
  }
}
*/


void KWriteDoc::doActionGroup(KWActionGroup *g, int flags, bool undo) {
  KWAction *a, *next;

  setPseudoModal(0L);
  if (!(flags & cfPersistent)) deselectAll();
  unmarkFound();
  tagEnd = 0;
  tagStart = 0xffffff;

  a = g->action;
  g->action = 0L;
  while (a) {
    doAction(a);
    next = a->next;
    g->insertAction(a);
    a = next;
  }
  optimizeSelection();
  if (tagStart <= tagEnd) updateLines(tagStart,tagEnd,flags);

  // the undo/redo functions set undo to true, all others should leave it
  // alone (default)
  if (! undo) {
    setModified(true);
    newUndo();
  }
}

int KWriteDoc::nextUndoType()
{
  KWActionGroup *g;

  if (currentUndo <= 0) return KWActionGroup::ugNone;
  g = undoList.at(currentUndo - 1);
  return g->undoType;
}

int KWriteDoc::nextRedoType()
{
  KWActionGroup *g;

  if (currentUndo >= (int) undoList.count()) return KWActionGroup::ugNone;
  g = undoList.at(currentUndo);
//  if (!g) return KWActionGroup::ugNone;
  return g->undoType;
}

void KWriteDoc::undoTypeList(std::list<int> &lst)
{
  KWActionGroup *g;
  int   currUndo = currentUndo;

  lst.clear();

  while (1) {
    if (currUndo <= 0) return;
    currUndo--;
    g = undoList.at(currUndo);
    lst.push_back(g->undoType);
  }
}

void KWriteDoc::redoTypeList(std::list<int> &lst)
{
  KWActionGroup *g;
  int   currUndo = currentUndo;

  lst.clear();

  while (1) {
    if (currUndo+1 > (int)undoList.count()) return;
    g = undoList.at(currUndo);
    currUndo++;
    lst.push_back(g->undoType);
  }
}

void KWriteDoc::undo(VConfig &c, int count) {
  KWActionGroup *g = 0L;
  int num;
  bool needUpdate = false; // don't update the cursor until completely done

  if (count <= 0) return;

  for (num = 0 ; num < count ; num++) {
    if (currentUndo <= 0) break;
    currentUndo--;
    g = undoList.at(currentUndo);
    doActionGroup(g, c.flags, true); // do not setModified() or newUndo()
    needUpdate = true;

//    if (num == 0) recordReset();
  }

  if (needUpdate) {
    // since we told doActionGroup() not to do this stuff, we need to do it now
    c.view->updateCursor(g->start);
    setModified(true);
    newUndo();
  }
}

void KWriteDoc::redo(VConfig &c, int count) {
  KWActionGroup *g = 0L;
  int num;
  bool needUpdate = false; // don't update the cursor until completely done

  if (count <= 0) return;

  for (num = 0 ; num < count ; num++) {
    if (currentUndo+1 > (int)undoList.count()) break;
    g = undoList.at(currentUndo);
    currentUndo++;
    doActionGroup(g, c.flags, true); // do not setModified() or newUndo()
    needUpdate = true;

//    if (num == 0) recordReset();
  }

  if (needUpdate) {
    // since we told doActionGroup() not to do this stuff, we need to do it now
    c.view->updateCursor(g->end);
    setModified(true);
    newUndo();
  }
}

void KWriteDoc::clearRedo() {
  // disable redos
  // this was added as an assist to the spell checker
  bool deleted = false;

  while ((int) undoList.count() > currentUndo) {
    deleted = true;
    undoList.removeLast();
  }

  if (deleted) newUndo();
}

void KWriteDoc::setUndoSteps(int steps) {
  if (steps < 5) steps = 5;
  undoSteps = steps;
}

void KWriteDoc::setPseudoModal(QWidget *w) {
//  QWidget *old = pseudoModal;

  // (glenebob)
  // this is a temporary hack to make the spell checker work a little
  // better - as kspell progresses, this sort of thing should become
  // obsolete or worked around more cleanly
  // this is relied upon *only* by the spell-check code
  if (pseudoModal && pseudoModal != (QWidget*)1L)
    delete pseudoModal;

//  pseudoModal = 0L;
//  if (old || w) recordReset();

  pseudoModal = w;
}

void KWriteDoc::doIndent(VConfig &c, int change) {

//  int cursor_y = c.cursor.y;
// c.flags |= cfKeepIndentProfile;// | cfKeepExtraSpaces;
  c.cursor.x = 0;

  recordStart(c, (change < 0) ? KWActionGroup::ugUnindent
    : KWActionGroup::ugIndent);

  if (selectEnd < selectStart) {
    // single line
    optimizeLeadingSpace(c.cursor.y, c.flags, change);
  } else {
    // entire selection
    TextLine *textLine;
    int line, z;
    QChar ch;

    if (c.flags & cfKeepIndentProfile && change < 0) {
      // unindent so that the existing indent profile doesn´t get screwed
      // if any line we may unindent is already full left, don't do anything
      for (line = selectStart; line <= selectEnd; line++) {
        textLine = contents.at(line);
        if (textLine->isSelected() || textLine->numSelected()) {
          for (z = 0; z < tabChars; z++) {
            ch = textLine->getChar(z);
            if (ch == '\t') break;
            if (ch != ' ') {
              change = 0;
              goto jumpOut;
            }
          }
        }
      }
      jumpOut:;
    }

    for (line = selectStart; line <= selectEnd; line++) {
      textLine = contents.at(line);
      if (textLine->isSelected() || textLine->numSelected()) {
        optimizeLeadingSpace(line, c.flags, change);
      }
    }
  }
  // recordEnd now removes empty undo records
  recordEnd(c.view, c.cursor, c.flags | cfPersistent);
}

/*
  Optimize the leading whitespace for a single line.
  If change is > 0, it adds indentation units (tabChars)
  if change is == 0, it only optimizes
  If change is < 0, it removes indentation units
  This will be used to indent, unindent, and optimal-fill a line.
  If excess space is removed depends on the flag cfKeepExtraSpaces
  which has to be set by the user
*/
void KWriteDoc::optimizeLeadingSpace(int line, int flags, int change) {
  TextLine *textLine;
  int len;
  int chars, space, okLen;
  QChar ch;
  int extra;
  QString s;
  PointStruc cursor;

  textLine = contents.at(line);
  len = textLine->length();
  space = 0; // length of space at the beginning of the textline
  okLen = 0; // length of space which does not have to be replaced
  for (chars = 0; chars < len; chars++) {
    ch = textLine->getChar(chars);
    if (ch == ' ') {
      space++;
      if (flags & cfReplaceTabs && okLen == chars) okLen++;
    } else if (ch == '\t') {
      space += tabChars - space % tabChars;
      if (!(flags & cfReplaceTabs) && okLen == chars) okLen++;
    } else break;
  }

  space += change*tabChars; // modify space width
  // if line contains only spaces it will be cleared
  if (space < 0 || chars == len) space = 0;

  extra = space % tabChars; // extra spaces which don´t fit the indentation pattern
  if (flags & cfKeepExtraSpaces) chars -= extra;

  if (flags & cfReplaceTabs) {
    space -= extra;
    ch = ' ';
  } else {
    space /= tabChars;
    ch = '\t';
  }

  // don´t replace chars which are already ok
  cursor.x = QMIN(okLen, QMIN(chars, space));
  chars -= cursor.x;
  space -= cursor.x;
  if (chars == 0 && space == 0) return; //nothing to do

  s.fill(ch, space);

//printf("chars %d insert %d cursor.x %d\n", chars, insert, cursor.x);
  cursor.y = line;
  recordReplace(cursor, chars, s);
}

/*
  Optimize the leading whitespace for a single provided line,
  and can also add or delete one unit of indentation (tabChars).
  This will be used to indent, unindent, and optimal-fill a line.
  In the case where this is used to indent/unindent a line, we always want
  to remove excess space, leaving only an optimized amount of whitespace.
  If an optimization is actually done and bool undo is true, recordStart()
  is called and true is returned, otherwise false is returned.

  glenebob@nwlink.com
*/
/*
bool KWriteDoc::optimizeLeadingSpace(VConfig &c, TextLine *textLine, int change, bool undo)
{
  int len = textLine->length();

  // if this line is empty, bail now
  if (! len)
    return false;

  int space = 0;      // existing space
  int optimized = 0;  // how much space we want
  int excess = 0;     // how much space in excess of an even optimized amount we have
  QString oldWS;      // the actual whitespace text we have
    */
  /*
    calculate three things:
    1) the whitespace char count we currently have.
    2) the optimized whitespace we want to have.
    3) the excess white space we have that does not add up to a full
       tabChar width - this will be removed.
    */ /*
  for (int i = 0 ;; i++) {
    if (i == len) {
      // line is only white space - it will be cleared
      optimized = 0;
      excess = 0;
      break;
    } else if (textLine->getChar(i) == '\t') {
      space++;
      optimized++;
      excess = 0;
      oldWS += '\t';
    } else if (textLine->getChar(i) == ' ') {
      space++;
      excess++;
      if (excess == tabChars) {
        optimized++;
        excess = 0;
      }
      oldWS += ' ';
    } else {
      // no more whitespace
      break;
    }
  }

  // bail if there is nothing to actually be done
  if (change <= 0 && ! space)
    return false;

  // now we do the actual optimization based on our calculations

  QString     newWS;
  QString     newExc;

  if (space && ! optimized && ! excess) {
    // this was a white-space only line - clear it
    newWS = "";
  } else if (! change) {
    // no change - just optimize, so that the line still *looks* the same
    if (c.flags & cfReplaceTabs) {
      newWS.fill(' ', optimized * tabChars + excess);
    } else {
      newWS.fill('\t', optimized);
      newExc.fill(' ', excess);
      newWS += newExc;
    }
  } else if (change > 0) {
    // add 1 optimized space to the line
    optimized ++;
    if (c.flags & cfReplaceTabs)
      newWS.fill(' ', optimized * tabChars);
    else
      newWS.fill('\t', optimized);
  } else if (change < 0) {
    // remove 1 optimized space from the line
    if (! excess)
      optimized --;
    if (c.flags & cfReplaceTabs)
      newWS.fill(' ', optimized * tabChars);
    else
      newWS.fill('\t', optimized);
  }

  // make sure our optimized indent is actually different than what was there
  if (oldWS == newWS)
    return false;

  // replace the current whitespace with the optimized whitespace
  if (undo) // start an undo record
    recordStart(c, (change < 0 ? KWActionGroup::ugUnindent : KWActionGroup::ugIndent));

  recordReplace(c.cursor, space, newWS);

  return undo;
} */

void KWriteDoc::newBracketMark(PointStruc &cursor, BracketMark &bm) {
  TextLine *textLine;
  int x, line, count, attr;
  char bracket, opposite, ch;
  Attribute *a;

  bm.eXPos = -1; //mark bracked mark as invalid

  x = cursor.x -1; // -1 to look at left side of cursor
  if (x < 0) return;
  line = cursor.y; //current line
  count = 0; //bracket counter for nested brackets
  textLine = contents.at(line);
  bracket = textLine->getChar(x);
  attr = textLine->getAttr(x);
  if (bracket == '(' || bracket == '[' || bracket == '{') {
    //get opposite bracket
    opposite = ')';
    if (bracket == '[') opposite = ']';
    if (bracket == '{') opposite = '}';
    //get attribute of bracket (opposite bracket must have the same attribute)
    x++;
    while (line - cursor.y < 40) {
      //go to next line on end of line
      while (x >= textLine->length()) {
        line++;
        if (line > lastLine()) return;
        textLine = contents.at(line);
        x = 0;
      }
      if (textLine->getAttr(x) == attr) {
        //try to find opposite bracked
        ch = textLine->getChar(x);
        if (ch == bracket) count++; //same bracket : increase counter
        if (ch == opposite) {
          count--;
          if (count < 0) goto found;
        }
      }
      x++;
    }
  } else if (bracket == ')' || bracket == ']' || bracket == '}') {
    opposite = '(';
    if (bracket == ']') opposite = '[';
    if (bracket == '}') opposite = '{';
    x--;
    while (cursor.y - line < 20) {

      while (x < 0) {
        line--;
        if (line < 0) return;
        textLine = contents.at(line);
        x = textLine->length() -1;
      }
      if (textLine->getAttr(x) == attr) {
        ch = textLine->getChar(x);
        if (ch == bracket) count++;
        if (ch == opposite) {
          count--;
          if (count < 0) goto found;
        }
      }
      x--;
    }
  }
  return;
found:
  //cursor position of opposite bracket
  bm.cursor.x = x;
  bm.cursor.y = line;

  //x position (start and end) of related bracket
  bm.sXPos = textWidth(textLine, x);
  a = &attribs[attr];
  bm.eXPos = bm.sXPos + a->width(bracket);//a->fm.width(bracket);
}
