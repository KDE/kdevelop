#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <qstring.h>
#include <qwidget.h>
#include <qfont.h>
#include <qpainter.h>
#include <qkeycode.h>
#include <qmsgbox.h>
#include <qpixmap.h>
#include <qfileinf.h>
#include <qfile.h>
#include <qdir.h>

#include <kapp.h>
#include <kfiledialog.h>

#include <kdir.h>

#include <X11/Xlib.h> //used to have XSetTransientForHint()

#include "kwview.h"
#include "kwdoc.h"

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

int keys[] = {Key_1,Key_2,Key_3,Key_4,Key_5,Key_6,Key_7,Key_8,Key_9};

struct BufferInfo {
  void *user;
  int w;
  int h;
};

QList<BufferInfo> bufferInfoList;
QPixmap *buffer = 0;

QPixmap *getBuffer(void *user) {
  BufferInfo *info;

  if (!buffer) buffer = new QPixmap;
  info = new BufferInfo;
  info->user = user;
  info->w = 0;
  info->h = 0;
  bufferInfoList.append(info);
  return buffer;
}

void resizeBuffer(void *user, int w, int h) {
  int z;
  BufferInfo *info;
  int maxW, maxH;

  maxW = w;
  maxH = h;
  for (z = 0; z < (int) bufferInfoList.count(); z++) {
    info = bufferInfoList.at(z);
    if (info->user == user) {
      info->w = w;
      info->h = h;
    } else {
      if (info->w > maxW) maxW = info->w;
      if (info->h > maxH) maxH = info->h;
    }
  }
  if (maxW != buffer->width() || maxH != buffer->height()) {
    buffer->resize(maxW,maxH);
  }
}

void releaseBuffer(void *user) {
  int z;
  BufferInfo *info;

  for (z = (int) bufferInfoList.count() -1; z >= 0 ; z--) {
    info = bufferInfoList.at(z);
    if (info->user == user) bufferInfoList.remove(z);
  }
  resizeBuffer(0,0,0);
}


KWriteView::KWriteView(KWrite *write, KWriteDoc *doc) : QWidget(write) {
  kWrite = write;
  kWriteDoc = doc;

  QWidget::setCursor(ibeamCursor);
  setBackgroundMode(NoBackground);
  setFocusPolicy(StrongFocus);
  move(2,2);

  xScroll = new QScrollBar(QScrollBar::Horizontal,write);
  yScroll = new QScrollBar(QScrollBar::Vertical,write);
  connect(xScroll,SIGNAL(valueChanged(int)),SLOT(changeXPos(int)));
  connect(yScroll,SIGNAL(valueChanged(int)),SLOT(changeYPos(int)));

  xPos = 0;
  yPos = 0;

  scrollTimer = 0;

  cursor.x = 0;
  cursor.y = 0;
  cursorOn = false;
  cursorTimer = 0;
  cXPos = 0;
  cOldXPos = 0;
  exposeCursor = false;

  startLine = 0;
  endLine = 0;
  updateState = 0;

  drawBuffer = getBuffer(this);

  doc->registerView(this);
}

KWriteView::~KWriteView() {
  kWriteDoc->removeView(this);
  releaseBuffer(this);
}

void KWriteView::cursorLeft(VConfig &c) {

  cursor.x--;
  if (c.flags & cfWrapCursor && cursor.x < 0 && cursor.y > 0) {
    cursor.y--;
    cursor.x = kWriteDoc->textLength(cursor.y);
  }
  cOldXPos = cXPos = kWriteDoc->textWidth(cursor);
  update(c);
}

void KWriteView::cursorRight(VConfig &c) {

  if (c.flags & cfWrapCursor) {
    if (cursor.x >= kWriteDoc->textLength(cursor.y)) {
      if (cursor.y == kWriteDoc->lastLine()) return;
      cursor.y++;
      cursor.x = -1;
    }
  }  
  cursor.x++;
  cOldXPos = cXPos = kWriteDoc->textWidth(cursor);
  update(c);
}


void KWriteView::cursorUp(VConfig &c) {

  cursor.y--;
  cXPos = kWriteDoc->textWidth(c.flags & cfWrapCursor,cursor,cOldXPos);
  update(c);
}

void KWriteView::cursorDown(VConfig &c) {
  int x;
  
  if (cursor.y == kWriteDoc->lastLine()) {
    x = kWriteDoc->textLength(cursor.y);
    if (cursor.x >= x) return;
    cursor.x = x;
    cXPos = kWriteDoc->textWidth(cursor);
  } else {
    cursor.y++;
    cXPos = kWriteDoc->textWidth(c.flags & cfWrapCursor,cursor,cOldXPos);
  }  
  update(c);
}

void KWriteView::home(VConfig &c) {

  cursor.x = 0;
  cOldXPos = cXPos = 0;
  update(c);
}

void KWriteView::end(VConfig &c) {
  
  cursor.x = kWriteDoc->textLength(cursor.y);
  cOldXPos = cXPos = kWriteDoc->textWidth(cursor);
  update(c);
}

void KWriteView::pageUp(VConfig &c) {

  cursor.y -= endLine - startLine;
  cXPos = kWriteDoc->textWidth(c.flags & cfWrapCursor,cursor,cOldXPos);
  update(c);
}

void KWriteView::pageDown(VConfig &c) {
  
  cursor.y += endLine - startLine;
  cXPos = kWriteDoc->textWidth(c.flags & cfWrapCursor,cursor,cOldXPos);
  update(c);
}

void KWriteView::top(VConfig &c) {

  cursor.x = 0;
  cursor.y = 0;
  cOldXPos = cXPos = 0;
  update(c);
}

void KWriteView::bottom(VConfig &c) {
  
  cursor.x = 0;
  cursor.y = kWriteDoc->lastLine();
  cOldXPos = cXPos = 0;
  update(c);
}


void KWriteView::changeXPos(int p) {
  int dx;

  dx = xPos - p;
  xPos = p;
  if (QABS(dx) < width()) scroll(dx,0); else QWidget::update();
}

void KWriteView::changeYPos(int p) {
  int dy;

  dy = yPos - p;
  yPos = p;
  startLine = yPos / kWriteDoc->fontHeight;
  endLine = (yPos + height() -1) / kWriteDoc->fontHeight;
  if (QABS(dy) < height()) scroll(0,dy); else QWidget::update();
}


void KWriteView::getVConfig(VConfig &c) {

  c.cursor = cursor;
  c.flags = kWrite->configFlags;
  c.wrapAt = kWrite->wrapAt;
}



void KWriteView::update(VConfig &c) {

  if (cursor.x == c.cursor.x && cursor.y == c.cursor.y) return;
  exposeCursor = true;

  kWriteDoc->unmarkFound();
  
  if (cursorOn) {
    tagLines(c.cursor.y,c.cursor.y);
    cursorOn = false;
  }

  if (c.flags & cfMark) {
    kWriteDoc->selectTo(c.cursor,cursor,c.flags);
  } else {
    if (!(c.flags & cfPersistent)) kWriteDoc->deselectAll();
  }
}

/*
void KWriteView::updateCursor(PointStruc &start, PointStruc &end, bool insert) {
  int dy;
  PointStruc oldCursor;

  oldCursor = cursor;
  dy = end.y - start.y;

  if (cursor.y > start.y) {
    cursor.y += dy;
  } else if (cursor.y == start.y && cursor.x >= start.x) {
    cursor.y += dy;
    cursor.x += end.x - start.x;
  } else if (cursor.y > end.y || (cursor.y == end.y && cursor.x > end.x)) {
    cursor.y = end.y;
    cursor.x = end.x;
  }

  if (cursor.x != oldCursor.x || cursor.y != oldCursor.y) {
    exposeCursor = true;
    cOldXPos = cXPos = kWriteDoc->textWidth(cursor);
  }

  if (insert) {
    if (start.y < startLine && end.y < startLine) {
      startLine += dy;
      endLine += dy;
      yPos += dy*kWriteDoc->fontHeight;
    } else if (start.y <= endLine || end.y <= endLine) {
      if (dy == 0) {
        if (start.y == cursor.y) cursorOn = false;
        tagLines(start.y,start.y);
      } else tagAll();
    }
  } else {
    if (dy >= 0) tagLines(start.y,end.y); else tagLines(end.y,start.y);
  }
}
*/
void KWriteView::insLine(int line) {

  if (line <= cursor.y) {
    cursor.y++;
//    cursorMoved = true;
  }  
  if (line < startLine) {
    startLine++;
    endLine++;
    yPos += kWriteDoc->fontHeight;
  } else if (line <= endLine) {
    tagAll();
  }  

  //bookmarks
  KWBookmark *b;
  for (b = kWrite->bookmarks.first(); b != 0L; b = kWrite->bookmarks.next()) {
    if (b->cursor.y >= line) {
      b->cursor.y++;
      b->yPos += kWriteDoc->fontHeight;
    }
  }
}

