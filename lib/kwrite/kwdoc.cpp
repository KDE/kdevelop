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

#include <sys/time.h>
#include <unistd.h>

#include <stdio.h>

#include <qobject.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qfont.h>
#include <qpainter.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtextcodec.h>

#include <klocale.h>
#include <kcharsets.h>
#include <kdebug.h>

#include <kglobalsettings.h>
#include <kaction.h>
#include <kstdaction.h>

#include "kwrite_factory.h"
#include "kwview.h"
#include "kwbuffer.h"
#include "kwtextline.h"
#include "kwattribute.h"
#include "kwdoc.h"
#include "kwdoc.moc"

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

const char * KWActionGroup::typeName(int type)
{
  // return a short text description of the given undo group type suitable for a menu
  // not the lack of i18n's, the caller is expected to handle translation
  switch (type) {
  case ugPaste : return "Paste Text";
  case ugDelBlock : return "Selection Overwrite";
  case ugIndent : return "Indent";
  case ugUnindent : return "Unindent";
  case ugComment : return "Comment";
  case ugUncomment : return "Uncomment";
  case ugReplace : return "Text Replace";
  case ugSpell : return "Spell Check";
  case ugInsChar : return "Typing";
  case ugDelChar : return "Delete Text";
  case ugInsLine : return "New Line";
  case ugDelLine : return "Delete Line";
  }
  return "";
}

const int KWriteDoc::maxAttribs = 32;

KWriteDoc::KWriteDoc(HlManager *hlManager, const QString &path,
                     bool bSingleViewMode, bool bBrowserView,
                     QWidget *parentWidget, const char *widgetName,
                     QObject *parent, const char *name )
  : KTextEditor::Document(parent, name), hlManager(hlManager) {

  setInstance( KWriteFactory::instance() );

  m_url.setPath( path );

  m_bSingleViewMode = bSingleViewMode;

  buffer = new KWBuffer;
  connect(buffer, SIGNAL(linesChanged(int)), this, SLOT(slotBufferChanged()));

  colors[0] = KGlobalSettings::baseColor();
  colors[1] = KGlobalSettings::highlightColor();
  colors[2] = KGlobalSettings::textColor();
  colors[3] = KGlobalSettings::textColor();
  colors[4] = KGlobalSettings::baseColor();

  m_attribs = new Attribute[maxAttribs];

  m_highlight = 0L;
  tabChars = 8;

  m_singleSelection = false;

  newDocGeometry = false;
  readOnly = false;
  newDoc = false;

  modified = false;

  undoList.setAutoDelete(true);
  undoState = 0;
  undoSteps = 50;

//  recordReset();

  pseudoModal = 0L;
  clear();
  clearFileName();

  setHighlight(0); //calls updateFontData()
  // if the user changes the highlight with the dialog, notify the doc
  connect(hlManager,SIGNAL(changed()),SLOT(hlChanged()));

  newDocGeometry = false;

  if ( m_bSingleViewMode )
  {
    KTextEditor::View *view = createView( parentWidget, widgetName );
    view->show();
    setWidget( view );

    if ( bBrowserView )
    {
      // We are embedded in konqueror, let's provide an XML file and actions.
      (void)new KWriteBrowserExtension( this );
      setXMLFile( "kwrite_browser.rc" );

      KStdAction::selectAll( view, SLOT( selectAll() ), actionCollection(), "select_all" );
      (void)new KAction( i18n( "Unselect all" ), 0, view, SLOT( deselectAll() ), actionCollection(), "unselect_all" );
      //(void)new KAction( i18n( "Invert selection" ), 0, view, SLOT( invertSelection() ), actionCollection(), "invert_select" );
      KStdAction::find( view, SLOT( find() ), actionCollection(), "find" );
      KStdAction::findNext( view, SLOT( findAgain() ), actionCollection(), "find_again" );
      KStdAction::gotoLine( view, SLOT( gotoLine() ), actionCollection(), "goto_line" );

      // TODO highlight select submenu
    }
  }
}

KWriteDoc::~KWriteDoc() {
  m_highlight->release();
  kdDebug() << "KWriteDoc::~KWriteDoc" << endl;

  if ( !m_bSingleViewMode )
  {
    m_views.setAutoDelete( true );
    m_views.clear();
    m_views.setAutoDelete( false );
  }
}

bool KWriteDoc::openFile()
{
#ifdef NEW_CODE
  // TODO: Pass codec around.
  loadFile( m_file, QTextCodec::codecForLocale());
#else
  QFile f( m_file );
  if ( !f.open( IO_ReadOnly ) )
    return false;

  loadFile( f );
#endif
//  if ( updateHighlight )
  {
    //highlight detection
    //  pos = fName.findRev('/') +1;
    //  if (pos >= (int) fName.length()) return; //no filename
    //  hl = hlManager->wildcardFind(s.right( s.length() - pos ));
    QString fn = m_url.fileName();
    if ( fn.isEmpty() )
        return false;

    int hl = hlManager->wildcardFind( fn );

#ifndef NEW_CODE
    if (hl == -1)
    {
      // fill the detection buffer with the contents of the text
      const int HOWMANY = 1024;
      QByteArray buf(HOWMANY);
      int bufpos = 0, len;
      for (TextLine::List::ConstIterator it = contents.begin();
           it != contents.end();
           ++it)
      {
        TextLine::Ptr textLine = *it;
        len = textLine->length();
        if (bufpos + len > HOWMANY)
          len = HOWMANY - bufpos;
        memcpy(&buf[bufpos], textLine->getText(), len);
        bufpos += len;
        if (bufpos >= HOWMANY)
          break;
      }
      //    hl = hlManager->mimeFind(buf, s.right( s.length() - pos));
      hl = hlManager->mimeFind( buf, fn );
    }
#endif
    setHighlight(hl);
  }

  updateLines();
  updateViews();
  //  setFileName( m_url.path() );

#ifndef NEW_CODE
  f.close();
#endif
  return true;
}

bool KWriteDoc::saveFile()
{
#ifdef NEW_CODE
  return writeFile( m_file, QTextCodec::codecForLocale());
#else
  QFile f( m_file );
  if ( !f.open( IO_WriteOnly | IO_Truncate ) )
    return false;

  writeFile( f );

  f.close();
  return true;
#endif
}

KTextEditor::View *KWriteDoc::createView( QWidget *parent, const char *name )
{
  return new KWrite( this, parent, name, true, false );
}

QString KWriteDoc::textLine( int line ) const
{
  QString res;

  TextLine::Ptr l = getTextLine( line );
  if ( !l )
    return res;

  return QConstString( l->getText(), l->length() ).string();
}

void KWriteDoc::insertLine( const QString &, int )
{
}

void KWriteDoc::insertAt( const QString &s, int line, int col, bool  )
{
  VConfig c;
  c.view = 0; // ### FIXME
  c.cursor.x = col;
  c.cursor.y = line;
  c.cXPos = 0; // ### FIXME
  c.flags = 0; // ### FIXME
  c.wrapAt = 80; // ### FIXME
  insert( c, s );
}

void KWriteDoc::removeLine( int  )
{
}

int KWriteDoc::length() const
{
  return text().length();
}

void KWriteDoc::setSelection( int , int , int , int )
{
}

bool KWriteDoc::hasSelection() const
{
  return false;
}

QString KWriteDoc::selection() const
{
  return QString::null;
}

int KWriteDoc::numLines() const
{
#ifdef NEW_CODE
  return buffer->count();
#else
  return (int) contents.count();
#endif
}


TextLine::Ptr KWriteDoc::getTextLine(int line) const {
#ifdef NEW_CODE
  // This is a hack to get this stuff working.
  return buffer->line(line);
#else
  if (line >= numLines())
    return 0L;

  return contents[line];
#endif
}

int KWriteDoc::textLength(int line) {
  TextLine::Ptr textLine = getTextLine(line);
  if (!textLine) return 0;
  return textLine->length();
}

