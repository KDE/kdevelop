#include <stdio.h>
#include <qobject.h>
#include <qapp.h>
#include <qclipbrd.h>
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

void TextLine::move(int pos, int n) {

  if (pos >= len) {
    resize(pos + n);
    memset(&text[len],' ',pos - len);
    memset(&attribs[len],attr,pos - len + n);
    len = pos;
  } else {
    resize(len + n);
    memmove(&text[pos + n],&text[pos],len - pos);
    memmove(&attribs[pos + n],&attribs[pos],len - pos);
    memset(&attribs[pos],attribs[pos],n);
  }
  len += n;
}

/*
void TextLine::ins(int pos, char c, int n) {

  move(pos,n);
  memset(&text[pos],c,n);
}

void TextLine::ovr(int pos, char c, int n) {

  if (pos + n > len) {
    resize(pos + n);
    if (pos > len) memset(&text[len],' ',pos - len);
    memset(&attribs[len],attr,pos + n - len);
    len = pos + n;
  }
  memset(&text[pos],c,n);
}
*/

void TextLine::insert(int pos, const char *s, int l) {

  move(pos,l);
  memcpy(&text[pos],s,l);
}


void TextLine::overwrite(int pos, const char *s, int l) {

//  if (n == -1) n = strlen(s);
  if (pos + l > len) {
    resize(pos + l);
    if (pos > len) memset(&text[len],' ',pos - len);
    memset(&attribs[len],attr,pos + l - len);
    len = pos + l;
  }
  memcpy(&text[pos],s,l);
}

void TextLine::append(char c, int n) {

  resize(len + n);
  memset(&text[len],c,n);
  memset(&attribs[len],attr,n);
  len += n;
}

void TextLine::del(int pos, int n) {
  int count;

  count = len - (pos + n);
  if (count <= 0) {
    if (len > pos) len = pos;
  } else {
    memmove(&text[pos],&text[pos + n],count);
    memmove(&attribs[pos],&attribs[pos + n],count);
    len -= n;
  }
}

int TextLine::length() {
  return len;
}

void TextLine::setLength(int l) {
  if (l >= len) {
    resize(l);
    memset(&text[len],' ',l - len);
    memset(&attribs[len],attr,l - len);
  } else attr = attribs[l];
  len = l;
}
/*
void TextLine::truncate(int l) {
  if (l < len) {
    attr = attribs[l];
    len = l;
  }
}
*/
void TextLine::wrap(TextLine *nextLine, int pos) {
  int l;

  l = len - pos;
  if (l > 0) {
    nextLine->move(0,l);
    memcpy(nextLine->text,&text[pos],l);
    memcpy(nextLine->attribs,&attribs[pos],l);
    attr = attribs[pos];
    len = pos;
  }
}

void TextLine::unWrap(TextLine *nextLine, int pos) {
  
  if (nextLine->len < pos) pos = nextLine->len;
  resize(len + pos);
  memcpy(&text[len],nextLine->text,pos);
  memcpy(&attribs[len],nextLine->attribs,pos);
  len += pos;
  attr = nextLine->getRawAttr(pos);
  nextLine->del(0,pos);
}

void TextLine::removeSpaces() {

  while (len > 0 && text[len - 1] == ' ') len--;
}

int TextLine::firstChar() {
  int z;

  z = 0;
  while (z < len && text[z] <= 32) z++;
  return (z < len) ? z : -1;
}