void KWriteView::delLine(int line) {

  if (line <= cursor.y && cursor.y > 0) {
    cursor.y--;
  }
  if (line < startLine) {
    startLine--;
    endLine--;
    yPos -= kWriteDoc->fontHeight;
  } else if (line <= endLine) {
    tagAll();
  }  

  //bookmarks
  KWBookmark *b;
  for (b = kWrite->bookmarks.first(); b != 0L; b = kWrite->bookmarks.next()) {
    if (b->cursor.y > line) {
      b->cursor.y--;
      b->yPos -= kWriteDoc->fontHeight;
//      if (b->yPos < 0) b->yPos = 0;
    }
  }
}

void KWriteView::updateCursor() {
  cOldXPos = cXPos = kWriteDoc->textWidth(cursor);
}

void KWriteView::updateCursor(PointStruc &newCursor) {

  exposeCursor = true;
  if (cursorOn) {
    tagLines(cursor.y,cursor.y);
    cursorOn = false;
  }
  cursor = newCursor;
  cOldXPos = cXPos = kWriteDoc->textWidth(cursor);
}

void KWriteView::updateView(int flags, int newXPos, int newYPos) {
  int fontHeight;
  int oldXPos, oldYPos;
  int w, h;
  int z;
  bool b;
  int xMax, yMax;
  int cYPos;
  int cXPosMin, cXPosMax, cYPosMin, cYPosMax;
  int dx, dy;

  if (exposeCursor || flags & ufDocGeometry) {
    emit kWrite->newCurPos();
  } else if (updateState == 0) return;

  if (cursorTimer) {
    killTimer(cursorTimer);
    cursorTimer = startTimer(500);
    cursorOn = true;
  }

  oldXPos = xPos;
  oldYPos = yPos;
  if (flags & ufPos) {
    xPos = newXPos;
    yPos = newYPos;
    exposeCursor = true;//false;
  }

  fontHeight = kWriteDoc->fontHeight;
  cYPos = cursor.y*fontHeight;

  z = 0;
  do {
    w = kWrite->width() - 4;
    h = kWrite->height() - 4;

    xMax = kWriteDoc->textWidth() - w;
    b = (xPos > 0 || xMax > 0);
    if (b) h -= 16;
    yMax = kWriteDoc->textHeight() - h;
    if (yPos > 0 || yMax > 0) {
      w -= 16;
      xMax += 16;
      if (!b && xMax > 0) {
        h -= 16;
        yMax += 16;
      }
    }

    if (!exposeCursor) break;
//    if (flags & ufNoScroll) break;
/*
    if (flags & ufCenter) {
      cXPosMin = xPos + w/3;
      cXPosMax = xPos + (w*2)/3;
      cYPosMin = yPos + h/3;
      cYPosMax = yPos + ((h - fontHeight)*2)/3;
    } else {*/
      cXPosMin = xPos + 4;
      cXPosMax = xPos + w - 8;
      cYPosMin = yPos;
      cYPosMax = yPos + (h - fontHeight);
//    }

    if (cXPos < cXPosMin) {
      xPos -= cXPosMin - cXPos;
    }
    if (xPos < 0) xPos = 0;
    if (cXPos > cXPosMax) {
      xPos += cXPos - cXPosMax;
    }
    if (cYPos < cYPosMin) {
      yPos -= cYPosMin - cYPos;
    }
    if (yPos < 0) yPos = 0;
    if (cYPos > cYPosMax) {
      yPos += cYPos - cYPosMax;
    }

/*
    if (cXPos < xPos + 4) {
      xPos = cXPos - 8;
      if (xPos < 0) xPos = 0;
    }
    if (cXPos > xPos + w - 4) {
      xPos = cXPos - w +8;
    }

    if (cYPos < yPos) {
      yPos = cYPos;
      if (yPos < 0) yPos = 0;
    }
    if (cYPos > yPos + (h-fontHeight)) {
      yPos = cYPos - (h-fontHeight);
    }
*/
    z++;
  } while (z < 2);

  if (xMax < xPos) xMax = xPos;
  if (yMax < yPos) yMax = yPos;

  if (xMax > 0) {
    xScroll->blockSignals(true);
    xScroll->setGeometry(2,h + 2,w,16);
    xScroll->setRange(0,xMax);
    xScroll->setValue(xPos);
    xScroll->setSteps(fontHeight,w - 4 - 16);
    xScroll->blockSignals(false);
    xScroll->show();
  } else xScroll->hide();

  if (yMax > 0) {
    yScroll->blockSignals(true);
    yScroll->setGeometry(w + 2,2,16,h);
    yScroll->setRange(0,yMax);
    yScroll->setValue(yPos);
    yScroll->setSteps(fontHeight,h - 4 - 16);
    yScroll->blockSignals(false);
    yScroll->show();
  } else yScroll->hide();

  startLine = yPos / fontHeight;
  endLine = (yPos + h -1) / fontHeight;

  if (w != width() || h != height()) {
    resize(w,h);
  } else {
    dx = oldXPos - xPos;
    dy = oldYPos - yPos;

    b = updateState == 3;
    if (flags & ufUpdateOnScroll) {
      b |= dx || dy;
    } else {
      b |= QABS(dx)*3 > w*2 || QABS(dy)*3 > h*2;
    }

    if (b) {
      QWidget::update();
    } else {
      if (updateState > 0) paintTextLines(oldXPos,oldYPos);

      if (dx || dy) {
        scroll(dx,dy);
//        kapp->syncX();
//        scroll2(dx - dx/2,dy - dy/2);
      } else if (cursorOn) paintCursor();
    }
  }
  exposeCursor = false;
  updateState = 0;
}

/*
void KWriteView::scroll2( int dx, int dy ) {
  int w, h, x, y;
  Display *dpy;
  int winid;

  if ( dx == 0 && dy == 0 ) return;
  dpy = qt_xdisplay();
  winid = winId();
  w = width();
  h = height();

  XCopyArea( dpy, winid, winid, qt_xget_readonly_gc(),-dx,-dy,w,h,0,0);
//XSync(qt_xdisplay(),false);
usleep(100000);
*/
/*
  if ( dx ) {
    x = (dx < 0) ? w + dx : 0;
    dx = QABS(dx);
    XClearArea( dpy, winid,x,0,dx/2,h, TRUE);
    XClearArea( dpy, winid,x + dx/2,0,dx - dx/2,h, TRUE);
  }
  if ( dy ) {
    y = (dy < 0) ? h + dy : 0;
    XClearArea( dpy, winid,0,y,w,QABS(dy), TRUE);
  }
*/
//}


void KWriteView::tagLines(int start, int end) {
  int line, z;

  if (updateState < 3) {
    if (start < startLine) start = startLine;
    if (end > endLine) end = endLine;

    if (end - start > 1) {
      updateState = 3;
    } else {
      for (line = start; line <= end; line++) {
        for (z = 0; z < updateState && updateLines[z] != line; z++);
        if (z == updateState) {
          updateState++;
          if (updateState > 2) break;
          updateLines[z] = line;
        }
      }
    }
  }
}

void KWriteView::tagAll() {
  updateState = 3;
}

void KWriteView::paintTextLines(int xPos, int yPos) {
  int xStart, xEnd;
  int line, z;
  int h;

  QPainter paint;
  paint.begin(drawBuffer);

  xStart = xPos-2;
  xEnd = xStart + width();
  h = kWriteDoc->fontHeight;
  for (z = 0; z < updateState; z++) {
    line = updateLines[z];
    kWriteDoc->paintTextLine(paint,line,xStart,xEnd);
    bitBlt(this,0,line*h - yPos,drawBuffer,0,0,width(),h);
  }
  paint.end();
}