void KWriteDoc::setTabWidth(int chars) {
  if (tabChars == chars) return;
  if (chars < 1) chars = 1;
  if (chars > 16) chars = 16;
  tabChars = chars;
  updateFontData();

  maxLength = -1;
  for (TextLine::List::ConstIterator it = contents.begin();
       it != contents.end();
       ++it)
  {
    TextLine::Ptr textLine = *it;
    int len = textWidth(textLine,textLine->length());
    if (len > maxLength) {
      maxLength = len;
      longestLine = textLine;
    }
  }
//  tagAll();
}

void KWriteDoc::setReadWrite( bool rw )
{
  setReadOnly( !rw );
  KTextEditor::Document::setReadWrite( rw );
}

bool KWriteDoc::isReadWrite() const
{
  return !isReadOnly();
}

void KWriteDoc::setReadOnly(bool m) {
  KTextEditor::View *view;

  if (m != readOnly) {
    readOnly = m;
//    if (readOnly) recordReset();
    for (view = m_views.first(); view != 0L; view = m_views.next() ) {
      emit static_cast<KWrite *>( view )->newStatus();
    }
  }
}

bool KWriteDoc::isReadOnly() const {
  return readOnly;
}

void KWriteDoc::setNewDoc( bool m )
{
//  KTextEditor::View *view;

  if ( m != newDoc )
  {
    newDoc = m;
////    if (readOnly) recordReset();
//    for (view = m_views.first(); view != 0L; view = m_views.next() ) {
//      emit static_cast<KWrite *>( view )->newStatus();
//    }
  }
}

bool KWriteDoc::isNewDoc() const {
  return newDoc;
}

void KWriteDoc::setModified(bool m) {
  KTextEditor::View *view;

  if (m != modified) {
    modified = m;
    for (view = m_views.first(); view != 0L; view = m_views.next() ) {
      emit static_cast<KWrite *>( view )->newStatus();
    }
    emit modifiedChanged ();
  }
}

bool KWriteDoc::isModified() const {
  return modified;
}

void KWriteDoc::readConfig(KConfig *config) {
  int z;
  char s[16];

  setTabWidth(config->readNumEntry("TabWidth", 8));
  setUndoSteps(config->readNumEntry("UndoSteps", 50));
  m_singleSelection = config->readBoolEntry("SingleSelection", false);
  for (z = 0; z < 5; z++) {
    sprintf(s, "Color%d", z);
    colors[z] = config->readColorEntry(s, &colors[z]);
  }
}

void KWriteDoc::writeConfig(KConfig *config) {
  int z;
  char s[16];

  config->writeEntry("TabWidth", tabChars);
  config->writeEntry("UndoSteps", undoSteps);
  config->writeEntry("SingleSelection", m_singleSelection);
  for (z = 0; z < 5; z++) {
    sprintf(s, "Color%d", z);
    config->writeEntry(s, colors[z]);
  }
}

void KWriteDoc::readSessionConfig(KConfig *config) {
  readConfig(config);
  m_url = config->readEntry("URL"); // ### doesn't this break the encoding? (Simon)
  setHighlight(hlManager->nameFind(config->readEntry("Highlight")));
}

void KWriteDoc::writeSessionConfig(KConfig *config) {

  writeConfig(config);
  config->writeEntry("URL", m_url.url() ); // ### encoding?? (Simon)
  config->writeEntry("Highlight", m_highlight->name());
}


void KWriteDoc::setHighlight(int n) {
  Highlight *h;

//  hlNumber = n;

  h = hlManager->getHl(n);
  if (h == m_highlight) {
    updateLines();
  } else {
    if (m_highlight != 0L) m_highlight->release();
    h->use();
    m_highlight = h;
    makeAttribs();
  }
  emit(highlightChanged());
}

void KWriteDoc::makeAttribs() {

  m_numAttribs = hlManager->makeAttribs(m_highlight, m_attribs, maxAttribs);
  updateFontData();
  updateLines();
}

void KWriteDoc::updateFontData() {
  int maxAscent, maxDescent;
  int minTabWidth, maxTabWidth;
  int i, z;
  KWrite *view;

  maxAscent = 0;
  maxDescent = 0;
  minTabWidth = 0xffffff;
  maxTabWidth = 0;

  for (z = 0; z < m_numAttribs; z++) {
    i = m_attribs[z].fm.ascent();
    if (i > maxAscent) maxAscent = i;
    i = m_attribs[z].fm.descent();
    if (i > maxDescent) maxDescent = i;
    i = m_attribs[z].fm.width('x');
    if (i < minTabWidth) minTabWidth = i;
    if (i > maxTabWidth) maxTabWidth = i;
  }

  fontHeight = maxAscent + maxDescent + 1;
  fontAscent = maxAscent;
  m_tabWidth = tabChars*(maxTabWidth + minTabWidth)/2;

  for (view = views.first(); view != 0L; view = views.next() ) {
    resizeBuffer(view,view->width(),fontHeight);
    view->tagAll();
    view->updateCursor();
  }
}

void KWriteDoc::hlChanged() { //slot
  makeAttribs();
  updateViews();
}


void KWriteDoc::addView(KTextEditor::View *view) {
  views.append( static_cast<KWrite *>( view ) );
  KTextEditor::Document::addView( view );
  connect( static_cast<KWrite *>( view ), SIGNAL( destroyed() ), this, SLOT( slotViewDestroyed() ) );
}

void KWriteDoc::removeView(KTextEditor::View *view) {
//  if (undoView == view) recordReset();
  disconnect( static_cast<KWrite *>( view ), SIGNAL( destroyed() ), this, SLOT( slotViewDestroyed() ) );
  views.removeRef( static_cast<KWrite *>( view ) );
  KTextEditor::Document::removeView( view );
}

void KWriteDoc::slotViewDestroyed()
{
  views.removeRef( static_cast<const KWrite *>( sender() ) );
}

bool KWriteDoc::ownedView(KWrite *view) {
  // do we own the given view?
  return (views.containsRef(view) > 0);
}

bool KWriteDoc::isLastView(int numViews) {
  return ((int) views.count() == numViews);
}

int KWriteDoc::textWidth(const TextLine::Ptr &textLine, int cursorX) {
  int x;
  int z;
  QChar ch;
  Attribute *a;

  x = 0;
  for (z = 0; z < cursorX; z++) {
    ch = textLine->getChar(z);
    a = &m_attribs[textLine->getAttr(z)];
    x += (ch == '\t') ? m_tabWidth - (x % m_tabWidth) : a->fm.width(ch);//a->width(ch);
  }
  return x;
}

int KWriteDoc::textWidth(PointStruc &cursor) {
  if (cursor.x < 0)
     cursor.x = 0;
  if (cursor.y < 0)
     cursor.y = 0;
  if (cursor.y >= numLines())
     cursor.y = lastLine();
  return textWidth(getTextLine(cursor.y),cursor.x);
}

int KWriteDoc::textWidth(bool wrapCursor, PointStruc &cursor, int xPos) {
  int len;
  int x, oldX;
  int z;
  QChar ch;
  Attribute *a;

  if (cursor.y < 0) cursor.y = 0;
  if (cursor.y > lastLine()) cursor.y = lastLine();
  TextLine::Ptr textLine = getTextLine(cursor.y);
  len = textLine->length();

  x = oldX = z = 0;
  while (x < xPos && (!wrapCursor || z < len)) {
    oldX = x;
    ch = textLine->getChar(z);
    a = &m_attribs[textLine->getAttr(z)];
    x += (ch == '\t') ? m_tabWidth - (x % m_tabWidth) : a->fm.width(ch);//a->width(ch);
    z++;
  }
  if (xPos - oldX < x - xPos && z > 0) {
    z--;
    x = oldX;
  }
  cursor.x = z;
  return x;
}