char TextLine::getChar( int pos ) const {
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

int TextLine::getAttr(int pos) {
  if (pos < len) return attribs[pos] & taAttrMask;
  return attr & taAttrMask;
}

int TextLine::getAttr() {
  return attr & taAttrMask;
}

int TextLine::getRawAttr(int pos) {
  if (pos < len) return attribs[pos];
  return (attr & taSelectMask) ? attr : attr | 256;
}

int TextLine::getRawAttr() {
  return attr;
}

void TextLine::setContext(int context) {
  ctx = context;
}

int TextLine::getContext() {
  return ctx;
}

const char* TextLine::getString() {
  resize(len+1);
  text[len] = '\0';
  return text;
}

const char* TextLine::getText() {
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


int TextLine::numSelected() {
  int z, n;

  n = 0;
  for (z = 0; z < len; z++) if (attribs[z] & taSelected) n++;
  return n;
}

bool TextLine::isSelected(int pos) {
  if (pos < len) return (attribs[pos] & taSelected);
  return (attr & taSelected);
}

bool TextLine::isSelected() {
  return (attr & taSelected);
}

int TextLine::findSelected(int pos) {
  while (pos < len && attribs[pos] & taSelected) pos++;
  return pos;
}

int TextLine::findUnSelected(int pos) {
  while (pos < len && !(attribs[pos] & taSelected)) pos++;
  return pos;
}

int TextLine::findRevSelected(int pos) {
  while (pos > 0 && attribs[pos - 1] & taSelected) pos--;
  return pos;
}

int TextLine::findRevUnSelected(int pos) {
  while (pos > 0 && !(attribs[pos - 1] & taSelected)) pos--;
  return pos;
}


int TextLine::cursorX(int pos, int tabChars) {
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


void TextLine::resize(int newsize) {
  char *newtext;
  unsigned char *newattribs;

  if (newsize > size) {
    if (newsize*2 < size*3) newsize = (size*3)/2;
    size = (newsize + 15) & (~15);

    newtext = new char[size];
    newattribs = new unsigned char[size];
if (!newtext || !newattribs) {
  printf("error resizing textline\n");
  exit(1);
}
    memcpy(newtext,text,len);
    memcpy(newattribs,attribs,len);
    delete text;
    delete attribs;
    text = newtext;
    attribs = newattribs;
  }
}


Attribute::Attribute() : font(), fm(font) {
}

void Attribute::setFont(const QFont &f) {
  font = f;
  fm = QFontMetrics(f);
}


KWAction::KWAction(Action a, PointStruc &aCursor)
  : action(a), cursor(aCursor), len(0), text(0L), textLen(0) {
}

KWAction::~KWAction() {
  delete text;
}

void KWAction ::setData(int aLen, const char *aText, int aTextLen) {

  len = aLen;
  if (aTextLen > 0) {
    text = new char[aTextLen];
    memcpy((char *) text,aText,aTextLen);
    textLen = aTextLen;
  } else {
    text = 0L;
    textLen = 0;
  }
}

KWActionGroup::KWActionGroup(PointStruc &aStart) : start(aStart), action(0L) {
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


KWriteDoc::KWriteDoc(HlManager *hlManager)
  : QObject(0L), hlManager(hlManager) {

  contents.setAutoDelete(true);

  colors[0] = white;
  colors[1] = darkBlue;
  colors[2] = black;
  colors[3] = black;
  colors[4] = white;

  highlight = 0L;
  tabChars = 8;

  newDocGeometry = false;
  modified = false;

  undoList.setAutoDelete(true);
  undoState = 0;
  undoSteps = 50;

  pseudoModal = 0L;
  clear();

  setHighlight(0); //calls updateFontData()
  connect(hlManager,SIGNAL(changed()),SLOT(hlChanged()));
}

KWriteDoc::~KWriteDoc() {
  highlight->release();
}

int KWriteDoc::lastLine() const {
  return (int) contents.count() - 1;
}

TextLine *KWriteDoc::textLine(int line) {
//  if (line < 0) line = 0;
//  if (line >= (int) contents.count()) line = contents.count() -1;
  return contents.at(line);
}

int KWriteDoc::textLength(int line) {
  return contents.at(line)->length();
}

void KWriteDoc::tagLines(int start, int end) {
  int z;

  for (z = 0; z < (int) views.count(); z++) {
    views.at(z)->tagLines(start,end);
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
  config->writeEntry("Highlight",highlight->name());
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
}

void KWriteDoc::registerView(KWriteView *view) {
  views.append(view);
}

void KWriteDoc::removeView(KWriteView *view) {
  views.remove(view);
}


int KWriteDoc::currentColumn(PointStruc &cursor) {
  return contents.at(cursor.y)->cursorX(cursor.x,tabChars);
}

void KWriteDoc::insert(KWriteView *view, VConfig &c, const char *s) {
  char b[256];
  int pos;

  if (!s || !*s) return;
  recordStart(c.cursor);
  pos = 0;
  if (!(c.flags & cfVerticalSelect)) {
    while (*s != 0) {
      if ((unsigned char) *s >= 32 || *s == '\t') {
        b[pos] = *s;
        pos++;
      } else if (*s == '\n') {
        recordAction(KWAction::newLine,c.cursor);
        recordReplace(c.cursor,0,b,pos);
        c.cursor.y++;
        c.cursor.x = 0;
        pos = 0;
      }
      if (pos >= 256) {
        recordReplace(c.cursor,0,b,pos);
        c.cursor.x += pos;
        pos = 0;
      }
      s++;
    }
    if (pos > 0) {
      recordReplace(c.cursor,0,b,pos);
      c.cursor.x += pos;
    }
  } else {
    int xPos;

    xPos = textWidth(c.cursor);
    while (*s != 0) {
      if ((unsigned char) *s >= 32 || *s == '\t') {
        b[pos] = *s;
        pos++;
      } else if (*s == '\n') {
        recordReplace(c.cursor,0,b,pos);
        c.cursor.y++;
        if (c.cursor.y >= (int) contents.count()) {
          recordAction(KWAction::insLine,c.cursor);
        }
        c.cursor.x = textPos(contents.at(c.cursor.y),xPos);
        pos = 0;
      }
      s++;
      if (pos >= 256 || *s == 0) {
        recordReplace(c.cursor,0,b,pos);
        c.cursor.x += pos;
        pos = 0;
      }
    }
  }
  recordEnd(view,c);
}

void KWriteDoc::insertFile(KWriteView *view, VConfig &c, QIODevice &dev) {
  char buf[256];
  int len;
  char b[256];
  int pos;
  char *s;

  recordStart(c.cursor);
  pos = 0;
  do {
    len = dev.readBlock(buf,256);
    s = buf;
    while (len > 0) {
      if ((unsigned char) *s >= 32 || *s == '\t') {
        b[pos] = *s;
        pos++;
      } else if (*s == '\n') {
        recordAction(KWAction::newLine,c.cursor);
        recordReplace(c.cursor,0,b,pos);
        c.cursor.y++;
        c.cursor.x = 0;
        pos = 0;
      }
      if (pos >= 256) {
        recordReplace(c.cursor,0,b,pos);
        c.cursor.x += pos;
        pos = 0;
      }
      s++;
      len--;
    }
  } while (s != buf);
  if (pos > 0) {
    recordReplace(c.cursor,0,b,pos);
    c.cursor.x += pos;
  }
  recordEnd(view,c);
}

void KWriteDoc::loadFile(QIODevice &dev) {
  TextLine *textLine;
  char buf[512];
  int len;
  char *s;

  clear();

  textLine = contents.getFirst();
  do {
    len = dev.readBlock(buf,512);
    s = buf;
    while (len > 0) {
      if ((unsigned char) *s >= 32 || *s == '\t') {
        textLine->append(*s);
      } else if (*s == '\n') {
        textLine = new TextLine();
        contents.append(textLine);
      } else if (*s == '\r') {
        printf("dos text\n");
      }
      s++;
      len--;
    }
  } while (s != buf);

//  updateLines();
}

void KWriteDoc::writeFile(QIODevice &dev) {
  TextLine *textLine;

  textLine = contents.first();
  do {
    dev.writeBlock(textLine->getText(),textLine->length());
    textLine = contents.next();
    if (!textLine) break;
    dev.putch('\n');
  } while (true);
}

void KWriteDoc::insertChar(KWriteView *view, VConfig &c, char ch) {
  TextLine *textLine;
  int l, z;
  char buf[20];

  textLine = contents.at(c.cursor.y);

  if (ch == '\t' && c.flags & cfReplaceTabs) {
    l = tabChars - (textLine->cursorX(c.cursor.x,tabChars) % tabChars);
    for (z = 0; z < l; z++) buf[z] = ' ';
  } else {
    z = l = 1;
    buf[0] = ch;
    if (c.flags & cfAutoBrackets) {
      if (ch == '(') buf[l++] = ')';
      if (ch == '[') buf[l++] = ']';
      if (ch == '{') buf[l++] = '}';
    }
  }

  //do nothing if spaces will be removed
  if (buf[0] == ' ' && c.flags & cfRemoveSpaces && c.cursor.x >= textLine->length()) {
    c.cursor.x += z;
    view->updateCursor(c.cursor);
    return;
  }
  recordStart(c.cursor);

  recordReplace(c.cursor,(c.flags & cfOvr) ? l : 0,buf,l);
  c.cursor.x += z;

  if (c.flags & cfWordWrap) {
    int line;
    const unsigned char *s;
    int pos;
    PointStruc actionCursor;

    if (!(c.flags & cfPersistent)) deselectAll();
    line = c.cursor.y;
    do {
      textLine = contents.at(line);
      s = (unsigned char *) textLine->getText();
      l = textLine->length();
      for (z = c.wrapAt; z < l; z++) if (s[z] > 32) break; //search for text to wrap
      if (z >= l) break; // nothing more to wrap
      pos = c.wrapAt;
      for (; z >= 0; z--) { //find wrap position
        if (s[z] <= 32) {
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

      if (textLine == contents.getLast()) {
        //at end of doc: create new line
        actionCursor.x = pos;
        actionCursor.y = line;
        recordAction(KWAction::newLine,actionCursor);
      } else {
        //wrap
        actionCursor.y = line + 1;
        if (s[l - 1] > 32) { //add space in next line if necessary
          actionCursor.x = 0;
          recordReplace(actionCursor,0," ",1);
        }
        actionCursor.x = textLine->length() - pos;
        recordAction(KWAction::wordWrap,actionCursor);
      }
      line++;
    } while (true);
  }
  recordEnd(view,c);
}

void KWriteDoc::newLine(KWriteView *view, VConfig &c) {

  recordStart(c.cursor);

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
      recordReplace(c.cursor,0,textLine->getText(),pos);
      c.cursor.x = pos;
    }
  }

  recordEnd(view,c);
}

void KWriteDoc::killLine(KWriteView *view, VConfig &c) {

  recordStart(c.cursor);
  c.cursor.x = 0;
  recordReplace(c.cursor,0xffffff);
  if (c.cursor.y < (int) contents.count() - 1) {
    recordAction(KWAction::killLine,c.cursor);
  }
  recordEnd(view,c);
}

void KWriteDoc::backspace(KWriteView *view, VConfig &c) {

  if (c.cursor.x <= 0 && c.cursor.y <= 0) return;
  recordStart(c.cursor);
  if (c.cursor.x > 0) {
    if (!(c.flags & cfBackspaceIndent)) {
      c.cursor.x--;
      recordReplace(c.cursor,1);
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
      recordReplace(c.cursor,l);
    }
  } else {
    c.cursor.y--;
    c.cursor.x = contents.at(c.cursor.y)->length();
    recordAction(KWAction::delLine,c.cursor);
  }
  recordEnd(view,c);
}


void KWriteDoc::del(KWriteView *view, VConfig &c) {

  if (c.cursor.x < contents.at(c.cursor.y)->length()) {
    recordStart(c.cursor);
    recordReplace(c.cursor,1);
    recordEnd(view,c);
  } else {
    if (c.cursor.y < (int) contents.count() -1) {
      recordStart(c.cursor);
      recordAction(KWAction::delLine,c.cursor);
      recordEnd(view,c);
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
    endCtx = textLine->getContext();
    highlight->doHighlight(ctxNum,textLine);
    ctxNum = textLine->getContext();
    line++;
  } while (line <= lastLine && (line <= endLine || endCtx != ctxNum));
  tagLines(startLine,line - 1);

  for (line = startLine; line <= endLine; line++) {
    textLine = contents.at(line);
    updateMaxLength(textLine);
    if (flags & cfRemoveSpaces) textLine->removeSpaces();
  }
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

  flags = (newDocGeometry) ? ufDocGeometry : 0;
  for (view = views.first(); view != 0L; view = views.next()) {
    if (view != exclude) view->updateView(flags);
  }
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
    x += (ch == '\t') ? tabWidth - (x % tabWidth) : a->fm.width(&ch,1);
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
    x += (ch == '\t') ? tabWidth - (x % tabWidth) : a->fm.width(&ch,1);
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
  int len;
  int x, oldX;
  int z;
  char ch;
  Attribute *a;

  len = textLine->length();

  x = oldX = z = 0;
  while (x < xPos) { // && z < len) {
    oldX = x;
    ch = textLine->getChar(z);
    a = &attribs[textLine->getAttr(z)];
    x += (ch == '\t') ? tabWidth - (x % tabWidth) : a->fm.width(&ch,1);
    z++;
  }
  if (xPos - oldX < x - xPos && z > 0) z--;
  return z;
}

int KWriteDoc::textWidth() {
  return maxLength + 8;
}

int KWriteDoc::textHeight() {
  return contents.count()*fontHeight;
}

void KWriteDoc::toggleRect(int x1, int y1, int x2, int y2) {
  int z;
  bool t;
  int start, end;
  TextLine *textLine;

  if (x1 > x2) {
    z = x1;
    x1 = x2;
    x2 = z;
  }
  if (y1 > y2) {
    z = y1;
    y1 = y2;
    y2 = z;
  }

  t = false;
  for (z = y1; z < y2; z++) {
    textLine = contents.at(z);
    start = textPos(textLine,x1);
    end = textPos(textLine,x2);
    if (end > start) {
      textLine->toggleSelect(start,end);
      t = true;
    }
  }
  if (t) {
    y2--;
    tagLines(y1,y2);

    if (y1 < selectStart) selectStart = y1;
    if (y2 > selectEnd) selectEnd = y2;
  }
}

void KWriteDoc::selectTo(PointStruc &start, PointStruc &end, int flags) {

  if (start.x != select.x || start.y != select.y) {
    /* new selection */
    if (!(flags & cfKeepSelection)) deselectAll();
    anchor = start;
  }

  if (!(flags & cfVerticalSelect)) {
    //horizontal selections
    TextLine *textLine;
    int x, y;
    int xe, ye;
    bool sel;

    if (end.y > start.y || (end.y == start.y && end.x > start.x)) {
      x = start.x;
      y = start.y;
      xe = end.x;
      ye = end.y;
      sel = true;
    } else {
      x = end.x;
      y = end.y;
      xe = start.x;
      ye = start.y;
      sel = false;
    }
    tagLines(y,ye);

    if (y < selectStart) selectStart = y;
    if (ye > selectEnd) selectEnd = ye;

    textLine = contents.at(y);
//    bufferLine->copy(textLine);

    if (flags & cfXorSelect) {
      //xor selection with old selection
      while (y < ye) {
        textLine->toggleSelectEol(x);
//        optimizedDrawLine(paint,fm,*textLine,*bufferLine,y);
        x = 0;
        y++;
        textLine = contents.at(y);
//        bufferLine->copy(textLine);
      }
      textLine->toggleSelect(x,xe);
//      optimizedDrawLine(paint,fm,*textLine,*bufferLine,y);
    } else {
      //set selection over old selection

      if (anchor.y > y || (anchor.y == y && anchor.x > x)) {
        if (anchor.y < ye || (anchor.y == ye && anchor.x < xe)) {
          sel = !sel;
          while (y < anchor.y) {
            textLine->selectEol(sel,x);
//            optimizedDrawLine(paint,fm,*textLine,*bufferLine,y);
            x = 0;
            y++;
            textLine = contents.at(y);
//            bufferLine->copy(textLine);
          }
          textLine->select(sel,x,anchor.x);
          x = anchor.x;
        }
        sel = !sel;
      }
      while (y < ye) {
        textLine->selectEol(sel,x);
//        optimizedDrawLine(paint,fm,*textLine,*bufferLine,y);
        x = 0;
        y++;
        textLine = contents.at(y);
//        bufferLine->copy(textLine);
      }
      textLine->select(sel,x,xe);
//      optimizedDrawLine(paint,fm,*textLine,*bufferLine,y);
    }
  } else {
    //vertical (block) selections
    int ax, sx, ex;

    ax = textWidth(anchor);
    sx = textWidth(start);
    ex = textWidth(end);

    toggleRect(ax,start.y + 1,sx,end.y + 1);
    toggleRect(sx,anchor.y,ex,end.y + 1);
//    toggleRect(ax,start.y + 1,sx,end.y + 1);
//    toggleRect(sx,anchor.y,ex,end.y);
  }
  select = end;
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

  contents.clear();

  contents.append(longestLine = new TextLine());
  maxLength = 0;

  select.x = -1;

  selectStart = 0xffffff;
  selectEnd = 0;
//  selectStart = 0;
//  selectEnd = -1;

  foundLine = -1;

  setModified(false);

  undoList.clear();
  currentUndo = 0;
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

void KWriteDoc::paste(KWriteView *view, VConfig &c) {
  QString s = QApplication::clipboard()->text();
  if (!s.isEmpty()) {
//    unmarkFound();
    insert(view,c,s);
  }
}


void KWriteDoc::cut(KWriteView *view, VConfig &c) {

  if (selectEnd < selectStart) return;

//  unmarkFound();
  copy(c.flags);
  delMarkedText(view,c);
}

void KWriteDoc::selectAll() {
  int z;
  TextLine *textLine;

  select.x = -1;

  unmarkFound();
  selectStart = 0;
  selectEnd = contents.count() -1;

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
  tagLines(selectStart,selectEnd);

  for (z = selectStart; z <= selectEnd; z++) {
    textLine = contents.at(z);
    textLine->selectEol(false,0);
  }
  selectEnd = -1;
}



void KWriteDoc::invertSelection() {
  int z;
  TextLine *textLine;

  select.x = -1;

  unmarkFound();
  selectStart = 0;
  selectEnd = contents.count() -1;

  tagLines(selectStart,selectEnd);

  for (z = selectStart; z < selectEnd; z++) {
    textLine = contents.at(z);
    textLine->toggleSelectEol(0);
  }
  textLine = contents.at(z);
  textLine->toggleSelect(0,textLine->length());
}


QString KWriteDoc::text() {
  TextLine *textLine;
  int len, last, z, end, i;

  len = 1;
  last = lastLine();
  for (z = 0; z <= last; z++) {
    textLine = contents.at(z);
    len += textLine->length() + 1;
  }
  QString s(len);
  len = 0;
  for (z = 0; z <= last; z++) {
    textLine = contents.at(z);
    end = textLine->length();
    for (i = 0; i < end; i++) s[len + i] = textLine->getChar(i);
    len += end;
    s[len] = '\n';
    len++;
  }
  s[len] = '\0';
  return s;
}

QString KWriteDoc::currentWord(PointStruc &cursor) {
  TextLine *textLine;
  int start, end, len, z;

  textLine = contents.at(cursor.y);
  len = textLine->length();
  start = end = cursor.x;
  while (start > 0 && highlight->isInWord(textLine->getChar(start - 1))) start--;
  while (end < len && highlight->isInWord(textLine->getChar(end))) end++;
  len = end - start;
  QString s(len +1);
  for (z = 0; z < len; z++) s[z] = textLine->getChar(start + z);
  s[len] = '\0';
  return s;
}

void KWriteDoc::setText(const char *s) {
  TextLine *textLine;

  clear();

  textLine = contents.getFirst();
  while (*s) {
    if ((unsigned char) *s >= 32 || *s == '\t') {
      textLine->append(*s);
    } else if (*s == '\n') {
      textLine = new TextLine();
      contents.append(textLine);
    }
    s++;
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
    QString s(len);
    len = 0;
    for (z = selectStart; z <= selectEnd; z++) {
      textLine = contents.at(z);
      end = 0;
      do {
        start = textLine->findUnSelected(end);
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
    QString s(len);
    len = 0;
    for (z = selectStart; z <= selectEnd; z++) {
      textLine = contents.at(z);
      end = 0;
      do {
        start = textLine->findUnSelected(end);
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

void KWriteDoc::delMarkedText(KWriteView *view, VConfig &c) {
  TextLine *textLine;
  int end = 0;

  if (selectEnd < selectStart) return;

  recordStart(c.cursor);

  for (c.cursor.y = selectEnd; c.cursor.y >= selectStart; c.cursor.y--) {
    textLine = contents.at(c.cursor.y);

    c.cursor.x = textLine->length();
    do {
      end = textLine->findRevUnSelected(c.cursor.x);
      if (end == 0) break;
      c.cursor.x = textLine->findRevSelected(end);
      recordReplace(c.cursor,end - c.cursor.x);
    } while (true);
    end = c.cursor.x;
    c.cursor.x = textLine->length();
    if (textLine->isSelected()) recordAction(KWAction::delLine,c.cursor);
  }
  c.cursor.y++;
  if (end < c.cursor.x) c.cursor.x = end;

  selectEnd = -1;
  select.x = -1;

  recordEnd(view,c);
}

QColor &KWriteDoc::cursorCol(int x, int y) {
  TextLine *textLine;
  int attr;
  Attribute *a;

//  if (x > 0) x--;
  textLine = contents.at(y);
  attr = textLine->getRawAttr(x);
  a = &attribs[attr & taAttrMask];
  if (attr & taSelectMask) return a->selCol; else return a->col;
}

/*
void KWriteDoc::paintTextLine(QPainter &paint, int line,
                              int xPos, int xStart, int xEnd, int yPos) {
  int y;
  TextLine *textLine;
  int z, x;
  char ch;
  Attribute *a = 0;
  int attr, nextAttr;
  int xs;
  int xc, zc;

  y = line*fontHeight - yPos;
  if (line >= (int) contents.count()) {
    paint.fillRect(xStart - xPos,y,xEnd - xStart,fontHeight,colors[4]);
    return;
  }
//printf("xStart = %d, xEnd = %d, line = %d\n",xStart,xEnd,line);
//printf("text = ");
  textLine = contents.at(line);

  z = 0;
  x = 0;
  do {
    xc = x;
    ch = textLine->getChar(z);
    if (ch == '\t') {
      x += tabWidth - (x % tabWidth);
    } else {
      a = attribs[textLine->getAttr(z)];
      x += a->fm.width(&ch,1);
    }
    z++;
  } while (x <= xStart);
  zc = z - 1;

  xs = xStart;
  attr = textLine->getRawAttr(zc);
  while (x < xEnd) {
    nextAttr = textLine->getRawAttr(z);
    if ((nextAttr ^ attr) & taSelectMask) {
      paint.fillRect(xs - xPos,y,x - xs,fontHeight,colors[attr >> taShift]);
      xs = x;
      attr = nextAttr;
    }
    ch = textLine->getChar(z);
    if (ch == '\t') {
      x += tabWidth - (x % tabWidth);
    } else {
      a = attribs[attr & taAttrMask];
      x += a->fm.width(&ch,1);
    }
    z++;
  }
  paint.fillRect(xs - xPos,y,xEnd - xs,fontHeight,colors[attr >> taShift]);
//int len = textLine->length();
  y += fontAscent -1;
  attr = -1;
  while (xc < xEnd) {
    ch = textLine->getChar(zc);
    if (ch == '\t') {
      xc += tabWidth - (xc % tabWidth);
    } else {
      nextAttr = textLine->getRawAttr(zc);
      if (nextAttr != attr) {
        attr = nextAttr;
        a = attribs[attr & taAttrMask];
        if (attr & taSelectMask) paint.setPen(a->selCol); else paint.setPen(a->col);
        paint.setFont(a->font);
      }
      paint.drawText(xc - xPos,y,&ch,1);
      xc += a->fm.width(&ch,1);
//if (zc < len) printf("%c",ch);
    }
    zc++;
  }

//printf("\n");
}
*/

void KWriteDoc::paintTextLine(QPainter &paint, int line,
                              int xStart, int xEnd) {
  int y;
  TextLine *textLine;
  int z, x;
  char ch;
  Attribute *a = 0;
  int attr, nextAttr;
  int xs;
  int xc, zc;

  y = 0;//line*fontHeight - yPos;
  if (line >= (int) contents.count()) {
    paint.fillRect(0,y,xEnd - xStart,fontHeight,colors[4]);
    return;
  }
//printf("xStart = %d, xEnd = %d, line = %d\n",xStart,xEnd,line);
//printf("text = ");
  textLine = contents.at(line);

  z = 0;
  x = 0;
  do {
    xc = x;
    ch = textLine->getChar(z);
    if (ch == '\t') {
      x += tabWidth - (x % tabWidth);
    } else {
      a = &attribs[textLine->getAttr(z)];
      x += a->fm.width(&ch,1);
    }
    z++;
  } while (x <= xStart);
  zc = z - 1;

  xs = xStart;
  attr = textLine->getRawAttr(zc);
  while (x < xEnd) {
    nextAttr = textLine->getRawAttr(z);
    if ((nextAttr ^ attr) & (taSelectMask | 256)) {
      paint.fillRect(xs - xStart,y,x - xs,fontHeight,colors[attr >> taShift]);
      xs = x;
      attr = nextAttr;
    }
    ch = textLine->getChar(z);
    if (ch == '\t') {
      x += tabWidth - (x % tabWidth);
    } else {
      a = &attribs[attr & taAttrMask];
      x += a->fm.width(&ch,1);
    }
    z++;
  }
  paint.fillRect(xs - xStart,y,xEnd - xs,fontHeight,colors[attr >> taShift]);
//int len = textLine->length();
  y += fontAscent -1;
  attr = -1;
  while (xc < xEnd) {
    ch = textLine->getChar(zc);
    if (ch == '\t') {
      xc += tabWidth - (xc % tabWidth);
    } else {
      nextAttr = textLine->getRawAttr(zc);
      if (nextAttr != attr) {
        attr = nextAttr;
        a = &attribs[attr & taAttrMask];
        if (attr & taSelectMask) paint.setPen(a->selCol); else paint.setPen(a->col);
        paint.setFont(a->font);
      }
      paint.drawText(xc - xStart,y,&ch,1);
      xc += a->fm.width(&ch,1);
//if (zc < len) printf("%c",ch);
    }
    zc++;
  }

//printf("\n");
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

//bool KWriteDoc::isModified() {
//  return modified;
//}

bool KWriteDoc::isLastView(int numViews) {
  return ((int) views.count() == numViews);
}

void KWriteDoc::setFileName(const char *s) {
  int z;

  fName = s;
  for (z = 0; z < (int) views.count(); z++) {
    emit views.at(z)->kWrite->newCaption();
  }

  //highlight detection
  if (fName.isEmpty()) return;
  int pos = fName.findRev('/') + 1;
  setHighlight(hlManager->highlightFind(this));
  updateViews();
}

bool KWriteDoc::hasFileName() {
  return !fName.isEmpty();
}

const char *KWriteDoc::fileName() {
  return fName;
}

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
          newPos = textLine->findUnSelected(pos);
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
          newPos = textLine->findUnSelected(pos);
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
}

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
  
//  if (selectEnd >= (int) contents.count()) selectEnd = (int) contents.count() -1;
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
//  printf("selectStart %d, selectEnd %d\n",selectStart,selectEnd);
  if (selectStart > selectEnd) {
    selectStart = 0xffffff;
    selectEnd = 0;
  }
}

void KWriteDoc::doAction(KWAction *a) {

  switch (a->action) {
    case KWAction::replace:
      delete doReplace(a);
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

const char *KWriteDoc::doReplace(KWAction *a) {
  const char *text;
  int textLen;
  int len, pos;
  TextLine *textLine;
  int l;

  pos = a->cursor.x;    //position where to replace
  len = a->len;         //length of old text
  text = a->text;       //new text
  textLen = a->textLen; //length of new text

  textLine = contents.at(a->cursor.y);
  l = textLine->length() - pos;
  if (l > len) l = len;
  a->setData(textLen,&textLine->getText()[pos],l); //save old text

  if (textLen > len) { //new text longer than old text
    textLine->move(pos + len - (len > 0),textLen - len); //move right
  } else {
    textLine->del(pos + textLen,len - textLen);
  }
  textLine->overwrite(pos,text,textLen);

  tagLine(a->cursor.y);

  return text;
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
  textLine->setLength(a->len);
  textLine->unWrap(contents.next(),a->cursor.x);

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
  textLine->setLength(a->cursor.x);
  textLine->unWrap(nextLine,nextLine->length());
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
  if (state != undoState) {
    undoState = state;
    for (view = views.first(); view != 0L; view = views.next()) {
      emit view->kWrite->newUndo();
    }
  }
}

void KWriteDoc::recordStart(PointStruc &cursor, bool keepModal) {
  KWActionGroup *g;

  if (!keepModal) setPseudoModal(0L);
  while ((int) undoList.count() > currentUndo) undoList.removeLast();
  while ((int) undoList.count() > undoSteps) {
    undoList.removeFirst();
    currentUndo--;
  }
  g = new KWActionGroup(cursor);
  undoList.append(g);
  currentUndo++;

  unmarkFound();
  tagEnd = 0;
  tagStart = 0xffffff;
}

void KWriteDoc::recordAction(KWAction::Action action, PointStruc &cursor) {
  KWAction *a;

  a = new KWAction(action,cursor);
  doAction(a);
  undoList.getLast()->insertAction(a);
}

void KWriteDoc::recordReplace(PointStruc &cursor,
      int len, const char *text, int textLen) {
  KWAction *a;

  a = new KWAction(KWAction::replace,cursor);
  a->len = len;
  a->text = text;
  a->textLen = textLen;
  doReplace(a);
  undoList.getLast()->insertAction(a);
}

void KWriteDoc::recordEnd(KWriteView *view, VConfig &c) {
  recordEnd(view,c.cursor,c.flags);
}

void KWriteDoc::recordEnd(KWriteView *view, PointStruc &cursor, int flags) {

  if (!(flags & cfPersistent)) deselectAll();

  undoList.getLast()->end = cursor;
  view->updateCursor(cursor);

  optimizeSelection();
  if (tagStart <= tagEnd) updateLines(tagStart,tagEnd,flags);
  setModified(true);
  newUndo();
}

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


void KWriteDoc::doActionGroup(KWActionGroup *g, int flags) {
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
  setModified(true);
  newUndo();
}

void KWriteDoc::undo(KWriteView *view, int flags) {
  KWActionGroup *g;

  if (currentUndo <= 0) return;
  currentUndo--;
  g = undoList.at(currentUndo);
  doActionGroup(g,flags);
  view->updateCursor(g->start);
}

void KWriteDoc::redo(KWriteView *view, int flags) {
  KWActionGroup *g;

//  if (currentUndo >= (int) undoList.count()) return;
  g = undoList.at(currentUndo);
  if (!g) return;
  currentUndo++;
  doActionGroup(g,flags);
  view->updateCursor(g->end);
}

void KWriteDoc::setUndoSteps(int steps) {
  if (steps < 5) steps = 5;
  undoSteps = steps;
}

void KWriteDoc::setPseudoModal(QWidget *w) {
  delete pseudoModal;
  pseudoModal = w;
}

void KWriteDoc::indent(KWriteView *view, VConfig &c) {
  TextLine *textLine;

  recordStart(c.cursor);
  c.cursor.x = 0;
  if (selectEnd < selectStart) {
    //indent single line
    textLine = contents.at(c.cursor.y);
    recordReplace(c.cursor,0," ",1);
  } else {
    //indent selection
    for (c.cursor.y = selectStart; c.cursor.y <= selectEnd; c.cursor.y++) {
      textLine = contents.at(c.cursor.y);
      if (textLine->isSelected() || textLine->numSelected()) recordReplace(c.cursor,0," ",1);
    }
    c.cursor.y--;
  }
  recordEnd(view,c);
}

void KWriteDoc::unIndent(KWriteView *view, VConfig &c) {
  char s[16];
  PointStruc cursor;
  TextLine *textLine;
  int l;
  bool started;

  memset(s,' ',16);
  cursor = c.cursor;
  c.cursor.x = 0;
  if (selectEnd < selectStart) {
    //unindent single line
    textLine = contents.at(c.cursor.y);
    if (textLine->firstChar() == 0) return;
    recordStart(cursor);
    l = (textLine->getChar(0) == '\t') ? tabChars - 1 : 0;
    recordReplace(c.cursor,1,s,l);
  } else {
    //unindent selection
/*    for (c.cursor.y = selectStart; c.cursor.y <= selectEnd; c.cursor.y++) {
      textLine = contents.at(c.cursor.y);
      if ((textLine->isSelected() || textLine->numSelected())
        && textLine->firstChar() == 0) return;
    }*/
    started = false;
    for (c.cursor.y = selectStart; c.cursor.y <= selectEnd; c.cursor.y++) {
      textLine = contents.at(c.cursor.y);
      if ((textLine->isSelected() || textLine->numSelected())
        && textLine->firstChar() > 0) {
        if (!started) {
          recordStart(cursor);
          started = true;
        }
        l = (textLine->getChar(0) == '\t') ? tabChars - 1 : 0;
        recordReplace(c.cursor,1,s,l);
      }
    }
    c.cursor.y--;
  }
  recordEnd(view,c);
}