void KWriteView::paintCursor() {
  int h, y, x;

  h = kWriteDoc->fontHeight;
  y = h*cursor.y - yPos;
  x = cXPos - (xPos-2);

  QPainter paint;
  if (cursorOn) {
    paint.begin(this);
    paint.setPen(kWriteDoc->cursorCol(cursor.x,cursor.y));

    h += y - 1;
    paint.drawLine(x,y,x,h);
    paint.drawLine(x-2,y,x+2,y);
    paint.drawLine(x-2,h,x+2,h);
  } else {
    paint.begin(drawBuffer);
    kWriteDoc->paintTextLine(paint,cursor.y,cXPos - 2,cXPos + 3);
    bitBlt(this,x - 2,y,drawBuffer,0,0,5,h);
  }
  paint.end();
}

void KWriteView::placeCursor(int x, int y, int flags) {
  VConfig c;

  getVConfig(c);
  c.flags |= flags;
  cursor.y = (yPos + y)/kWriteDoc->fontHeight;
  cXPos = cOldXPos = kWriteDoc->textWidth(c.flags & cfWrapCursor, cursor,xPos-2 + x);
  update(c);
}

void KWriteView::focusInEvent(QFocusEvent *) {
//  printf("got focus %d\n",cursorTimer);

  if (!cursorTimer) {
    cursorTimer = startTimer(500);
    cursorOn = true;
    paintCursor();
  }
}

void KWriteView::focusOutEvent(QFocusEvent *) {
//  printf("lost focus\n");

  if (cursorTimer) {
    killTimer(cursorTimer);
    cursorTimer = 0;
  }

  if (cursorOn) {
    cursorOn = false;
    paintCursor();
  }
}

void KWriteView::keyPressEvent(QKeyEvent *e) {
  VConfig c;
  bool t;

  if (e->state() & AltButton) {
    e->ignore();
    return;
  }
//  printf("ascii %i, key %i, state %i\n",e->ascii(), e->key(), e->state());

/*
CTRL+
A, <-  : home
B      : <-
C      : copy
D      : del
E, ->  : end
F      : ->
H      : backspace
K      : kill line
N      : down
P      : up
V      : paste
X      : cut
*/

  getVConfig(c);

  if ((e->ascii() >= 32 || e->ascii() == '\t')
    && e->key() != Key_Delete && e->key() != Key_Backspace) {
//    printf("input %d\n",e->ascii());
    if (c.flags & cfDelOnInput) {
      kWriteDoc->delMarkedText(this,c);
      getVConfig(c);
    }
    kWriteDoc->insertChar(this,c,e->ascii());
  } else {
    if (e->state() & ShiftButton) c.flags |= cfMark;

    t = false;
    if ( e->state() & ControlButton ) {
      t = true;
      switch ( e->key() ) {
/*
        case Key_A:
        case Key_Left:
            home(c);
            break;
        case Key_B:
            cursorLeft(c);
            break; */
        case Key_Insert:
//        case Key_C:
            kWriteDoc->copy(c.flags);
            break;
/*        case Key_D:
            kWriteDoc->del(c);
            break;
        case Key_E:
        case Key_Right:
            end(c);
            break;
        case Key_F:
            cursorRight(c);
            break;
        case Key_H:
            kWriteDoc->backspace(c);
            break;   */
        case Key_K:
            kWriteDoc->killLine(this,c);
            break;
/*        case Key_N:
            cursorDown(c);
            break;
        case Key_P:
            cursorUp(c);
            break;
        case Key_V:
            kWriteDoc->paste(c);
            break;   */
        case Key_Delete:
//        case Key_X:
            kWriteDoc->cut(this,c);
            break;
//      case Key_Left:
//          cursorLeft(c);
//          break;
//      case Key_Right:
//          cursorRight(c);
//          break;
        case Key_Next:
            bottom(c);
            break;
        case Key_Prior:
            top(c);
            break;
        default:
            t = false;
      }
    }
    if (!t) {
      if (e->state() & ControlButton) c.flags |= cfMark | cfKeepSelection;
      switch ( e->key() ) {
        case Key_Left:
            cursorLeft(c);
            break;
        case Key_Right:
            cursorRight(c);
            break;
        case Key_Up:
            cursorUp(c);
            break;
        case Key_Down:
            cursorDown(c);
            break;
        case Key_Backspace:
            kWriteDoc->backspace(this,c);
            break;
        case Key_Home:
            home(c);
            break;
        case Key_End:
            end(c);
            break;
        case Key_Delete:
            if (e->state() & ShiftButton) kWriteDoc->cut(this,c);
            else if ((c.flags & cfDelOnInput) && kWriteDoc->hasMarkedText())
              kWriteDoc->delMarkedText(this,c);
            else kWriteDoc->del(this,c);
            break;
        case Key_Next:
            pageDown(c);
            break;
        case Key_Prior:
            pageUp(c);
            break;
        case Key_Enter:
        case Key_Return:
            kWriteDoc->newLine(this,c);
            //emit returnPressed();
            //e->ignore();
            break;
        case Key_Insert:
            if (e->state() & ShiftButton) kWriteDoc->paste(this,c);
              else kWrite->toggleOverwrite();
/*        case Key_F9:
            printf("text() %s\n", kWrite->text().data());
            break;
        case Key_F10:
            printf("currentWord() %s\n", kWrite->currentWord().data());
            break;*/
      }
    }
  }
  kWriteDoc->updateViews();
  e->accept();
}

void KWriteView::mousePressEvent(QMouseEvent *e) {

  if (e->button() == LeftButton) {
    int flags;

    flags = 0;
    if (e->state() & ShiftButton) {
      flags |= cfMark;
      if (e->state() & ControlButton) flags |= cfMark | cfKeepSelection;
    }
    placeCursor(e->x(),e->y(),flags);
    scrollX = 0;
    scrollY = 0;
    if (!scrollTimer) scrollTimer = startTimer(50);
    kWriteDoc->updateViews();
  }
  if (e->button() == MidButton) {
    placeCursor(e->x(),e->y(),0);
    kWrite->paste();
  }
//if (e->button() == RightButton) printf("word %s\n", kWrite->word(e->x(), e->y()).data());
  if (kWrite->popup && e->button() == RightButton) {
    kWrite->popup->popup(mapToGlobal(e->pos()));
  }
}

void KWriteView::mouseReleaseEvent(QMouseEvent *e) {

  if (e->button() == LeftButton) {
    kWrite->copy();
    killTimer(scrollTimer);
    scrollTimer = 0;
  }
}

void KWriteView::mouseMoveEvent(QMouseEvent *e) {

  if (e->state() & LeftButton) {
    int flags;
    int d;

    mouseX = e->x();
    mouseY = e->y();
    scrollX = 0;
    scrollY = 0;
    d = kWriteDoc->fontHeight;
    if (mouseX < 0) {
      mouseX = 0;
      scrollX = -d;
    }
    if (mouseX > width()) {
      mouseX = width();
      scrollX = d;
    }
    if (mouseY < 0) {
      mouseY = 0;
      scrollY = -d;
    }
    if (mouseY > height()) {
      mouseY = height();
      scrollY = d;
    }

    flags = cfMark;
    if (e->state() & ControlButton) flags |= cfKeepSelection;
    placeCursor(mouseX,mouseY,flags);
    kWriteDoc->updateViews(/*ufNoScroll*/);
  }
}

void KWriteView::paintEvent(QPaintEvent *e) {
  int xStart, xEnd;
  int h;
  int line, y, yEnd;

  QRect updateR = e->rect();
//  printf("update rect  = ( %i, %i, %i, %i )\n",
//    updateR.x(),updateR.y(), updateR.width(), updateR.height() );

  QPainter paint;
  paint.begin(drawBuffer);

  xStart = xPos-2 + updateR.x();
  xEnd = xStart + updateR.width();

  h = kWriteDoc->fontHeight;
  line = (yPos + updateR.y()) / h;
  y = line*h - yPos;
  yEnd = updateR.y() + updateR.height();

  while (y < yEnd) {
    kWriteDoc->paintTextLine(paint,line,xStart,xEnd);
//    if (cursorOn && line == cursor.y) paintCursor(paint,cXPos - xStart,h);
    bitBlt(this,updateR.x(),y,drawBuffer,0,0,updateR.width(),h);

    line++;
    y += h;
  }
  paint.end();
  if (cursorOn) paintCursor();
}