int KWriteDoc::textPos(const TextLine::Ptr &textLine, int xPos) {
/*  int newXPos;

  return textPos(textLine, xPos, newXPos);
}

int KWriteDoc::textPos(const TextLine::Ptr &textLine, int xPos, int &newXPos) {
*/
//  int len;
  int x, oldX;
  int z;
  QChar ch;
  Attribute *a;

//  len = textLine->length();

  x = oldX = z = 0;
  while (x < xPos) { // && z < len) {
    oldX = x;
    ch = textLine->getChar(z);
    a = &m_attribs[textLine->getAttr(z)];
    x += (ch == '\t') ? m_tabWidth - (x % m_tabWidth) : a->fm.width(ch);//a->width(ch);
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
  return numLines()*fontHeight;
}

void KWriteDoc::insert(VConfig &c, const QString &s) {
  int pos;
  QChar ch;
  QString buf;

  if (s.isEmpty()) return;

  recordStart(c, KWActionGroup::ugPaste);

  pos = 0;
  if (!(c.flags & KWrite::cfVerticalSelect)) {
    do {
      ch = s[pos];
      if (ch.isPrint() || ch == '\t') {
        buf += ch; // append char to buffer
      } else if (ch == '\n') {
        recordAction(KWAction::newLine, c.cursor); // wrap contents behind cursor to new line
        recordInsert(c, buf); // append to old line
//        c.cursor.x += buf.length();
        buf.truncate(0); // clear buffer
        c.cursor.y++;
        c.cursor.x = 0;
      }
      pos++;
    } while (pos < (int) s.length());
  } else {
    int xPos;

    xPos = textWidth(c.cursor);
    do {
      ch = s[pos];
      if (ch.isPrint() || ch == '\t') {
        buf += ch;
      } else if (ch == '\n') {
        recordInsert(c, buf);
        c.cursor.x += buf.length();
        buf.truncate(0);
        c.cursor.y++;
        if (c.cursor.y >= numLines())
          recordAction(KWAction::insLine, c.cursor);
        c.cursor.x = textPos(getTextLine(c.cursor.y), xPos);
      }
      pos++;
    } while (pos < (int) s.length());
  }
  recordInsert(c, buf);
  c.cursor.x += buf.length();
  recordEnd(c);
}

void KWriteDoc::insertFile(VConfig &c, QIODevice &dev)
{
  recordStart(c, KWActionGroup::ugPaste);

  QString buf;
  QChar ch, last;

  QTextStream stream( &dev );
  while ( !stream.eof() ) {
    stream >> ch;

    if (ch.isPrint() || ch == '\t') {
        buf += ch;
    } else if (ch == '\n' || ch == '\r') {
        if (last != '\r' || ch != '\n') {
          recordAction(KWAction::newLine, c.cursor);
          recordInsert(c, buf);
          buf.truncate(0);
          c.cursor.y++;
          c.cursor.x = 0;
        }
        last = ch;
    }
  }

  recordInsert(c, buf);
  recordEnd(c);
}

#ifdef NEW_CODE
void KWriteDoc::loadFile(const QString &file, QTextCodec *codec)
{
  buffer->insertFile(0, file, codec);
qWarning("Linecount = %d (loadFile)", buffer->count());
}

void KWriteDoc::appendData(const QByteArray &data, QTextCodec *codec)
{
  buffer->insertData(buffer->count(), data, codec);
qWarning("Linecount = %d (appendData)", buffer->count());
  slotBufferChanged();
}

bool KWriteDoc::writeFile(const QString &file, QTextCodec *codec)
{
qWarning("writeFile()");
  QFile f( file );
  if ( !f.open( IO_WriteOnly ) )
    return false; // Error

  QTextStream stream(&f);
  stream.setCodec(codec);
  int maxLine = numLines();
  int line = 0;
  while(true)
  {
    TextLine::Ptr textLine = getTextLine(line);
    QConstString str((QChar *) textLine->getText(), textLine->length());
    stream << str.string();
    line++;
    if (line >= maxLine) break;
    if (eolMode != KWrite::eolUnix) stream << QChar('\r');
    if (eolMode != KWrite::eolMacintosh) stream << QChar('\n');
  };
  f.close();
  return (f.status() == IO_Ok);
}
#else
void KWriteDoc::loadFile(QIODevice &dev) {
  QChar ch;
  char last = '\0';
  char s;

  clear();

  TextLine::Ptr textLine = contents.first();
  QTextStream stream( &dev );
  while ( !stream.eof() ) {
      stream >> ch;
      s = ch.latin1();
      if (ch.isPrint() || s == '\t') {
        textLine->append(&ch, 1);
      } else if (s == '\n' || s == '\r') {
        if (last != '\r' || s != '\n') {
          textLine = new TextLine();
          contents.append(textLine);
          if (s == '\r') eolMode = KWrite::eolMacintosh;
        } else eolMode = KWrite::eolDos;
        last = s;
      }
  }
//  updateLines();
}

void KWriteDoc::writeFile(QIODevice &dev) {
  TextLine::List::ConstIterator it = contents.begin();
  QTextStream stream(&dev);
  do {
    TextLine::Ptr textLine = *it;
    QConstString str((QChar *) textLine->getText(), textLine->length());
    stream << str.string();
    ++it;
    if (it == contents.end()) break;
    if (eolMode != KWrite::eolUnix) dev.putch('\r');
    if (eolMode != KWrite::eolMacintosh) dev.putch('\n');
  } while (true);
}
#endif

int KWriteDoc::currentColumn(PointStruc &cursor) {
  return getTextLine(cursor.y)->cursorX(cursor.x,tabChars);
}

bool KWriteDoc::insertChars(VConfig &c, const QString &chars) {
  int z, pos, l;
  bool onlySpaces;
  QChar ch;
  QString buf;

  TextLine::Ptr textLine = getTextLine(c.cursor.y);

  pos = 0;
  onlySpaces = true;
  for (z = 0; z < (int) chars.length(); z++) {
    ch = chars[z];
    if (ch == '\t' && c.flags & KWrite::cfReplaceTabs) {
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
      if (c.flags & KWrite::cfAutoBrackets) {
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
  if (c.flags &KWrite:: cfDelOnInput) delMarkedText(c);

/*  //do nothing if spaces will be removed
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
  }*/

  recordStart(c, KWActionGroup::ugInsChar);
  recordReplace(c/*.cursor*/, (c.flags & KWrite::cfOvr) ? buf.length() : 0, buf);
  c.cursor.x += pos;

  if (c.flags & KWrite::cfWordWrap && c.wrapAt > 0) {
    int line;
    const QChar *s;
//    int pos;
    PointStruc actionCursor;

    line = c.cursor.y;
    do {
      textLine = getTextLine(line);
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

      if (line == lastLine() || (getTextLine(line+1)->length() == 0) ) {
        //at end of doc: create new line
        actionCursor.x = pos;
        actionCursor.y = line;
        recordAction(KWAction::newLine,actionCursor);
      } else {
        //wrap
        actionCursor.y = line + 1;
        if (!s[l - 1].isSpace()) { //add space in next line if necessary
          actionCursor.x = 0;
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

QString tabString(int pos, int tabChars) {
  QString s;
  while (pos >= tabChars) {
    s += '\t';
    pos -= tabChars;
  }
  while (pos > 0) {
    s += ' ';
    pos--;
  }
  return s;
}

void KWriteDoc::newLine(VConfig &c) {

  //auto deletion of marked text is done by the view to have a more
  // "low level" KWriteDoc::newLine method
  recordStart(c, KWActionGroup::ugInsLine);

  if (!(c.flags & KWrite::cfAutoIndent)) {
    recordAction(KWAction::newLine,c.cursor);
    c.cursor.y++;
    c.cursor.x = 0;
  } else {
    TextLine::Ptr textLine = getTextLine(c.cursor.y);
    int pos = textLine->firstChar();
    if (c.cursor.x < pos) c.cursor.x = pos; // place cursor on first char if before

    int y = c.cursor.y;
    while ((y > 0) && (pos < 0)) { // search a not empty text line
      textLine = getTextLine(--y);
      pos = textLine->firstChar();
    }
    recordAction(KWAction::newLine, c.cursor);
    c.cursor.y++;
    c.cursor.x = 0;
    if (pos > 0) {
      pos = textLine->cursorX(pos, tabChars);
      if (getTextLine(c.cursor.y)->length() > 0) {
        QString s = tabString(pos, (c.flags & KWrite::cfSpaceIndent) ? 0xffffff : tabChars);
        recordInsert(c.cursor, s);
        pos = s.length();
      }
//      recordInsert(c.cursor, QString(textLine->getText(), pos));
      c.cursor.x = pos;
    }
  }

  recordEnd(c);
}

void KWriteDoc::killLine(VConfig &c) {

  recordStart(c, KWActionGroup::ugDelLine);
  c.cursor.x = 0;
  recordDelete(c.cursor, 0xffffff);
  if (c.cursor.y < lastLine()) {
    recordAction(KWAction::killLine, c.cursor);
  }
  recordEnd(c);
}

void KWriteDoc::backspace(VConfig &c) {

  if (c.cursor.x <= 0 && c.cursor.y <= 0) return;

  if (c.cursor.x > 0) {
    recordStart(c, KWActionGroup::ugDelChar);
    if (!(c.flags & KWrite::cfBackspaceIndents)) {
      // ordinary backspace
      c.cursor.x--;
      recordDelete(c.cursor, 1);
    } else {
      // backspace indents: erase to next indent position
      int l = 1; // del one char

      TextLine::Ptr textLine = getTextLine(c.cursor.y);
      int pos = textLine->firstChar();
      if (pos < 0 || pos >= c.cursor.x) {
        // only spaces on left side of cursor
        // search a line with less spaces
        int y = c.cursor.y;
        while (y > 0) {
          textLine = getTextLine(--y);
          pos = textLine->firstChar();
          if (pos >= 0 && pos < c.cursor.x) {
            l = c.cursor.x - pos; // del more chars
            break;
          }
        }
      }
      // break effectively jumps here
      c.cursor.x -= l;
      recordDelete(c.cursor, l);
    }
  } else {
    // c.cursor.x == 0: wrap to previous line
    recordStart(c, KWActionGroup::ugDelLine);
    c.cursor.y--;
    c.cursor.x = getTextLine(c.cursor.y)->length();
    recordAction(KWAction::delLine,c.cursor);
  }
  recordEnd(c);
}


void KWriteDoc::del(VConfig &c) {
  TextLine::Ptr textLine = getTextLine(c.cursor.y);
  int len =  (c.flags & KWrite::cfRemoveSpaces) ? textLine->lastChar() : textLine->length();
  if (c.cursor.x < len/*getTextLine(c.cursor.y)->length()*/) {
    // delete one character
    recordStart(c, KWActionGroup::ugDelChar);
    recordDelete(c.cursor, 1);
    recordEnd(c);
  } else {
    if (c.cursor.y < lastLine()) {
      // wrap next line to this line
      textLine->truncate(c.cursor.x); // truncate spaces
      recordStart(c, KWActionGroup::ugDelLine);
      recordAction(KWAction::delLine,c.cursor);
      recordEnd(c);
    }
  }
}

void KWriteDoc::clear() {
  PointStruc cursor;
  KWrite *view;

  setPseudoModal(0L);
  cursor.x = cursor.y = 0;
  for (view = views.first(); view != 0L; view = views.next() ) {
    view->updateCursor(cursor);
    view->tagAll();
  }

  eolMode = KWrite::eolUnix;


#ifndef NEW_CODE
  contents.clear();
  contents.append(longestLine = new TextLine());
#else
  buffer->clear();
  longestLine = buffer->line(0);
#endif

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

void KWriteDoc::cut(VConfig &c) {

  if (selectEnd < selectStart) return;

//  unmarkFound();
  copy(c.flags);
  delMarkedText(c);
}

void KWriteDoc::copy(int flags) {

  if (selectEnd < selectStart) return;

  QString s = markedText(flags);
  if (!s.isEmpty()) {
//#if defined(_WS_X11_)
    if (m_singleSelection)
      disconnect(QApplication::clipboard(), SIGNAL(dataChanged()), this, 0);
//#endif
    QApplication::clipboard()->setText(s);
//#if defined(_WS_X11_)
    if (m_singleSelection) {
      connect(QApplication::clipboard(), SIGNAL(dataChanged()),
        this, SLOT(clipboardChanged()));
    }
//#endif
  }
}

void KWriteDoc::paste(VConfig &c) {
  QString s = QApplication::clipboard()->text();
  if (!s.isEmpty()) {
//    unmarkFound();
    insert(c, s);
  }
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
    QChar ch;
    Attribute *a;

    TextLine::Ptr textLine = getTextLine(line);

    //--- speed optimization
    //s = textPos(textLine, x1, newX1);
    x = oldX = z = 0;
    while (x < x1) { // && z < len) {
      oldX = x;
      ch = textLine->getChar(z);
      a = &m_attribs[textLine->getAttr(z)];
      x += (ch == '\t') ? m_tabWidth - (x % m_tabWidth) : a->fm.width(ch);//a->width(ch);
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
      a = &m_attribs[textLine->getAttr(z)];
      x += (ch == '\t') ? m_tabWidth - (x % m_tabWidth) : a->fm.width(ch);//a->width(ch);
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
    emit selectionChanged();
  }
}

void KWriteDoc::selectTo(VConfig &c, PointStruc &cursor, int cXPos) {
  //c.cursor = old cursor position
  //cursor = new cursor position

  if (c.cursor.x != select.x || c.cursor.y != select.y) {
    //new selection

    if (!(c.flags & KWrite::cfKeepSelection)) deselectAll();
//      else recordReset();

    anchor = c.cursor;
    aXPos = c.cXPos;
  }

  if (!(c.flags & KWrite::cfVerticalSelect)) {
    //horizontal selections
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

    TextLine::Ptr textLine = getTextLine(y);

    if (c.flags & KWrite::cfXorSelect) {
      //xor selection with old selection
      while (y < ey) {
        textLine->toggleSelectEol(x);
        x = 0;
        y++;
        textLine = getTextLine(y);
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
            textLine = getTextLine(y);
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
        textLine = getTextLine(y);
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
  emit selectionChanged();
}


void KWriteDoc::selectAll() {
  int z;
  TextLine::Ptr textLine;

  select.x = -1;

  unmarkFound();
//  if (selectStart != 0 || selectEnd != lastLine()) recordReset();

  selectStart = 0;
  selectEnd = lastLine();

  tagLines(selectStart,selectEnd);

  for (z = selectStart; z < selectEnd; z++) {
    textLine = getTextLine(z);
    textLine->selectEol(true,0);
  }
  textLine = getTextLine(z);
  textLine->select(true,0,textLine->length());
  emit selectionChanged();
}

void KWriteDoc::deselectAll() {
  select.x = -1;
  if (selectEnd < selectStart) return;

  unmarkFound();
//  recordReset();

  tagLines(selectStart,selectEnd);

  for (int z = selectStart; z <= selectEnd; z++) {
    TextLine::Ptr textLine = getTextLine(z);
    textLine->selectEol(false,0);
  }
  selectStart = 0xffffff;
  selectEnd = 0;
  emit selectionChanged();
}

void KWriteDoc::invertSelection() {
  TextLine::Ptr textLine;

  select.x = -1;

  unmarkFound();
//  if (selectStart != 0 || selectEnd != lastLine()) recordReset();

  selectStart = 0;
  selectEnd = lastLine();

  tagLines(selectStart,selectEnd);

  for (int z = selectStart; z < selectEnd; z++) {
    textLine = getTextLine(z);
    textLine->toggleSelectEol(0);
  }
  textLine = getTextLine(selectEnd);
  textLine->toggleSelect(0,textLine->length());
  optimizeSelection();
  emit selectionChanged();
}

void KWriteDoc::selectWord(PointStruc &cursor, int flags) {
  int start, end, len;

  TextLine::Ptr textLine = getTextLine(cursor.y);
  len = textLine->length();
  start = end = cursor.x;
  while (start > 0 && m_highlight->isInWord(textLine->getChar(start - 1))) start--;
  while (end < len && m_highlight->isInWord(textLine->getChar(end))) end++;
  if (end <= start) return;
  if (!(flags & KWrite::cfKeepSelection)) deselectAll();
//    else recordReset();

  textLine->select(true, start, end);

  anchor.x = start;
  select.x = end;
  anchor.y = select.y = cursor.y;
  tagLines(cursor.y, cursor.y);
  if (cursor.y < selectStart) selectStart = cursor.y;
  if (cursor.y > selectEnd) selectEnd = cursor.y;
  emit selectionChanged();
}

void KWriteDoc::doIndent(VConfig &c, int change) {

  c.cursor.x = 0;

  recordStart(c, (change < 0) ? KWActionGroup::ugUnindent
    : KWActionGroup::ugIndent);

  if (selectEnd < selectStart) {
    // single line
    optimizeLeadingSpace(c.cursor.y, c.flags, change);
  } else {
    // entire selection
    TextLine::Ptr textLine;
    int line, z;
    QChar ch;

    if (c.flags & KWrite::cfKeepIndentProfile && change < 0) {
      // unindent so that the existing indent profile doesn´t get screwed
      // if any line we may unindent is already full left, don't do anything
      for (line = selectStart; line <= selectEnd; line++) {
        textLine = getTextLine(line);
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
      textLine = getTextLine(line);
      if (textLine->isSelected() || textLine->numSelected()) {
        optimizeLeadingSpace(line, c.flags, change);
      }
    }
  }
  // recordEnd now removes empty undo records
  recordEnd(c.view, c.cursor, c.flags | KWrite::cfPersistent);
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
  int len;
  int chars, space, okLen;
  QChar ch;
  int extra;
  QString s;
  PointStruc cursor;

  TextLine::Ptr textLine = getTextLine(line);
  len = textLine->length();
  space = 0; // length of space at the beginning of the textline
  okLen = 0; // length of space which does not have to be replaced
  for (chars = 0; chars < len; chars++) {
    ch = textLine->getChar(chars);
    if (ch == ' ') {
      space++;
      if (flags & KWrite::cfSpaceIndent && okLen == chars) okLen++;
    } else if (ch == '\t') {
      space += tabChars - space % tabChars;
      if (!(flags & KWrite::cfSpaceIndent) && okLen == chars) okLen++;
    } else break;
  }

  space += change*tabChars; // modify space width
  // if line contains only spaces it will be cleared
  if (space < 0 || chars == len) space = 0;

  extra = space % tabChars; // extra spaces which don´t fit the indentation pattern
  if (flags & KWrite::cfKeepExtraSpaces) chars -= extra;

  if (flags & KWrite::cfSpaceIndent) {
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

void KWriteDoc::doCommentLine(PointStruc &cursor) {

  QString startComment = m_highlight->getCommentStart() + " ";
  QString endComment = " " + m_highlight->getCommentEnd();

  // Add a start comment mark
  cursor.x = 0;
  recordReplace(cursor, 0, startComment);

  // Add an end comment mark
  if(endComment != " ") {
    TextLine* textline = getTextLine(cursor.y);
    cursor.x = textline->length();
    recordReplace(cursor, 0, endComment);
    cursor.x = 0;
  }
}

void KWriteDoc::doUncommentLine(PointStruc &cursor) {

  QString startComment = m_highlight->getCommentStart() + " ";
  QString otherStartComment = m_highlight->getCommentStart();
  QString endComment = "";
  if(m_highlight->getCommentEnd() != "") {
    endComment = " " + m_highlight->getCommentEnd();
  }

  int startCommentLen = startComment.length();
  int otherStartCommentLen = otherStartComment.length();
  int endCommentLen = endComment.length();

  TextLine* textline = getTextLine(cursor.y);

  if(textline->startingWith(startComment) && textline->endingWith(endComment)) {

    // Remove start comment mark
    cursor.x = 0;
    recordReplace(cursor, startCommentLen, "");

    // Remove end comment mark
    if(endComment != "") {
      cursor.x = textline->length() - endCommentLen;
      recordReplace(cursor, endCommentLen, "");
      cursor.x = 0;
    }

  } else if(textline->startingWith(otherStartComment) && textline->endingWith(endComment)) {

    // Remove start comment mark
    cursor.x = 0;
    recordReplace(cursor, otherStartCommentLen, "");

    // Remove end comment mark
    if(endComment != "") {
      cursor.x = textline->length() - endCommentLen;
      recordReplace(cursor, endCommentLen, "");
      cursor.x = 0;
    }
  }
}

void KWriteDoc::doComment(VConfig &c, int change) {

  c.flags |=KWrite:: cfPersistent;

  recordStart(c, (change < 0) ? KWActionGroup::ugUncomment
    : KWActionGroup::ugComment);

  if (selectEnd < selectStart) {
    if(change > 0) {
      // comment single line
      doCommentLine(c.cursor);
    } else if(change < 0) {
      // uncomment single line
      doUncommentLine(c.cursor);
    }
  } else {
    for (c.cursor.y = selectStart; c.cursor.y <= selectEnd; c.cursor.y++) {
      TextLine* textLine = getTextLine(c.cursor.y);
      if (textLine->isSelected() || textLine->numSelected()) {
        if(change > 0) {
          //comment selection
          doCommentLine(c.cursor);
        } else if(change < 0) {
          //uncomment selection
          doUncommentLine(c.cursor);
        }
      }
    }
    c.cursor.y--;
  }

  recordEnd(c.view, c.cursor, c.flags | KWrite::cfPersistent);
}


QString KWriteDoc::text() const {
  TextLine::List::ConstIterator it = contents.begin();
  QString s;

  for (; it != contents.end(); )
  {
    TextLine::Ptr textLine = *it;
    s.insert(s.length(), textLine->getText(), textLine->length());
    ++it;
    if ( it != contents.end())
      s.append('\n');
  }

  return s;
}

QString KWriteDoc::getWord(PointStruc &cursor) {
  int start, end, len;

  TextLine::Ptr textLine = getTextLine(cursor.y);
  len = textLine->length();
  start = end = cursor.x;
  while (start > 0 && m_highlight->isInWord(textLine->getChar(start - 1))) start--;
  while (end < len && m_highlight->isInWord(textLine->getChar(end))) end++;
  len = end - start;
  return QString(&textLine->getText()[start], len);
}

void KWriteDoc::setText(const QString &s) {
  int pos;
  QChar ch;

  clear();

  TextLine::Ptr textLine = contents.first();
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
  TextLine::Ptr textLine;
  int len, z, start, end, i;

  len = 1;
  if (!(flags & KWrite::cfVerticalSelect)) {
    for (z = selectStart; z <= selectEnd; z++) {
      textLine = getTextLine(z);
      len += textLine->numSelected();
      if (textLine->isSelected()) len++;
    }
    QString s;
    len = 0;
    for (z = selectStart; z <= selectEnd; z++) {
      textLine = getTextLine(z);
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
      textLine = getTextLine(z);
      len += textLine->numSelected() + 1;
    }
    QString s;
    len = 0;
    for (z = selectStart; z <= selectEnd; z++) {
      textLine = getTextLine(z);
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
  int end = 0;

  if (selectEnd < selectStart) return;

  // the caller may have already started an undo record for the current action
//  if (undo)

  //auto deletion of the marked text occurs not very often and can therefore
  //  be recorded separately
  recordStart(c, KWActionGroup::ugDelBlock);

  for (c.cursor.y = selectEnd; c.cursor.y >= selectStart; c.cursor.y--) {
    TextLine::Ptr textLine = getTextLine(c.cursor.y);

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

void KWriteDoc::updateLines(int startLine, int endLine, int flags, int cursorY) {
  TextLine::Ptr textLine;
  int line, last_line;
  int ctxNum, endCtx;

  last_line = lastLine();
  if (endLine >= last_line) endLine = last_line;

  line = startLine;
  ctxNum = 0;
  if (line > 0) ctxNum = getTextLine(line - 1)->getContext();
  do {
    textLine = getTextLine(line);
    if (line <= endLine && line != cursorY) {
      if (flags & KWrite::cfRemoveSpaces) textLine->removeSpaces();
      updateMaxLength(textLine);
    }
    endCtx = textLine->getContext();
    ctxNum = m_highlight->doHighlight(ctxNum,textLine);
    textLine->setContext(ctxNum);
    line++;
  } while (line <= last_line && (line <= endLine || endCtx != ctxNum));
  tagLines(startLine, line - 1);
}


void KWriteDoc::updateMaxLength(TextLine::Ptr &textLine) {
  int len;

  len = textWidth(textLine,textLine->length());

  if (len > maxLength) {
    longestLine = textLine;
    maxLength = len;
    newDocGeometry = true;
  } else {
    if (!longestLine || (textLine == longestLine && len <= maxLength*3/4)) {
      maxLength = -1;
      for (int i = 0; i < numLines();i++) {
        textLine = getTextLine(i);
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

void KWriteDoc::slotBufferChanged() {
  newDocGeometry = true;
  updateViews();
}

void KWriteDoc::updateViews(KWrite *exclude) {
  KWrite *view;
  int flags;
  bool markState = hasMarkedText();

  flags = (newDocGeometry) ? KWrite::ufDocGeometry : 0;
  for (view = views.first(); view != 0L; view = views.next() ) {
    if (view != exclude) view->updateView(flags);

    // notify every view about the changed mark state....
    if (oldMarkState != markState) emit view->newMarkStatus();
  }
  oldMarkState = markState;
  newDocGeometry = false;
}

QColor &KWriteDoc::cursorCol(int x, int y) {
  int attr;
  Attribute *a;

  TextLine::Ptr textLine = getTextLine(y);
  attr = textLine->getRawAttr(x);
  a = &m_attribs[attr & taAttrMask];
  if (attr & taSelectMask) return a->selCol; else return a->col;
}

QFont &KWriteDoc::getTextFont(int x, int y) {
  int attr;
  Attribute *a;

  TextLine::Ptr textLine = getTextLine(y);
  attr = textLine->getRawAttr(x);
  a = &m_attribs[attr & taAttrMask];
  return a->font;
}


void KWriteDoc::paintTextLine(QPainter &paint, int line, int xStart, int xEnd,
  bool showTabs) {

  int y;
  TextLine::Ptr textLine;
  int len;
  const QChar *s;
  int z, x;
  QChar ch;
  Attribute *a = 0L;
  int attr, nextAttr;
  int xs;
  int xc, zc;

//  struct timeval tv1, tv2, tv3; //for profiling
//  struct timezone tz;

  y = 0;//line*fontHeight - yPos;
  if (line > lastLine()) {
    paint.fillRect(0, y, xEnd - xStart,fontHeight, colors[4]);
    return;
  }

  textLine = getTextLine(line);
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
      x += m_tabWidth - (x % m_tabWidth);
    } else {
      a = &m_attribs[textLine->getAttr(z)];
      x += a->fm.width(ch);//a->width(ch);
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
      x += m_tabWidth - (x % m_tabWidth);
    } else {
      a = &m_attribs[attr & taAttrMask];
      x += a->fm.width(ch);//a->width(ch);
    }
    z++;
  }
  paint.fillRect(xs - xStart, y, xEnd - xs, fontHeight, colors[attr >> taShift]);
  len = z; //reduce length to visible length

//gettimeofday(&tv2, &tz);

  // draw text
  x = xc;
  z = zc;
  y += fontAscent;// -1;
  attr = -1;
  while (z < len) {
    ch = s[z];//textLine->getChar(z);
    if (ch == '\t') {
      if (z > zc) {
        //this should cause no copy at all
        QConstString str((QChar *) &s[zc], z - zc /*+1*/);
        QString s = str.string();
        paint.drawText(x - xStart, y, s);
        x += a->fm.width(s);//a->width(s);//&s[zc], z - zc);
      }
      zc = z +1;

      if (showTabs) {
        nextAttr = textLine->getRawAttr(z);
        if (nextAttr != attr) {
          attr = nextAttr;
          a = &m_attribs[attr & taAttrMask];

          if (attr & taSelectMask) paint.setPen(a->selCol);
            else paint.setPen(a->col);
          paint.setFont(a->font);
        }

//        paint.drawLine(x - xStart, y -2, x - xStart, y);
//        paint.drawLine(x - xStart, y, x - xStart + 2, y);
        paint.drawPoint(x - xStart, y);
        paint.drawPoint(x - xStart +1, y);
        paint.drawPoint(x - xStart, y -1);
      }
      x += m_tabWidth - (x % m_tabWidth);
    } else {
      nextAttr = textLine->getRawAttr(z);
      if (nextAttr != attr) {
        if (z > zc) {
          QConstString str((QChar *) &s[zc], z - zc /*+1*/);
          QString s = str.string();
          paint.drawText(x - xStart, y, s);
          x += a->fm.width(s);//a->width(s);//&s[zc], z - zc);
          zc = z;
        }
        attr = nextAttr;
        a = &m_attribs[attr & taAttrMask];

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

/*
void KWriteDoc::printTextLine(QPainter &paint, int line, int xEnd, int y) {
  TextLine::Ptr textLine;
  int z, x;
  Attribute *a = 0L;
  int attr, nextAttr;
  char ch;
  char buf[256];
  int bufp;

  if (line > lastLine()) return;
  textLine = getTextLine(line);

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
*/

void KWriteDoc::setURL( const KURL &url, bool updateHighlight )
{
  int hl;
  KTextEditor::View *view;

  m_url = url;
  for (view = m_views.first(); view != 0L; view = m_views.next() ) {
    emit static_cast<KWrite *>( view )->fileChanged();
  }

  if ( updateHighlight )
  {
    //highlight detection
    //  pos = fName.findRev('/') +1;
    //  if (pos >= (int) fName.length()) return; //no filename
    //  hl = hlManager->wildcardFind(s.right( s.length() - pos ));
    QString fn = m_url.fileName();
    if ( fn.isEmpty() )
        return;
    hl = hlManager->wildcardFind( fn );

    if (hl == -1) {
      // fill the detection buffer with the contents of the text
      const int HOWMANY = 1024;
      QByteArray buf(HOWMANY);
      int bufpos = 0, len;
      for (TextLine::List::ConstIterator it = contents.begin();
           it != contents.end();
           ++it)
      {
        TextLine::Ptr textLine = *it;
        len = textLine->length();
        if (bufpos + len > HOWMANY) len = HOWMANY - bufpos;
        memcpy(&buf[bufpos], textLine->getText(), len);
        bufpos += len;
        if (bufpos >= HOWMANY) break;
      }
      //    hl = hlManager->mimeFind(buf, s.right( s.length() - pos));
      hl = hlManager->mimeFind( buf, fn );
    }
    setHighlight(hl);
  }
  updateViews();
}

void KWriteDoc::clearFileName() {
  KTextEditor::View *view;

  //  fName.truncate(fName.findRev('/') +1);
  m_url = KURL();
  for (view = m_views.first(); view != 0L; view = m_views.next() ) {
    emit static_cast<KWrite *>( view )->fileChanged();
  }
}

// Applies the search context, and returns whether a match was found. If one is,
// the length of the string matched is also returned.
bool KWriteDoc::doSearch(SConfig &sc, const QString &searchFor) {
  int line, col;
  int searchEnd;
  int bufLen, tlen;
  QChar *t;
  TextLine::Ptr textLine;
  int pos, newPos;

  if (searchFor.isEmpty()) return false;

  bufLen = 0;
  t = 0L;

  line = sc.cursor.y;
  col = sc.cursor.x;
  if (!(sc.flags & KWrite::sfBackward)) {
    //forward search
    if (sc.flags & KWrite::sfSelected) {
      if (line < selectStart) {
        line = selectStart;
        col = 0;
      }
      searchEnd = selectEnd;
    } else searchEnd = lastLine();

    while (line <= searchEnd) {
      textLine = getTextLine(line);
      tlen = textLine->length();
      if (tlen > bufLen) {
        delete t;
        bufLen = (tlen + 255) & (~255);
        t = new QChar[bufLen];
      }
      memcpy(t, textLine->getText(), tlen*sizeof(QChar));
      if (sc.flags & KWrite::sfSelected) {
        pos = 0;
        do {
          pos = textLine->findSelected(pos);
          newPos = textLine->findUnselected(pos);
          memset(&t[pos], 0, (newPos - pos)*sizeof(QChar));
          pos = newPos;
        } while (pos < tlen);
      }

      QString text(t, tlen);
      if (sc.flags & KWrite::sfWholeWords) {
        // Until the end of the line...
        while (col < tlen) {
          // ...find the next match.
          col = sc.search(text, col);
          if (col != -1) {
            // Is the match delimited correctly?
            if (((col == 0) || (!m_highlight->isInWord(t[col]))) &&
              ((col + sc.matchedLength == tlen) || (!m_highlight->isInWord(t[col + sc.matchedLength])))) {
              goto found;
            }
            else {
              // Start again from the next character.
              col++;
            }
          }
          else {
            // No match.
            break;
          }
        }
      }
      else {
        // Non-whole-word search.
        col = sc.search(text, col);
        if (col != -1)
          goto found;
      }
      col = 0;
      line++;
    }
  } else {
    // backward search
    if (sc.flags & KWrite::sfSelected) {
      if (line > selectEnd) {
        line = selectEnd;
        col = -1;
      }
      searchEnd = selectStart;
    } else searchEnd = 0;

    while (line >= searchEnd) {
      textLine = getTextLine(line);
      tlen = textLine->length();
      if (tlen > bufLen) {
        delete t;
        bufLen = (tlen + 255) & (~255);
        t = new QChar[bufLen];
      }
      memcpy(t, textLine->getText(), tlen*sizeof(QChar));
      if (sc.flags & KWrite::sfSelected) {
        pos = 0;
        do {
          pos = textLine->findSelected(pos);
          newPos = textLine->findUnselected(pos);
          memset(&t[pos], 0, (newPos - pos)*sizeof(QChar));
          pos = newPos;
        } while (pos < tlen);
      }

      if (col < 0 || col > tlen) col = tlen;

      QString text(t, tlen);
      if (sc.flags & KWrite::sfWholeWords) {
        // Until the beginning of the line...
        while (col >= 0) {
          // ...find the next match.
          col = sc.search(text, col);
          if (col != -1) {
            // Is the match delimited correctly?
            if (((col == 0) || (!m_highlight->isInWord(t[col]))) &&
              ((col + sc.matchedLength == tlen) || (!m_highlight->isInWord(t[col + sc.matchedLength])))) {
              goto found;
            }
            else {
              // Start again from the previous character.
              col--;
            }
          }
          else {
            // No match.
            break;
          }
        }
      }
      else {
        // Non-whole-word search.
        col = sc.search(text, col);
        if (col != -1)
          goto found;
      }
      col = -1;
      line--;
    }
  }
  sc.flags |= KWrite::sfWrapped;
  return false;
found:
  if (sc.flags & KWrite::sfWrapped) {
    if ((line > sc.startCursor.y || (line == sc.startCursor.y && col >= sc.startCursor.x))
      ^ ((sc.flags & KWrite::sfBackward) != 0)) return false;
  }
  sc.cursor.x = col;
  sc.cursor.y = line;
  return true;
}

void KWriteDoc::unmarkFound() {
  if (pseudoModal) return;
  if (foundLine != -1) {
    getTextLine(foundLine)->unmarkFound();
    tagLines(foundLine,foundLine);
    foundLine = -1;
  }
}

void KWriteDoc::markFound(PointStruc &cursor, int len) {
//  unmarkFound();
//  recordReset();
  if (foundLine != -1) {
    getTextLine(foundLine)->unmarkFound();
    tagLines(foundLine,foundLine);
  }
  getTextLine(cursor.y)->markFound(cursor.x,len);
  foundLine = cursor.y;
  tagLines(foundLine,foundLine);
}


void KWriteDoc::tagLine(int line) {

  if (tagStart > line) tagStart = line;
  if (tagEnd < line) tagEnd = line;
}

void KWriteDoc::insLine(int line) {
  KWrite *view;

  if (selectStart >= line) selectStart++;
  if (selectEnd >= line) selectEnd++;
  if (tagStart >= line) tagStart++;
  if (tagEnd >= line) tagEnd++;

  newDocGeometry = true;
  for (view = views.first(); view != 0L; view = views.next() ) {
    view->insLine(line);
  }
}

void KWriteDoc::delLine(int line) {
  KWrite *view;

  if (selectStart >= line && selectStart > 0) selectStart--;
  if (selectEnd >= line) selectEnd--;
  if (tagStart >= line && tagStart > 0) tagStart--;
  if (tagEnd >= line) tagEnd--;

  newDocGeometry = true;
  for (view = views.first(); view != 0L; view = views.next() ) {
    view->delLine(line);
  }
}

void KWriteDoc::optimizeSelection() {
  TextLine::Ptr textLine;

  while (selectStart <= selectEnd) {
    textLine = getTextLine(selectStart);
    if (textLine->isSelected() || textLine->numSelected() > 0) break;
    selectStart++;
  }
  while (selectEnd >= selectStart) {
    textLine = getTextLine(selectEnd);
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
  TextLine::Ptr textLine;
  int l;

  //exchange current text with stored text in KWAction *a

  textLine = getTextLine(a->cursor.y);
  l = textLine->length() - a->cursor.x;
  if (l > a->len) l = a->len;

  QString oldText(&textLine->getText()[a->cursor.x], (l < 0) ? 0 : l);
  textLine->replace(a->cursor.x, a->len, a->text.unicode(), a->text.length());

  a->len = a->text.length();
  a->text = oldText;
#ifdef NEW_CODE
  buffer->changeLine(a->cursor.y);
#endif

  tagLine(a->cursor.y);
}

void KWriteDoc::doWordWrap(KWAction *a) {
  TextLine::Ptr textLine;

  textLine = getTextLine(a->cursor.y - 1);
  a->len = textLine->length() - a->cursor.x;
  textLine->wrap(getTextLine(a->cursor.y),a->len);

#ifdef NEW_CODE
  buffer->changeLine(a->cursor.y - 1);
  buffer->changeLine(a->cursor.y);
#endif

  tagLine(a->cursor.y - 1);
  tagLine(a->cursor.y);
  if (selectEnd == a->cursor.y - 1) selectEnd++;

  a->action = KWAction::wordUnWrap;
}

void KWriteDoc::doWordUnWrap(KWAction *a) {
  TextLine::Ptr textLine;

  textLine = getTextLine(a->cursor.y - 1);
//  textLine->setLength(a->len);
  textLine->unWrap(a->len, getTextLine(a->cursor.y),a->cursor.x);

#ifdef NEW_CODE
  buffer->changeLine(a->cursor.y - 1);
  buffer->changeLine(a->cursor.y);
#endif

  tagLine(a->cursor.y - 1);
  tagLine(a->cursor.y);

  a->action = KWAction::wordWrap;
}

void KWriteDoc::doNewLine(KWAction *a) {
  TextLine::Ptr textLine, newLine;

  textLine = getTextLine(a->cursor.y);
  newLine = new TextLine(textLine->getRawAttr(), textLine->getContext());
  textLine->wrap(newLine,a->cursor.x);
#ifdef NEW_CODE
  buffer->insertLine(a->cursor.y + 1, newLine);
  buffer->changeLine(a->cursor.y);
#else
  contents.insert(contents.at(a->cursor.y + 1),newLine);
#endif

  insLine(a->cursor.y + 1);
  tagLine(a->cursor.y);
  tagLine(a->cursor.y + 1);
  if (selectEnd == a->cursor.y) selectEnd++;//addSelection(a->cursor.y + 1);

  a->action = KWAction::delLine;
}

void KWriteDoc::doDelLine(KWAction *a) {
  TextLine::Ptr textLine, nextLine;

  textLine = getTextLine(a->cursor.y);
  nextLine = getTextLine(a->cursor.y+1);
//  textLine->setLength(a->cursor.x);
  textLine->unWrap(a->cursor.x, nextLine,nextLine->length());
  textLine->setContext(nextLine->getContext());
  if (longestLine == nextLine) longestLine = 0L;
#ifdef NEW_CODE
  buffer->changeLine(a->cursor.y);
  buffer->removeLine(a->cursor.y+1);
#else
  contents.remove(contents.at(a->cursor.y+1));
#endif

  tagLine(a->cursor.y);
  delLine(a->cursor.y + 1);

  a->action = KWAction::newLine;
}

void KWriteDoc::doInsLine(KWAction *a) {

#ifdef NEW_CODE
  buffer->insertLine(a->cursor.y, new TextLine());
#else
  contents.insert(contents.at(a->cursor.y),new TextLine());
#endif

  insLine(a->cursor.y);

  a->action = KWAction::killLine;
}

void KWriteDoc::doKillLine(KWAction *a) {
  TextLine::Ptr textLine = getTextLine(a->cursor.y);
  if (longestLine == textLine) longestLine = 0L;
#ifdef NEW_CODE
  buffer->removeLine(a->cursor.y);
#else
  contents.remove(contents.at(a->cursor.y));
#endif

  delLine(a->cursor.y);
  tagLine(a->cursor.y);

  a->action = KWAction::insLine;
}

void KWriteDoc::newUndo() {
  KTextEditor::View *view;
  int state;

  state = 0;
  if (currentUndo > 0) state |= 1;
  if (currentUndo < (int) undoList.count()) state |= 2;
  undoState = state;
  for (view = m_views.first(); view != 0L; view = m_views.next() ) {
    emit static_cast<KWrite *>( view )->newUndo();
  }
}

void KWriteDoc::recordStart(VConfig &c, int newUndoType) {
  recordStart(c.view, c.cursor, c.flags, newUndoType);
}

void KWriteDoc::recordStart(KWrite *, PointStruc &cursor, int flags,
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
  if (g != 0L && ((undoCount < 1024 && flags & KWrite::cfGroupUndo
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

void KWriteDoc::recordInsert(VConfig &c, const QString &text) {
  recordReplace(c, 0, text);
}

void KWriteDoc::recordReplace(VConfig &c, int len, const QString &text) {
  if (c.cursor.x > 0 && !(c.flags & KWrite::cfSpaceIndent)) {
    TextLine::Ptr textLine = getTextLine(c.cursor.y);
    if (textLine->length() == 0) {
      QString s = tabString(c.cursor.x, tabChars);
      int len = s.length();
      s += text;
      c.cursor.x = 0;
      recordReplace(c.cursor, len, s);
      c.cursor.x = len;
      return;
    }
  }
  recordReplace(c.cursor, len, text);
}

void KWriteDoc::recordInsert(PointStruc &cursor, const QString &text) {
  recordReplace(cursor, 0, text);
}

void KWriteDoc::recordDelete(PointStruc &cursor, int len) {
  recordReplace(cursor, len, QString::null);
}

void KWriteDoc::recordReplace(PointStruc &cursor, int len, const QString &text) {
  KWAction *a;
  TextLine::Ptr textLine;
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
  textLine = getTextLine(cursor.y);
  l = textLine->length() - cursor.x;
  if (l > len) l = len;
  a->text.insert(a->text.length(), &textLine->getText()[cursor.x], (l < 0) ? 0 : l);
  textLine->replace(cursor.x, len, text.unicode(), text.length());
  a->len += text.length();

#ifdef NEW_CODE
  buffer->changeLine(a->cursor.y);
#endif

  tagLine(a->cursor.y);
}

void KWriteDoc::recordEnd(VConfig &c) {
  recordEnd(c.view, c.cursor, c.flags);
}

void KWriteDoc::recordEnd(KWrite *view, PointStruc &cursor, int flags) {
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

  if (tagStart <= tagEnd) {
    optimizeSelection();
    updateLines(tagStart, tagEnd, flags, cursor.y);
    setModified(true);
  }

  view->updateCursor(cursor, flags);

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
void KWriteDoc::recordDel(PointStruc &cursor, TextLine::Ptr &textLine, int l) {
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
  if (!(flags & KWrite::cfPersistent)) deselectAll();
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
  if (tagStart <= tagEnd) updateLines(tagStart, tagEnd, flags);

  // the undo/redo functions set undo to true, all others should leave it
  // alone (default)
  if (!undo) {
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

void KWriteDoc::undoTypeList(QValueList<int> &lst)
{
  lst.clear();
  for (int i = currentUndo-1; i>=0 ;i--)
    lst.append(undoList.at(i)->undoType);
}

void KWriteDoc::redoTypeList(QValueList<int> &lst)
{
  lst.clear();
  for (int i = currentUndo+1; i<(int)undoList.count(); i++)
    lst.append(undoList.at(i)->undoType);
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


void KWriteDoc::newBracketMark(PointStruc &cursor, BracketMark &bm) {
  TextLine::Ptr textLine;
  int x, line, count, attr;
  QChar bracket, opposite, ch;
  Attribute *a;

  bm.eXPos = -1; //mark bracked mark as invalid

  x = cursor.x -1; // -1 to look at left side of cursor
  if (x < 0) return;
  line = cursor.y; //current line
  count = 0; //bracket counter for nested brackets
  textLine = getTextLine(line);
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
        textLine = getTextLine(line);
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
        textLine = getTextLine(line);
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
  a = &m_attribs[attr];
  bm.eXPos = bm.sXPos + a->fm.width(bracket);//a->width(bracket);
}

void KWriteDoc::clipboardChanged() { //slot
//#if defined(_WS_X11_)
  if (m_singleSelection) {
    disconnect(QApplication::clipboard(), SIGNAL(dataChanged()),
      this, SLOT(clipboardChanged()));
    deselectAll();
    updateViews();
  }
//#endif
}

void KWriteDoc::guiActivateEvent( KParts::GUIActivateEvent *ev )
{
  KParts::ReadWritePart::guiActivateEvent( ev );
  if ( ev->activated() )
    emit selectionChanged();
}

KWriteBrowserExtension::KWriteBrowserExtension( KWriteDoc *doc )
: KParts::BrowserExtension( doc, "kwritebrowserextension" )
{
  m_doc = doc;
  connect( m_doc, SIGNAL( selectionChanged() ),
           this, SLOT( slotSelectionChanged() ) );
}

void KWriteBrowserExtension::copy()
{
  m_doc->copy( 0 );
}

void KWriteBrowserExtension::slotSelectionChanged()
{
  emit enableAction( "copy", m_doc->hasMarkedText() );
}