void KWriteView::resizeEvent(QResizeEvent *) {
//  printf("KWriteView::resize\n");
  resizeBuffer(this,width(),kWriteDoc->fontHeight);
  QWidget::update();
}

void KWriteView::timerEvent(QTimerEvent *e) {
  if (e->timerId() == cursorTimer) {
    cursorOn = !cursorOn;
    paintCursor();
  }
  if (e->timerId() == scrollTimer && (scrollX | scrollY)) {
    xScroll->setValue(xPos + scrollX);
    yScroll->setValue(yPos + scrollY);

    placeCursor(mouseX,mouseY,cfMark);
    kWriteDoc->updateViews(/*ufNoScroll*/);
  }
}

KWBookmark::KWBookmark() {
  cursor.y = -1;
}

KWrite::KWrite(KWriteDoc *doc, QWidget *parent, const char *name)
  : QWidget(parent, name) {
  kWriteDoc = doc;
  kWriteView = new KWriteView(this,doc);

  configFlags = 0;
  wrapAt = 78;
  searchFlags = 0;
  replacePrompt = 0L;
  kfm = 0L;
  popup = 0L;
  bookmarks.setAutoDelete(true);

  kWriteView->setFocus();
}

KWrite::~KWrite() {
  delete kWriteView;
  delete popup; //right mouse button popup
}


int KWrite::currentLine() {
  return kWriteView->cursor.y;
}

int KWrite::currentColumn() {
  return kWriteDoc->currentColumn(kWriteView->cursor);
}

void KWrite::setCursorPosition(int line, int col) {
  PointStruc cursor;

  cursor.x = col;
  cursor.y = line;
  kWriteView->updateCursor(cursor);
  kWriteDoc->unmarkFound();
  kWriteDoc->updateViews();
}

int KWrite::config() {
  return configFlags;
}

void KWrite::setConfig(int flags) {
  if (flags != configFlags) {
    configFlags = flags;
    emit newStatus();
  }
}
/*
bool KWrite::isOverwriteMode() {
  return (configFlags & cfOvr);
} */

bool KWrite::isModified() {
  return kWriteDoc->modified;//isModified();
}

void KWrite::setModified(bool m) {
  kWriteDoc->setModified(m);
}

bool KWrite::isLastView() {
  return kWriteDoc->isLastView(1);
}

KWriteDoc *KWrite::doc() {
  return kWriteDoc;
}

int KWrite::undoState() {
  return kWriteDoc->undoState;
}

void KWrite::copySettings(KWrite *w) {
  configFlags = w->configFlags;
  wrapAt = w->wrapAt;
  searchFlags = w->searchFlags;
}

void KWrite::optDlg() {
  SettingsDialog *dlg;

  dlg = new SettingsDialog(configFlags,wrapAt,kWriteDoc->tabChars,kWriteDoc->undoSteps,
    topLevelWidget());
  if (dlg->exec() == QDialog::Accepted) {
    setConfig(dlg->getFlags() | (configFlags & cfOvr));
    wrapAt = dlg->getWrapAt();
    kWriteDoc->setTabWidth(dlg->getTabWidth());
    kWriteDoc->setUndoSteps(dlg->getUndoSteps());
    kWriteDoc->updateViews();
  }
  delete dlg;
}

void KWrite::colDlg() {
  ColorDialog *dlg;

  dlg = new ColorDialog(kWriteDoc->colors,this);
  if (dlg->exec() == QDialog::Accepted) {
    dlg->getColors(kWriteDoc->colors);
    kWriteDoc->tagAll();
    kWriteDoc->updateViews();
  }
  delete dlg;
}

void KWrite::toggleVertical() {
  setConfig(configFlags ^ cfVerticalSelect);
  emit statusMsg(i18n(configFlags & cfVerticalSelect ? "Vertical Selections On" : "Vertical Selections Off"));
}

void KWrite::toggleOverwrite() {
  setConfig(configFlags ^ cfOvr);
}

QString KWrite::text() {
  return kWriteDoc->text();
}

QString KWrite::markedText() {
  return kWriteDoc->markedText(configFlags);
}

QString KWrite::currentWord() {
  return kWriteDoc->currentWord(kWriteView->cursor);
}

QString KWrite::word(int x, int y) {
  PointStruc cursor;
  cursor.y = (kWriteView->yPos + y)/kWriteDoc->fontHeight;
  if (cursor.y < 0 || cursor.y > kWriteDoc->lastLine()) return QString();
  cursor.x = kWriteDoc->textPos(kWriteDoc->textLine(cursor.y), kWriteView->xPos-2 + x);
  return kWriteDoc->currentWord(cursor);
}

void KWrite::setText(const char *s) {
  kWriteDoc->setText(s);
  kWriteDoc->updateViews();
}


void KWrite::loadFile(QIODevice &dev, bool insert) {
  VConfig c;

  if (!insert) {
    kWriteDoc->loadFile(dev);
  } else {
    kWriteView->getVConfig(c);
    kWriteDoc->insertFile(kWriteView,c,dev);
kWriteDoc->updateViews();
  }
//  kWriteDoc->updateViews();
}

void KWrite::writeFile(QIODevice &dev) {
  kWriteDoc->writeFile(dev);
  kWriteDoc->updateViews();
}


bool KWrite::loadFile(const char *name, int flags) {

  QFileInfo info(name);
  if (!info.exists()) {
    if (flags & lfNewFile) return true;
    QMessageBox::warning(this,
      i18n("Sorry"),
      i18n("The specified File does not exist"),
      i18n("OK"),
      "",
      "",
      0,0);
    return false;
  }
  if (info.isDir()) {
    QMessageBox::warning(this,
      i18n("Sorry"),
      i18n("You have specified a directory"),
      i18n("OK"),
      "",
      "",
      0,0);
    return false;
  }
  if (!info.isReadable()) {
    QMessageBox::warning(this,
      i18n("Sorry"),
      i18n("You do not have read permission to this file"),
      i18n("OK"),
      "",
      "",
      0,0);
    return false;
  }

  QFile f(name);
  if (f.open(IO_ReadOnly)) {
    loadFile(f,flags & lfInsert);
    f.close();
    return true;
  }
  QMessageBox::warning(this,
    i18n("Sorry"),
    i18n("An Error occured while trying to open this Document"),
    i18n("OK"),
    "",
    "",
    0,0);
  return false;
}

bool KWrite::writeFile(const char *name) {

  QFileInfo info(name);
  if(info.exists() && !info.isWritable()) {
    QMessageBox::warning(this,
      i18n("Sorry"),
      i18n("You do not have write permission to this file"),
      i18n("OK"),
      "",
      "",
      0,0);
    return false;
  }

  QFile f(name);
  if (f.open(IO_WriteOnly | IO_Truncate)) {
    writeFile(f);
    f.close();
    return true;//kWriteDoc->setFileName(name);
  }
  QMessageBox::warning(this,
    i18n("Sorry"),
    i18n("An Error occured while trying to open this Document"),
    i18n("OK"),
    "",
    "",
    0,0);
  return false;
}


void KWrite::loadURL(const char *url, int flags) {
  KURL u(url);

  if (u.isMalformed()) {
    QString s;
    if (url) {
      s = "file:";
      if (*url != '/') {
        s += QDir::currentDirPath();
        s += '/';
      }
      s += url;
      u.parse(s);
    }
    if (u.isMalformed()) {
      s.sprintf("%s\n%s",i18n("Malformed URL"),url);
      QMessageBox::warning(this,
        i18n("Sorry"),
        s,
        i18n("OK"),
        "",
        "",
        0,0);
      return;
    }
  }
  if (u.isLocalFile()) {
    // usual local file
    emit statusMsg(i18n("Loading..."));

    QString name(u.path());
    KURL::decodeURL(name);
    if (loadFile(name,flags)) {
      name = u.url();
      if (flags & lfInsert) {
        name.prepend(": ");
        name.prepend(i18n("Inserted"));
      } else {
        if (!(flags & lfNoAutoHl)) kWriteDoc->setFileName(name);
          else kWriteDoc->updateLines();
        name.prepend(": ");
        name.prepend(i18n("Read"));
      }
      emit statusMsg(name);
    }
  } else {
    // url
    if (kfm != 0L) {
      QMessageBox::information(this,
        i18n("Sorry"),
        i18n("KWrite is already waiting\nfor an internet job to finish\n"\
             "Please wait until it has finished\nAlternatively stop the running one."),
        i18n("OK"),
        "",
        "",
        0,0);
      return;
    }
    emit statusMsg(i18n("Waiting for KFM..."));

    kfm = new KFM;
    if (!kfm->isOK()) {
      QMessageBox::warning(this,
        i18n("Sorry"),
        i18n("Could not start or find KFM"),
        i18n("OK"),
        "",
        "",
        0,0);
      delete kfm;
      kfm = 0L;
      return;
    }

    kfmURL = u.url();
    kfmFile.sprintf(_PATH_TMP"kwrite%i",time(0L));
    kfmAction = KWrite::GET;
    kfmFlags = flags;

    connect(kfm,SIGNAL(finished()),this,SLOT(kfmFinished()));
    connect(kfm,SIGNAL(error(int, const char *)),this,SLOT(kfmError(int, const char *)));
    kfm->copy(url,kfmFile);
  }
}


void KWrite::writeURL(const char *url, int flags) {
  KURL u(url);

  if (u.isLocalFile()) {
    // usual local file
    emit statusMsg(i18n("Saving..."));

    QString name(u.path());
    KURL::decodeURL(name);
    if (writeFile(name)) {
      if (!(flags & lfNoAutoHl)) kWriteDoc->setFileName(url);
      name = url;
      name.prepend(": ");
      name.prepend(i18n("Wrote"));
      emit statusMsg(name);
      setModified(false);
    }
  } else {
    // url
    if (kfm != 0L) {
      QMessageBox::information(this,
        i18n("Sorry"),
        i18n("KWrite is already waiting\nfor an internet job to finish\n"\
             "Please wait until it has finished\nAlternatively stop the running one."),
        i18n("OK"),
        "",
        "",
        0,0);
      return;
    }
    emit statusMsg(i18n("Waiting for KFM..."));

    kfmURL = url;
    kfmFile.sprintf(_PATH_TMP"kwrite%i",time(0L));
    kfmAction = KWrite::PUT;
    kfmFlags = flags;
    if (!writeFile(kfmFile)) return;

    kfm = new KFM;

    if (!kfm->isOK()) {
      QMessageBox::warning(this,
        i18n("Sorry"),
        i18n("Could not start or find KFM"),
        i18n("OK"),
        "",
        "",
        0,0);
      delete kfm;
      kfm = 0L;
      return;
    }

    connect(kfm,SIGNAL(finished()),this,SLOT(kfmFinished()));
    connect(kfm,SIGNAL(error(int, const char *)),this,SLOT(kfmError(int, const char *)));
    kfm->copy(kfmFile,url);
  }
}

void KWrite::kfmFinished() {
//  QString string;
//  string.sprintf(klocale->translate("Finished '%s'"),tmpFile.data());
//  setGeneralStatusField(string);

  if (kfmAction == GET ) {
//    KURL u(kfmFile);
  //  if (!kfm->isOK()) printf("kfm not ok!!!\n");

    if (loadFile(kfmFile,kfmFlags)) {
      if (kfmFlags & lfInsert) {
        kfmURL.prepend(": ");
        kfmURL.prepend(i18n("Inserted"));
      } else {
        if (!(kfmFlags & lfNoAutoHl)) {
          kWriteDoc->setFileName(kfmURL);
        } else {
          kWriteDoc->updateLines();
          kWriteDoc->updateViews();
        }
        kfmURL.prepend(": ");
        kfmURL.prepend(i18n("Read"));
      }
      emit statusMsg(kfmURL);
    }
    //clean up
    unlink(kfmFile);
  }
  if (kfmAction == PUT) {
    if (!(kfmFlags & lfNoAutoHl)) kWriteDoc->setFileName(kfmURL);
    kfmURL.prepend(": ");
    kfmURL.prepend(i18n("Wrote"));
    emit statusMsg(kfmURL);
    setModified(false);
    //clean up
    unlink(kfmFile);
  }
  delete kfm;
  kfm = 0L;
}

void KWrite::kfmError(int e, const char *s) {
  printf("error %d = %s\n",e,s);
}


const char *KWrite::fileName() {
  return kWriteDoc->fileName();
}

void KWrite::setFileName(const char *s) {
  kWriteDoc->setFileName(s);
}

bool KWrite::canDiscard() {
  int query;

  if (isModified()) {
    query = QMessageBox::warning(this,
      i18n("Warning"),
      i18n("The current Document has been modified.\nWould you like to save it?"),
      i18n("Yes"),
      i18n("No"),
      i18n("Cancel"),
      0,2);
    switch (query) {
      case 0: //yes
        save();
        if (isModified()) {
            query = QMessageBox::warning(this,
            i18n("Sorry"),
            i18n("Could not save the document.\nOpen a new document anyways?"),
            i18n("Yes"),
            i18n("No"),
            "",
            0,1);
          if (query == 1) return false; //no
        }
        break;
      case 2: //cancel
        return false;
    }
  }
  return true;
}

void KWrite::newDoc() {

  if (canDiscard()) clear();
}

void KWrite::open() {
  QString url;    

  if (!canDiscard()) return;
//  if (kWriteDoc->hasFileName()) s = QFileInfo(kWriteDoc->fileName()).dirPath();
//    else s = QDir::currentDirPath();

  url = KFileDialog::getOpenFileURL(kWriteDoc->fileName(),"*",topLevelWidget());
  if (url.isEmpty()) return;
//  kapp->processEvents();
  loadURL(url);
}

void KWrite::insertFile() {
  QString url;

  url = KFileDialog::getOpenFileURL(kWriteDoc->fileName(),"*",topLevelWidget());
  if (url.isEmpty()) return;
//  kapp->processEvents();
  loadURL(url,lfInsert);
}

void KWrite::save() {
  if (isModified()) {
    if (kWriteDoc->hasFileName()) {
      writeURL(kWriteDoc->fileName(),lfNoAutoHl);
    } else saveAs();
  } else emit statusMsg(i18n("No changes need to be saved"));
}

void KWrite::saveAs() {
  QString url;
  int query;

  do {
    query = 0;
    url = KFileDialog::getSaveFileURL(kWriteDoc->fileName(),"*",this);
    if (url.isEmpty()) return;

    KURL u(url);
    if (u.isLocalFile()) {
      QFileInfo info;
      QString name(u.path());
      KURL::decodeURL(name);
      info.setFile(name);
      if (info.exists()) {
        query = QMessageBox::warning(this,
          i18n("Warning"),
          i18n("A Document with this Name already exists.\nDo you want to overwrite it?"),
          i18n("Yes"),
          i18n("No"),
          "",0,1);
      }
    }
  } while (query == 1);

//  kapp->processEvents();
  writeURL(url);
}



void KWrite::clear() {
  kWriteDoc->clear();
  kWriteDoc->setFileName(0);
  kWriteDoc->updateViews();
}

void KWrite::cut() {
  VConfig c;
  kWriteView->getVConfig(c);
  kWriteDoc->cut(kWriteView,c);
  kWriteDoc->updateViews();
}

void KWrite::copy() {
  kWriteDoc->copy(configFlags);
}

void KWrite::paste() {
  VConfig c;
  kWriteView->getVConfig(c);
  kWriteDoc->paste(kWriteView,c);
  kWriteDoc->updateViews();
}

void KWrite::undo() {
  kWriteDoc->undo(kWriteView,configFlags);
  kWriteDoc->updateViews();
}

void KWrite::redo() {
  kWriteDoc->redo(kWriteView,configFlags);
  kWriteDoc->updateViews();
}

void KWrite::indent() {
  VConfig c;
  kWriteView->getVConfig(c);
  kWriteDoc->indent(kWriteView,c);
  kWriteDoc->updateViews();
}

void KWrite::unIndent() {
  VConfig c;
  kWriteView->getVConfig(c);
  kWriteDoc->unIndent(kWriteView,c);
  kWriteDoc->updateViews();
}

void KWrite::selectAll() {
  kWriteDoc->selectAll();
  kWriteDoc->updateViews();
}

void KWrite::deselectAll() {
  kWriteDoc->deselectAll();
  kWriteDoc->updateViews();
}

void KWrite::invertSelection() {
  kWriteDoc->invertSelection();
  kWriteDoc->updateViews();
}



void KWrite::search() {
  SearchDialog *searchDialog;

  searchDialog = new SearchDialog(searchFor,0L,
    searchFlags & ~sfReplace,topLevelWidget());

  kWriteView->focusOutEvent(0L);// QT bug ?
  if (searchDialog->exec() == QDialog::Accepted) {
    searchFor = searchDialog->getSearchFor();
    searchFlags = searchDialog->getFlags() | (searchFlags & sfPrompt);
    initSearch(s,searchFlags);
    searchAgain(s);
  }
  delete searchDialog;
}

void KWrite::replace() {
  SearchDialog *searchDialog;

  searchDialog = new SearchDialog(searchFor,replaceWith,
    searchFlags | sfReplace,topLevelWidget());

  kWriteView->focusOutEvent(0L);// QT bug ?
  if (searchDialog->exec() == QDialog::Accepted) {
    searchFor = searchDialog->getSearchFor();
    replaceWith = searchDialog->getReplaceWith();
    searchFlags = searchDialog->getFlags();
    initSearch(s,searchFlags);
    replaceAgain();
  }
  delete searchDialog;
}

//usleep(50000);
//XSync(qt_xdisplay(),true);
//kapp->syncX();
//printf("xpending %d\n",XPending(qt_xdisplay()));
//kapp->processEvents();
//    kWriteView->tagAll();
//    searchAgain();

void KWrite::searchAgain() {

  initSearch(s,searchFlags | sfFromCursor | sfPrompt | sfAgain);
  if (s.flags & sfReplace) replaceAgain(); else searchAgain(s);
}

void KWrite::gotoLine() {
  GotoLineDialog *dlg;
  PointStruc cursor;

  dlg = new GotoLineDialog(kWriteView->cursor.y + 1,this);
  if (dlg->exec() == QDialog::Accepted) {
    cursor.x = 0;
    cursor.y = dlg->getLine() - 1;
    kWriteView->updateCursor(cursor);
    kWriteView->updateView(ufUpdateOnScroll);
  }
  delete dlg;
}


void KWrite::initSearch(SConfig &s, int flags) {

  s.flags = flags;
  if (s.flags & sfFromCursor) {
    s.cursor = kWriteView->cursor;
  } else {
    if (!(s.flags & sfBackward)) {
      s.cursor.x = 0;
      s.cursor.y = 0;
    } else {
      s.cursor.x = -1;
      s.cursor.y = kWriteDoc->lastLine();
    }
    s.flags |= sfFinished;
  }
  if (!(s.flags & sfBackward)) {
    if (!(s.cursor.x || s.cursor.y)) s.flags |= sfFinished;
  } else {
    s.startCursor.x -= searchFor.length();
  }
  s.startCursor = s.cursor;
}

void KWrite::continueSearch(SConfig &s) {

  if (!(s.flags & sfBackward)) {
    s.cursor.x = 0;
    s.cursor.y = 0;
  } else {
    s.cursor.x = -1;
    s.cursor.y = kWriteDoc->lastLine();
  }
  s.flags |= sfFinished;
  s.flags &= ~sfAgain;
}

void KWrite::searchAgain(SConfig &s) {
  int query;
  PointStruc cursor;
  int slen;
  QString str;

//  cursor = kWriteView->cursor;//getVConfig(c);
//  if (!(cursor.x || cursor.y || flags & sfBackward)) flags &= ~sfFromCursor;

  slen = searchFor.length();
  do {
    query = 1;
    if (kWriteDoc->doSearch(s,searchFor)) {
//    if (kWriteDoc->doSearch(cursor,searchFor,flags)) {
      //kWriteDoc->markFound(cursor,slen);
      cursor = s.cursor;
      if (!(s.flags & sfBackward)) s.cursor.x += slen;
      kWriteView->updateCursor(s.cursor);
      //kWriteDoc->updateViews(((flags & sfAgain) ? 0 : ufUpdateOnScroll) | ufCenter);
      exposeFound(cursor,slen,(s.flags & sfAgain) ? 0 : ufUpdateOnScroll,false);
    } else {
      if (!(s.flags & sfFinished)) {
        // ask for continue
        if (!(s.flags & sfBackward)) {
          // forward search
          str.sprintf("%s.\n%s?",
            i18n("End of document reached"),
            i18n("Continue from the beginning"));
          query = QMessageBox::information(this,
            i18n("Find"),
            str,
            i18n("Yes"),
            i18n("No"),
            "",0,1);
        } else {
          // backward search
          str.sprintf("%s.\n%s?",
            i18n("Beginning of document reached"),
            i18n("Continue from the end"));
          query = QMessageBox::information(this,
            i18n("Find"),
            str,
            i18n("Yes"),
            i18n("No"),
            "",0,1);
        }
        continueSearch(s);
      } else {
        // wrapped
        QMessageBox::information(this,
          i18n("Find"),
          i18n("Search string not found!"),
          i18n("OK"),
          "",
          "",0,0);
      }
    }
  } while (query == 0);
}

//void qt_enter_modal(QWidget *);


void KWrite::replaceAgain() {

  replaces = 0;
  if (s.flags & sfPrompt) {
    doReplaceAction(-1);
  } else {
    doReplaceAction(srAll);
  }
}

void KWrite::doReplaceAction(int result, bool found) {
  int slen, rlen;
  PointStruc cursor;
  bool started;

  slen = searchFor.length();
  rlen = replaceWith.length();

  switch (result) {
    case srYes: //yes
      kWriteDoc->recordStart(s.cursor,true);
      kWriteDoc->recordReplace(s.cursor,slen,replaceWith,rlen);
      replaces++;
      if (s.cursor.y == s.startCursor.y && s.cursor.x < s.startCursor.x)
        s.startCursor.x += rlen - slen;
      if (!(s.flags & sfBackward)) s.cursor.x += rlen;
      kWriteDoc->recordEnd(kWriteView,s.cursor,configFlags);
      break;
    case srNo: //no
      if (!(s.flags & sfBackward)) s.cursor.x += slen;
      break;
    case srAll: //replace all
//      deleteReplacePrompt();
      do {
        started = false;
        while (found || kWriteDoc->doSearch(s,searchFor)) {
          if (!started) {
            found = false;
            kWriteDoc->recordStart(s.cursor);
            started = true;
          }
          kWriteDoc->recordReplace(s.cursor,slen,replaceWith,rlen);
          replaces++;
          if (s.cursor.y == s.startCursor.y && s.cursor.x < s.startCursor.x)
            s.startCursor.x += rlen - slen;
          if (!(s.flags & sfBackward)) s.cursor.x += rlen;
        }
        if (started) kWriteDoc->recordEnd(kWriteView,s.cursor,configFlags);
      } while (!askReplaceEnd());
      return;
    case srCancel: //cancel
      deleteReplacePrompt();
      return;
    default:
      replacePrompt = 0L;
  }

  do {
    if (kWriteDoc->doSearch(s,searchFor)) {
      cursor = s.cursor;
      if (!(s.flags & sfBackward)) cursor.x += slen;
      kWriteView->updateCursor(cursor);
      exposeFound(s.cursor,slen,(s.flags & sfAgain) ? 0 : ufUpdateOnScroll,true);
      if (!replacePrompt) {
        replacePrompt = new ReplacePrompt(this);
        XSetTransientForHint(qt_xdisplay(),replacePrompt->winId(),topLevelWidget()->winId());
        kWriteDoc->setPseudoModal(replacePrompt);//disable();
        connect(replacePrompt,SIGNAL(clicked()),this,SLOT(replaceSlot()));
        replacePrompt->show();
      }
      return;
    }
  } while (!askReplaceEnd());
  deleteReplacePrompt();
}

void KWrite::exposeFound(PointStruc &cursor, int slen, int flags, bool replace) {
  TextLine *textLine;
  int x1, x2, y1, y2, xPos, yPos;

  kWriteDoc->markFound(cursor,slen);

//  kWriteDoc->updateViews();

  textLine = kWriteDoc->textLine(cursor.y);
  x1 = kWriteDoc->textWidth(textLine,cursor.x)        -10;
  x2 = kWriteDoc->textWidth(textLine,cursor.x + slen) +20;
  y1 = kWriteDoc->fontHeight*cursor.y                 -10;
  y2 = y1 + kWriteDoc->fontHeight                     +30;

  xPos = kWriteView->xPos;
  yPos = kWriteView->yPos;

//  w = kWriteView->width();
//  h = kWriteView->

  if (x1 < 0) x1 = 0;
  if (replace) y2 += 90;

  if (x1 < xPos || x2 > xPos + kWriteView->width()) {
    xPos = x2 - kWriteView->width();
//    xPos = e - 3*width()/4;
//    if (xPos > s) xPos = s;
  }
  if (y1 < yPos || y2 > yPos + kWriteView->height()) {
    xPos = x2 - kWriteView->width();
    yPos = kWriteDoc->fontHeight*cursor.y - height()/3;
  }
  kWriteView->updateView(flags | ufPos,xPos,yPos);
  kWriteDoc->updateViews(kWriteView);
}

void KWrite::deleteReplacePrompt() {
  kWriteDoc->setPseudoModal(0L);
}
/*
        if (!prompt) {
          prompt = new ReplacePrompt(topLevelWidget());
          query = prompt->exec();
        } else {
          qt_enter_modal(prompt);
          kapp->enter_loop();
          query = prompt->result();
        }
*/
//XSetTransientForHint(qt_xdisplay(), prompt->winId(), topLevelWidget()->winId());


bool KWrite::askReplaceEnd() {
  QString str;
  int query;

  kWriteDoc->updateViews();
  if (s.flags & sfFinished) {
    // replace finished
    str.sprintf("%d %s.",
      replaces,i18n("replace(s) made"));
    QMessageBox::information(this,
      i18n("Replace"),
      str,
      i18n("OK"),
      "",
      "",0,0);
    return true;
  }

  // ask for continue
  if (!(s.flags & sfBackward)) {
    // forward search
    str.sprintf("%d %s.\n%s.\n%s?",
      replaces,i18n("replace(s) made"),
      i18n("End of document reached"),
      i18n("Continue from the beginning"));
    query = QMessageBox::information(this,
      i18n("Replace"),
      str,
      i18n("Yes"),
      i18n("No"),
      "",0,1);
  } else {
    // backward search
    str.sprintf("%d %s.\n%s.\n%s?",
      replaces,i18n("replace(s) made"),
      i18n("Beginning of document reached"),
      i18n("Continue from the end"));
    query = QMessageBox::information(this,
      i18n("Replace"),
      str,
      i18n("Yes"),
      i18n("No"),
      "",0,1);
  }
  replaces = 0;
  continueSearch(s);
  return query;
}

void KWrite::replaceSlot() {
  doReplaceAction(replacePrompt->result(),true);
}


/*void KWrite::format() {
  dlg = new FormatDialog()
  if (dlg->exec() == QDialog::Accepted) {

  }
  delete dlg;
}            */

void KWrite::installRBPopup(QPopupMenu *p) {
  popup = p;
}

void KWrite::installBMPopup(QPopupMenu *p/*KWBookPopup *p*/) {
//  bookPopup = p;
  connect(p,SIGNAL(aboutToShow()),SLOT(updateBMPopup()));
  connect(p,SIGNAL(activated(int)),SLOT(gotoBookmark(int)));
  bmEntries = p->count();
}

void KWrite::setBookmark(int n) {
  KWBookmark *b;

  while ((int) bookmarks.count() <= n) bookmarks.append(new KWBookmark());
  b = bookmarks.at(n);
  b->xPos = kWriteView->xPos;
  b->yPos = kWriteView->yPos;
  b->cursor = kWriteView->cursor;
}

void KWrite::setBookmark() {
  QPopupMenu *popup;
  int z;
  char s[8];

  popup = new QPopupMenu(0L);

  for (z = 1; z <= 9; z++) {
    sprintf(s,"&%d",z);
    popup->insertItem(s,z);
  }

  popup->move(mapToGlobal(QPoint((width() - 41/*popup->width()*/)/2,
    (height() - 211/*popup->height()*/)/2)));

  z = popup->exec();
//  printf("map %d %d\n",popup->width(),popup->height());
  delete popup;
  if (z > 0) {
    setBookmark(z - 1);
  }
}

void KWrite::addBookmark() {
  int z;

  for (z = 0; z < (int) bookmarks.count(); z++) {
    if (bookmarks.at(z)->cursor.y == -1) break;
  }
  setBookmark(z);
}

void KWrite::gotoBookmark(int n) {
  KWBookmark *b;
//  printf("KWrite::gotoBookmark %d\n",n);

  if (n < 0 || n >= (int) bookmarks.count()) return;
  b = bookmarks.at(n);
  if (b->cursor.y == -1) return;
  kWriteView->updateCursor(b->cursor);
  kWriteDoc->unmarkFound();
  kWriteView->updateView(ufPos,b->xPos,b->yPos);
  kWriteDoc->updateViews(kWriteView);
}

void KWrite::clearBookmarks() {
  bookmarks.clear();
}

void KWrite::updateBMPopup() {
  QPopupMenu *p;
  KWBookmark *b;
  char buf[64];
  int z;

  p = (QPopupMenu *) sender();

  while ((int) p->count() > bmEntries) {
    p->removeItemAt(p->count() - 1);
  }

  for (z = 0; z < (int) bookmarks.count(); z++) {
    b = bookmarks.at(z);
//  for (b = bookmarks.first(); b != 0L; b = bookmarks.next()) {
    if (b->cursor.y >= 0) {
      if ((int) p->count() == bmEntries) p->insertSeparator();
      sprintf(buf,"%s %d",i18n("Line"),b->cursor.y +1);
      p->insertItem(buf,z);
      if (z < 9) p->setAccel(ALT+keys[z],z);
    }
  }
}


void KWrite::readConfig(KConfig *config) {

  searchFlags = config->readNumEntry("SearchFlags",sfPrompt);
  configFlags = config->readNumEntry("ConfigFlags",
    cfPersistent | cfRemoveSpaces) & ~cfMark;
  wrapAt = config->readNumEntry("WrapAt",78);
/*
  int flags;

  config->setGroup("Search Options");
  flags = 0;
  if (config->readNumEntry("CaseSensitive")) flags |= sfCaseSensitive;
  if (config->readNumEntry("WholeWordsOnly")) flags |= sfWholeWords;
  if (config->readNumEntry("FromCursor")) flags |= sfFromCursor;
  if (config->readNumEntry("FindBackwards")) flags |= sfBackward;
  if (config->readNumEntry("SelectedText")) flags |= sfSelected;
  if (config->readNumEntry("PromptOnReplace",1)) flags |= sfPrompt;
  searchFlags = flags;

  config->setGroup("Settings");
  flags = 0;
  if (config->readNumEntry("AutoIndent")) flags |= cfAutoIndent;
  if (config->readNumEntry("BackspaceIndent")) flags |= cfBackspaceIndent;
  if (config->readNumEntry("WordWrap")) flags |= cfWordWrap;
  if (config->readNumEntry("ReplaceTabs")) flags |= cfReplaceTabs;
  if (config->readNumEntry("RemoveTrailingSpaces",1)) flags |= cfRemoveSpaces;
  if (config->readNumEntry("WrapCursor")) flags |= cfWrapCursor;
  if (config->readNumEntry("AutoBrackets")) flags |= cfAutoBrackets;
  if (config->readNumEntry("PersistentSelections",1)) flags |= cfPersistent;
  if (config->readNumEntry("MultipleSelections")) flags |= cfKeepSelection;
  if (config->readNumEntry("VerticalSelections")) flags |= cfVerticalSelect;
  if (config->readNumEntry("DeleteOnInput")) flags |= cfDelOnInput;
  if (config->readNumEntry("ToggleOld")) flags |= cfXorSelect;
  configFlags = flags;

  wrapAt = config->readNumEntry("WrapAt",78);
  kWriteDoc->setTabWidth(config->readNumEntry("TabWidth",8));
  kWriteDoc->setUndoSteps(config->readNumEntry("UndoSteps",50));
*/
}

void KWrite::writeConfig(KConfig *config) {

  config->writeEntry("SearchFlags",searchFlags);
  config->writeEntry("ConfigFlags",configFlags);
  config->writeEntry("WrapAt",wrapAt);

/*
  int flags;

  config->setGroup("Search Options");
  flags = searchFlags;
  config->writeEntry("CaseSensitive",(flags & sfCaseSensitive) != 0);
  config->writeEntry("WholeWordsOnly",(flags & sfWholeWords) != 0);
  config->writeEntry("FromCursor",(flags & sfFromCursor) != 0);
  config->writeEntry("FindBackwards",(flags & sfBackward) != 0);
  config->writeEntry("SelectedText",(flags & sfSelected) != 0);
  config->writeEntry("PromptOnReplace",(flags & sfPrompt) != 0);

  config->setGroup("Settings");
  flags = configFlags;
  config->writeEntry("AutoIndent",(flags & cfAutoIndent) != 0);
  config->writeEntry("BackspaceIndent",(flags & cfBackspaceIndent) != 0);
  config->writeEntry("WordWrap",(flags & cfWordWrap) != 0);
  config->writeEntry("ReplaceTabs",(flags & cfReplaceTabs) != 0);
  config->writeEntry("RemoveTrailingSpaces",(flags & cfRemoveSpaces) != 0);
  config->writeEntry("WrapCursor",(flags & cfWrapCursor) != 0);
  config->writeEntry("AutoBrackets",(flags & cfAutoBrackets) != 0);
  config->writeEntry("PersistentSelections",(flags & cfPersistent) != 0);
  config->writeEntry("MultipleSelections",(flags & cfKeepSelection) != 0);
  config->writeEntry("VerticalSelections",(flags & cfVerticalSelect) != 0);
  config->writeEntry("DeleteOnInput",(flags & cfDelOnInput) != 0);
  config->writeEntry("ToggleOld",(flags & cfXorSelect) != 0);

  config->writeEntry("WrapAt",wrapAt);
  config->writeEntry("TabWidth",kWriteDoc->tabChars);
  config->writeEntry("UndoSteps",kWriteDoc->undoSteps);
*/
}

void KWrite::readSessionConfig(KConfig *config) {
  PointStruc cursor;
  int count, z;
  char s1[16];
  QString s2;
  KWBookmark *b;

/*
  searchFlags = config->readNumEntry("SearchFlags",sfPrompt);
  configFlags = config->readNumEntry("ConfigFlags");
  wrapAt = config->readNumEntry("WrapAt",78);
*/
  readConfig(config);

  kWriteView->xPos = config->readNumEntry("XPos");
  kWriteView->yPos = config->readNumEntry("YPos");
  cursor.x = config->readNumEntry("CursorX");
  cursor.y = config->readNumEntry("CursorY");
  kWriteView->updateCursor(cursor);

  count = config->readNumEntry("Bookmarks");
  for (z = 0; z < count; z++) {
    b = new KWBookmark();
    bookmarks.append(b);
    sprintf(s1,"Bookmark%d",z+1);
    s2 = config->readEntry(s1);
    if (!s2.isEmpty()) {
      sscanf(s2,"%d,%d,%d,%d",&b->xPos,&b->yPos,&b->cursor.x,&b->cursor.y);
    }
  }
}

void KWrite::writeSessionConfig(KConfig *config) {
  int z;
  char s1[16];
  char s2[64];
  KWBookmark *b;

/*
  config->writeEntry("SearchFlags",searchFlags);
  config->writeEntry("ConfigFlags",configFlags);
  config->writeEntry("WrapAt",wrapAt);
*/
  writeConfig(config);

  config->writeEntry("XPos",kWriteView->xPos);
  config->writeEntry("YPos",kWriteView->yPos);
  config->writeEntry("CursorX",kWriteView->cursor.x);
  config->writeEntry("CursorY",kWriteView->cursor.y);

  config->writeEntry("Bookmarks",bookmarks.count());
  for (z = 0; z < (int) bookmarks.count(); z++) {
    b = bookmarks.at(z);
    if (b->cursor.y != -1) {
      sprintf(s1,"Bookmark%d",z+1);
      sprintf(s2,"%d,%d,%d,%d",b->xPos,b->yPos,b->cursor.x,b->cursor.y);
      config->writeEntry(s1,s2);
    }
  }
}

/*
void KWrite::setHighlight(Highlight *hl) {
  if (hl) {
    kWriteDoc->setHighlight(hl);
    kWriteDoc->updateViews();
  }
}
*/

void KWrite::hlDef() {
  DefaultsDialog *dlg;
  HlManager *hlManager;
  ItemStyleList defaultStyleList;
  ItemFont defaultFont;
//  int count, z;

  hlManager = kWriteDoc->hlManager;
  defaultStyleList.setAutoDelete(true);

  hlManager->getDefaults(defaultStyleList,defaultFont);
/*
  defItemStyleList = kWriteDoc->defItemStyleList;
  count = defItemStyleList->count();
  for (z = 0; z < count ; z++) {
    itemStyleList.append(new ItemStyle(*defItemStyleList->at(z)));
  }
  */
  dlg = new DefaultsDialog(hlManager,&defaultStyleList,&defaultFont,this);
  if (dlg->exec() == QDialog::Accepted) {
    hlManager->setDefaults(defaultStyleList,defaultFont);
/*    for (z = 0; z < count; z++) {
      defItemStyleList->at(z)->setData(*itemStyleList.at(z));
    }
    kWriteDoc->defFont->setData(defFont);*/
  }
  delete dlg;
}

void KWrite::hlDlg() {
  HighlightDialog *dlg;
  HlManager *hlManager;
  HlDataList hlDataList;
//  int count, z;

  hlManager = kWriteDoc->hlManager;
  hlDataList.setAutoDelete(true);
  hlManager->getHlDataList(hlDataList);
  dlg = new HighlightDialog(hlManager,&hlDataList,kWriteDoc->getHighlight(),this);
//  dlg->hlChanged(kWriteDoc->getHighlight());
  if (dlg->exec() == QDialog::Accepted) {
    hlManager->setHlDataList(hlDataList);
  }
  delete dlg;
}

void KWrite::setHl(int n) {
  kWriteDoc->setHighlight(n);
  kWriteDoc->updateViews();
}

void KWrite::keyPressEvent(QKeyEvent *e) {
  int z;

  if (e->state() & AltButton) {
    for (z = 0; z < 9; z++) {
      if (keys[z] == e->key()) gotoBookmark(z);
    }
  }
}

void KWrite::paintEvent(QPaintEvent *e) {
  int x, y;

  QRect updateR = e->rect();                    // update rectangle
//  printf("Update rect = ( %i, %i, %i, %i )\n",
//    updateR.x(),updateR.y(), updateR.width(), updateR.height() );

  int ux1 = updateR.x();
  int uy1 = updateR.y();
  int ux2 = ux1 + updateR.width();
  int uy2 = uy1 + updateR.height();

  QPainter paint;
  paint.begin(this);

  QColorGroup g = colorGroup();
  x = width();
  y = height();

  paint.setPen(g.dark());
  if (uy1 <= 0) paint.drawLine(0,0,x-2,0);
  if (ux1 <= 0) paint.drawLine(0,1,0,y-2);

  paint.setPen(black);
  if (uy1 <= 1) paint.drawLine(1,1,x-3,1);
  if (ux1 <= 1) paint.drawLine(1,2,1,y-3);

  paint.setPen(g.midlight());
  if (uy2 >= y-1) paint.drawLine(1,y-2,x-3,y-2);
  if (ux2 >= x-1) paint.drawLine(x-2,1,x-2,y-2);

  paint.setPen(g.light());
  if (uy2 >= y) paint.drawLine(0,y-1,x-2,y-1);
  if (ux2 >= x) paint.drawLine(x-1,0,x-1,y-1);

  x -= 2 + 16;
  y -= 2 + 16;
  if (ux2 > x && uy2 > y) {
    paint.fillRect(x,y,16,16,g.background());
  }
  paint.end();
}

void KWrite::resizeEvent(QResizeEvent *) {

//  printf("Resize %d, %d\n",e->size().width(),e->size().height());

  kWriteView->tagAll();//updateState = 3;
  kWriteView->updateView(0/*ufNoScroll*/);
}



