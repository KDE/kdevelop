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

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <qstring.h>
#include <qwidget.h>
#include <qfont.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qdir.h>
#include <qprinter.h>
#include <qprintdialog.h>
#include <qpaintdevicemetrics.h>
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
/*
int keys[] = {Qt::Key_1,Qt::Key_2,Qt::Key_3,Qt::Key_4,Qt::Key_5,
	      Qt::Key_6,Qt::Key_7,Qt::Key_8,Qt::Key_9};
*/
//int keys[] = {Key_1,Key_2,Key_3,Key_4,Key_5,
//	      Key_6,Key_7,Key_8,Key_9};

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

  startLine = 0;
  endLine = -1;

  exposeCursor = false;
  updateState = 0;
  numLines = 0;
  lineRanges = 0L;
  newXPos = -1;
  newYPos = -1;

  drawBuffer = getBuffer(this);

  bm.sXPos = 0;
  bm.eXPos = -1;

  doc->registerView(this);
}

KWriteView::~KWriteView() {
  kWriteDoc->removeView(this);
  delete [] lineRanges;
  releaseBuffer(this);
}


void KWriteView::doCursorCommand(VConfig &c, int cmdNum) {

  switch (cmdNum) {
    case cmLeft:
      cursorLeft(c);
      break;
    case cmRight:
      cursorRight(c);
      break;
    case cmWordLeft:
      wordLeft(c);
      break;
    case cmWordRight:
      wordRight(c);
      break;
    case cmHome:
      home(c);
      break;
    case cmEnd:
      end(c);
      break;
    case cmUp:
      cursorUp(c);
      break;
    case cmDown:
      cursorDown(c);
      break;
    case cmScrollUp:
      scrollUp(c);
      break;
    case cmScrollDown:
      scrollDown(c);
      break;
    case cmTopOfView:
      topOfView(c);
      break;
    case cmBottomOfView:
      bottomOfView(c);
      break;
    case cmPageUp:
      pageUp(c);
      break;
    case cmPageDown:
      pageDown(c);
      break;
    case cmCursorPageUp:
      cursorPageUp(c);
      break;
    case cmCursorPageDown:
      cursorPageDown(c);
      break;
    case cmTop:
      top(c);
      break;
    case cmBottom:
      bottom(c);
      break;
  }
}

void KWriteView::doEditCommand(VConfig &c, int cmdNum) {

  switch (cmdNum) {
    case cmReturn:
      if ((c.flags & cfDelOnInput) && kWriteDoc->hasMarkedText())
        kWriteDoc->delMarkedText(c);
      kWriteDoc->newLine(c);
      //emit returnPressed();
      //e->ignore();
      break;
    case cmDelete:
      if ((c.flags & cfDelOnInput) && kWriteDoc->hasMarkedText())
        kWriteDoc->delMarkedText(c);
      else kWriteDoc->del(c);
      break;
    case cmBackspace:
      if ((c.flags & cfDelOnInput) && kWriteDoc->hasMarkedText())
        kWriteDoc->delMarkedText(c);
      else kWriteDoc->backspace(c);
      break;
    case cmKillLine:
      kWriteDoc->killLine(c);
      break;
    case cmCut:
      kWriteDoc->cut(c);
      break;
    case cmCopy:
      kWriteDoc->copy(c.flags);
      break;
    case cmPaste:
      if (c.flags & cfDelOnInput) kWriteDoc->delMarkedText(c);
      kWriteDoc->paste(c);
      break;
/*    case cmFind:
      break;
    case cmReplace:
      break;
    case cmFindAgain:
      break;
    case cmGotoLine:
      break;     */
    case cmUndo:
      kWriteDoc->undo(c);
      break;
    case cmRedo:
      kWriteDoc->redo(c);
      break;
    case cmIndent:
      kWriteDoc->indent(c);
      break;
    case cmUnindent:
      kWriteDoc->unIndent(c);
      break;
    case cmSelectAll:
      kWriteDoc->selectAll();
      break;
    case cmDeselectAll:
      kWriteDoc->deselectAll();
      break;
    case cmInvertSelection:
      kWriteDoc->invertSelection();
      break;
  }
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

void KWriteView::wordLeft(VConfig &c) {

  kWriteDoc->wordLeft(cursor);
  cOldXPos = cXPos = kWriteDoc->textWidth(cursor);
  update(c);
}

void KWriteView::wordRight(VConfig &c) {

  kWriteDoc->wordRight(cursor);
  cOldXPos = cXPos = kWriteDoc->textWidth(cursor);
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

void KWriteView::scrollUp(VConfig &c) {

  newYPos = yPos - kWriteDoc->fontHeight;
  if (newYPos < 0) newYPos = 0;
  if (cursor.y == (yPos + height())/kWriteDoc->fontHeight -1) {
    cursor.y--;
    cXPos = kWriteDoc->textWidth(c.flags & cfWrapCursor,cursor,cOldXPos);
  }
  update(c);
}

void KWriteView::scrollDown(VConfig &c) {

  newYPos = yPos + kWriteDoc->fontHeight;
  if (cursor.y == (yPos + kWriteDoc->fontHeight -1)/kWriteDoc->fontHeight) {
    cursor.y++;
    cXPos = kWriteDoc->textWidth(c.flags & cfWrapCursor,cursor,cOldXPos);
  }
  update(c);
}

void KWriteView::topOfView(VConfig &c) {

  cursor.y = (yPos + kWriteDoc->fontHeight -1)/kWriteDoc->fontHeight;
  cursor.x = 0;
  cOldXPos = cXPos = 0;
  update(c);
}

void KWriteView::bottomOfView(VConfig &c) {

  cursor.y = (yPos + height())/kWriteDoc->fontHeight -1;
  if (cursor.y < 0) cursor.y = 0;
  cursor.x = 0;
  cOldXPos = cXPos = 0;
  update(c);
}

void KWriteView::pageUp(VConfig &c) {

  if (yPos > 0) {
    newYPos = yPos - (endLine - startLine)*kWriteDoc->fontHeight;
    if (newYPos < 0) newYPos = 0;
  }
  cursorPageUp(c);
}

void KWriteView::pageDown(VConfig &c) {

  if (endLine < kWriteDoc->lastLine()) {
    newYPos = yPos + (endLine - startLine)*kWriteDoc->fontHeight;
  }
  cursorPageDown(c);
}

void KWriteView::cursorPageUp(VConfig &c) {

  cursor.y -= endLine - startLine;
  cXPos = kWriteDoc->textWidth(c.flags & cfWrapCursor,cursor,cOldXPos);
  update(c);
}

void KWriteView::cursorPageDown(VConfig &c) {

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
  if (QABS(dx) < width()) scroll(dx, 0); else QWidget::update();
}

void KWriteView::changeYPos(int p) {
  int dy;

  dy = yPos - p;
  yPos = p;
  lineValues(height());
  if (QABS(dy) < height()) scroll(0, dy); else QWidget::update();
}


void KWriteView::getVConfig(VConfig &c) {

  c.view = this;
  c.cursor = cursor;
  c.cXPos = cXPos;
  c.flags = kWrite->configFlags;
  c.wrapAt = kWrite->wrapAt;
}



void KWriteView::update(VConfig &c) {

  if (cursor.x == c.cursor.x && cursor.y == c.cursor.y) return;
  kWriteDoc->unmarkFound();
  
  exposeCursor = true;
  if (cursorOn) {
//    tagLines(c.cursor.y, c.cursor.y, c.cXPos -2, c.cXPos +3);
    tagLines(c.cursor.y, c.cursor.y, 0, 0xffffff);
    cursorOn = false;
  }

  if (bm.sXPos < bm.eXPos) {
    tagLines(bm.cursor.y, bm.cursor.y, bm.sXPos, bm.eXPos);
  }
  kWriteDoc->newBracketMark(cursor, bm);

  if (c.flags & cfMark) {
    kWriteDoc->selectTo(c, cursor, cXPos);
  } else {
    if (!(c.flags & cfPersistent)) kWriteDoc->deselectAll();
  }
}

void KWriteView::insLine(int line) {

  if (line <= cursor.y) {
    cursor.y++;
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
    tagLines(cursor.y, cursor.y, cXPos -2, cXPos +3);
    cursorOn = false;
  }

  if (bm.sXPos < bm.eXPos) {
    tagLines(bm.cursor.y, bm.cursor.y, bm.sXPos, bm.eXPos);
  }
  kWriteDoc->newBracketMark(newCursor, bm);

  cursor = newCursor;
  cOldXPos = cXPos = kWriteDoc->textWidth(cursor);
}


void KWriteView::lineValues(int h) {
  int lines, z;

  startLine = yPos/kWriteDoc->fontHeight;
  endLine = (yPos + h -1)/kWriteDoc->fontHeight;

  updateState = 0;

  lines = endLine - startLine +1;
  if (lines > numLines) {
    numLines = lines*2;
    delete [] lineRanges;
    lineRanges = new LineRange[numLines];
  }

  for (z = 0; z < lines; z++) {
    lineRanges[z].start = 0xffffff;
    lineRanges[z].end = -2;
  }
  newXPos = newYPos = -1;
}

void KWriteView::tagLines(int start, int end, int x1, int x2) {
  LineRange *r;
  int z;

  start -= startLine;
  if (start < 0) start = 0;
  end -= startLine;
  if (end > endLine - startLine) end = endLine - startLine;

  if (x1 == 0) x1 = -2;
  if (x1 < xPos-2) x1 = xPos-2;
  if (x2 > width() + xPos-2) x2 = width() + xPos-2;
  if (x1 >= x2) return;

  r = &lineRanges[start];
  for (z = start; z <= end; z++) {
    if (x1 < r->start) r->start = x1;
    if (x2 > r->end) r->end = x2;
    r++;
    updateState |= 1;
  }
}

/*
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
*/
void KWriteView::tagAll() {
  updateState = 3;
}
/*
void KWriteView::clearTags() {
  int z;

  for (z = 0; z < numLines; z++) {
    lineRanges[z].start = 0xffffff;
    lineRanges[z].end = 0;
  }
}
*/
void KWriteView::setPos(int x, int y) {
  newXPos = x;
  newYPos = y;
}

void KWriteView::center() {
  newXPos = 0;
  newYPos = cursor.y*kWriteDoc->fontHeight - height()/2;
  if (newYPos < 0) newYPos = 0;
}

void KWriteView::updateView(int flags) {
  int fontHeight;
  int oldXPos, oldYPos;
  int w, h;
  int z;
  bool b;
  int xMax, yMax;
  int cYPos;
  int cXPosMin, cXPosMax, cYPosMin, cYPosMax;
  int dx, dy;

//printf("upView %d %d %d %d %d\n", exposeCursor, updateState, flags, newXPos, newYPos);
  if (exposeCursor || flags & ufDocGeometry) {
    emit kWrite->newCurPos();
  } else {
    if (updateState == 0 && newXPos < 0 && newYPos < 0) return;
  }

  if (cursorTimer) {
    killTimer(cursorTimer);
    cursorTimer = startTimer(500);
    cursorOn = true;
  }

  oldXPos = xPos;
  oldYPos = yPos;
/*  if (flags & ufPos) {
    xPos = newXPos;
    yPos = newYPos;
    exposeCursor = true;
  }*/
  if (newXPos >= 0) xPos = newXPos;
  if (newYPos >= 0) yPos = newYPos;

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

  //startLine = yPos / fontHeight;
  //endLine = (yPos + h -1) / fontHeight;

  if (w != width() || h != height()) {
    lineValues(h);
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
      lineValues(h);
      QWidget::update();
    } else {
      if (updateState > 0) paintTextLines(oldXPos, oldYPos);
      lineValues(h);

      if (dx || dy) {
        scroll(dx,dy);
//        kapp->syncX();
//        scroll2(dx - dx/2,dy - dy/2);
      } else {
        if (cursorOn) paintCursor();
        if (bm.eXPos > bm.sXPos) paintBracketMark();
      }
    }
  }
  exposeCursor = false;
//  updateState = 0;
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




void KWriteView::paintTextLines(int xPos, int yPos) {
//  int xStart, xEnd;
  int line;//, z;
  int h;
  LineRange *r;

  QPainter paint;
  paint.begin(drawBuffer);

  h = kWriteDoc->fontHeight;
  r = lineRanges;
  for (line = startLine; line <= endLine; line++) {
    if (r->start < r->end) {
//printf("painttextline %d %d %d\n", line, r->start, r->end);
      kWriteDoc->paintTextLine(paint, line, r->start, r->end);
      bitBlt(this, r->start - (xPos-2), line*h - yPos, drawBuffer, 0, 0,
        r->end - r->start, h);
    }
    r++;
  }
/*
  xStart = xPos-2;
  xEnd = xStart + width();
  h = kWriteDoc->fontHeight;
  for (z = 0; z < updateState; z++) {
    line = updateLines[z];
    kWriteDoc->paintTextLine(paint,line,xStart,xEnd);
    bitBlt(this,0,line*h - yPos,drawBuffer,0,0,width(),h);
  }*/
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

void KWriteView::paintBracketMark() {
  int y;

  y = kWriteDoc->fontHeight*(bm.cursor.y +1) - yPos -1;

  QPainter paint;
  paint.begin(this);
  paint.setPen(kWriteDoc->cursorCol(bm.cursor.x, bm.cursor.y));

  paint.drawLine(bm.sXPos - (xPos-2), y, bm.eXPos - (xPos-2) -1, y);
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
//  printf("lost focus %d\n", cursorTimer);

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
//  int keyCode;
  int ascii;

/*  if (e->state() & AltButton) {
    e->ignore();
    return;
  }*/
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
/*
  keyCode = e->key();
  if (e->state() & ShiftButton) keyCode |= SHIFT;
  if (e->state() & ControlButton) keyCode |= CTRL;*/
  ascii = e->ascii();
/*
  if (keyCode == Key_Enter || keyCode == Key_Return) {
    doEditCommand(c, cmReturn);
  } else if (keyCode == Key_Delete) {
    doEditCommand(c, cmDelete);
  } else if (keyCode == Key_Backspace) {
    doEditCommand(c, cmBackspace);*/
  if ((ascii >= 32 || ascii == '\t') && ascii != 127) {
    if (c.flags & cfDelOnInput) {
      kWriteDoc->delMarkedText(c);
      getVConfig(c);
    }
    kWriteDoc->insertChar(c, ascii);
    kWriteDoc->updateViews();
    e->accept();
  } else {
    e->ignore();
    //when not used with KGuiCommand
    //command constants cmXxx are equal to default keycodes
/*    doEditCommand(c, keyCode);
    if (e->state() & ShiftButton) {
      keyCode &= ~(SHIFT | CTRL);
      c.flags |= cfMark;
      if (e->state() & ControlButton) c.flags |= cfKeepSelection;
    }
    doCursorCommand(c, keyCode);*/
  }
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
  if (kWrite->popup && e->button() == RightButton) {
    kWrite->popup->popup(mapToGlobal(e->pos()));
  }
  kWrite->mousePressEvent(e);
}

void KWriteView::mouseDoubleClickEvent(QMouseEvent *e) {
  if (e->button() == LeftButton) {
    VConfig c;
    getVConfig(c);
    kWriteDoc->selectWord(c.cursor, c.flags);
    kWriteDoc->updateViews();
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
//printf("modifiers %d\n", ((KGuiCmdApp *) kapp)->getModifiers());
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
  if (bm.eXPos > bm.sXPos) paintBracketMark();
}

void KWriteView::resizeEvent(QResizeEvent *) {
//  printf("KWriteView::resize\n");
  resizeBuffer(this, width(), kWriteDoc->fontHeight);
//  QWidget::update();

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
  wrapAt = 79;
  searchFlags = 0;
  replacePrompt = 0L;
  kfm = 0L;
  popup = 0L;
  bookmarks.setAutoDelete(true);

  kWriteView->setFocus();
  resize(parent->width() -4, parent->height() -4);
}

KWrite::~KWrite() {
  delete kWriteView;
  delete popup; //right mouse button popup
}


void KWrite::addCursorCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctCursorCommands, "Cursor Movement");
  cmdMngr.setSelectModifiers(SHIFT, selectFlag, ALT, multiSelectFlag);
  cmdMngr.addCommand(cmLeft,            "Left"            , Key_Left, CTRL+Key_B);
  cmdMngr.addCommand(cmRight,           "Right"           , Key_Right, CTRL+Key_F);
  cmdMngr.addCommand(cmWordLeft,        "Word Left"       , CTRL+Key_Left);
  cmdMngr.addCommand(cmWordRight,       "Word Right"      , CTRL+Key_Right);
  cmdMngr.addCommand(cmHome,            "Home"            , Key_Home, CTRL+Key_A);
  cmdMngr.addCommand(cmEnd,             "End"             , Key_End, CTRL+Key_E);
  cmdMngr.addCommand(cmUp,              "Up"              , Key_Up, CTRL+Key_P);
  cmdMngr.addCommand(cmDown,            "Down"            , Key_Down, CTRL+Key_N);
  cmdMngr.addCommand(cmScrollUp,        "Scroll Up"       , CTRL+Key_Up);
  cmdMngr.addCommand(cmScrollDown,      "Scroll Down"     , CTRL+Key_Down);
  cmdMngr.addCommand(cmTopOfView,       "Top Of View"     , CTRL+Key_PageUp);
  cmdMngr.addCommand(cmBottomOfView,    "Bottom Of View"  , CTRL+Key_PageDown);
  cmdMngr.addCommand(cmPageUp,          "Page Up"         , Key_PageUp);
  cmdMngr.addCommand(cmPageDown,        "Page Down"       , Key_PageDown);
  cmdMngr.addCommand(cmCursorPageUp,    "Cursor Page Up"  );
  cmdMngr.addCommand(cmCursorPageDown,  "Cursor Page Down");
  cmdMngr.addCommand(cmTop,             "Top"             , CTRL+Key_Home);
  cmdMngr.addCommand(cmBottom,          "Bottom"          , CTRL+Key_End);
}

void KWrite::addEditCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctEditCommands, "Edit Commands");
  cmdMngr.addCommand(cmReturn,          "Return"           , Key_Return, Key_Enter);
  cmdMngr.addCommand(cmDelete,          "Delete"           , Key_Delete, CTRL+Key_D);
  cmdMngr.addCommand(cmBackspace,       "Backspace"        , Key_Backspace, CTRL+Key_H);
  cmdMngr.addCommand(cmKillLine,        "Kill Line"        , CTRL+Key_K);
  cmdMngr.addCommand(cmUndo,            "U&ndo"            , CTRL+Key_Z);
  cmdMngr.addCommand(cmRedo,            "R&edo"            , CTRL+Key_Y);
  cmdMngr.addCommand(cmCut,             "C&ut"             , CTRL+Key_X, SHIFT+Key_Delete);
  cmdMngr.addCommand(cmCopy,            "&Copy"            , CTRL+Key_C, CTRL+Key_Insert);
  cmdMngr.addCommand(cmPaste,           "&Paste"           , CTRL+Key_V, SHIFT+Key_Insert);
  cmdMngr.addCommand(cmIndent,          "&Indent"          , CTRL+Key_I);
  cmdMngr.addCommand(cmUnindent,        "Uninden&t"        , CTRL+Key_U);
  cmdMngr.addCommand(cmSelectAll,       "&Select All"      );
  cmdMngr.addCommand(cmDeselectAll,     "&Deselect All"    );
  cmdMngr.addCommand(cmInvertSelection, "In&vert Selection");
}

void KWrite::addFindCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctFindCommands, "Find Commands");
  cmdMngr.addCommand(cmFind,            "&Find..."         , CTRL+Key_F);
  cmdMngr.addCommand(cmReplace,         "&Replace..."      , CTRL+Key_R);
  cmdMngr.addCommand(cmFindAgain,       "Find &Again"      , Key_F3);
  cmdMngr.addCommand(cmGotoLine,        "&Goto Line..."    , CTRL+Key_G);
}

void KWrite::addBookmarkCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctBookmarkCommands, "Bookmark Commands");
  cmdMngr.addCommand(cmSetBookmark,       "&Set Bookmark...", ALT+Key_S);
  cmdMngr.addCommand(cmAddBookmark,       "&Add Bookmark"   , ALT+Key_A);
  cmdMngr.addCommand(cmClearBookmarks,    "&ClearBookmarks" , ALT+Key_C);
  cmdMngr.addCommand(cmSetBookmarks +0,   "Set Bookmark 1"  );
  cmdMngr.addCommand(cmSetBookmarks +1,   "Set Bookmark 2"  );
  cmdMngr.addCommand(cmSetBookmarks +2,   "Set Bookmark 3"  );
  cmdMngr.addCommand(cmSetBookmarks +3,   "Set Bookmark 4"  );
  cmdMngr.addCommand(cmSetBookmarks +4,   "Set Bookmark 5"  );
  cmdMngr.addCommand(cmSetBookmarks +5,   "Set Bookmark 6"  );
  cmdMngr.addCommand(cmSetBookmarks +6,   "Set Bookmark 7"  );
  cmdMngr.addCommand(cmSetBookmarks +7,   "Set Bookmark 8"  );
  cmdMngr.addCommand(cmSetBookmarks +8,   "Set Bookmark 9"  );
  cmdMngr.addCommand(cmSetBookmarks +9,   "Set Bookmark 10" );
  cmdMngr.addCommand(cmGotoBookmarks +0,  "Goto Bookmark 1" , ALT+Key_1);
  cmdMngr.addCommand(cmGotoBookmarks +1,  "Goto Bookmark 2" , ALT+Key_2);
  cmdMngr.addCommand(cmGotoBookmarks +2,  "Goto Bookmark 3" , ALT+Key_3);
  cmdMngr.addCommand(cmGotoBookmarks +3,  "Goto Bookmark 4" , ALT+Key_4);
  cmdMngr.addCommand(cmGotoBookmarks +4,  "Goto Bookmark 5" , ALT+Key_5);
  cmdMngr.addCommand(cmGotoBookmarks +5,  "Goto Bookmark 6" , ALT+Key_6);
  cmdMngr.addCommand(cmGotoBookmarks +6,  "Goto Bookmark 7" , ALT+Key_7);
  cmdMngr.addCommand(cmGotoBookmarks +7,  "Goto Bookmark 8" , ALT+Key_8);
  cmdMngr.addCommand(cmGotoBookmarks +8,  "Goto Bookmark 9" , ALT+Key_9);
  cmdMngr.addCommand(cmGotoBookmarks +9,  "Goto Bookmark 10", ALT+Key_0);
}

void KWrite::addStateCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctStateCommands, "State Commands");
  cmdMngr.addCommand(cmToggleInsert,    "Insert Mode"         , Key_Insert);
  cmdMngr.addCommand(cmToggleVertical,  "&Vertical Selections", Key_F5);
}


int KWrite::currentLine() {
  return kWriteView->cursor.y;
}

int KWrite::currentColumn() {
  return kWriteDoc->currentColumn(kWriteView->cursor);
}

int KWrite::currentCharNum() {
  return kWriteView->cursor.x;
}

void KWrite::setCursorPosition(int line, int col) {
  PointStruc cursor;

  cursor.x = col;
  cursor.y = line;
  kWriteView->updateCursor(cursor);
  kWriteView->center();
  kWriteDoc->unmarkFound();
//  kWriteView->updateView(ufPos, 0, line*kWriteDoc->fontHeight - height()/2);
//  kWriteDoc->updateViews(kWriteView); //uptade all other views except this one
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

void KWrite::doStateCommand(int cmdNum) {
  switch (cmdNum) {
    case cmToggleInsert:
      toggleInsert();
      return;
    case cmToggleVertical:
      toggleVertical();
      return;
  }
}

void KWrite::toggleInsert() {
  setConfig(configFlags ^ cfOvr);
}

void KWrite::toggleVertical() {
  setConfig(configFlags ^ cfVerticalSelect);
  emit statusMsg(configFlags & cfVerticalSelect ? i18n("Vertical Selections On") : i18n("Vertical Selections Off"));
}


int KWrite::numLines() {
  return kWriteDoc->numLines();
}

QString KWrite::text() {
  return kWriteDoc->text();
}

QString KWrite::currentTextLine() {
  TextLine *textLine;
  textLine = kWriteDoc->textLine(kWriteView->cursor.y);
  return QString(textLine->getText(), textLine->length());
}

QString KWrite::textLine(int num) {
  TextLine *textLine;
  textLine = kWriteDoc->textLine(num);
  return QString(textLine->getText(), textLine->length());
}

QString KWrite::currentWord() {
  return kWriteDoc->getWord(kWriteView->cursor);
}

QString KWrite::word(int x, int y) {
  PointStruc cursor;
  cursor.y = (kWriteView->yPos + y)/kWriteDoc->fontHeight;
  if (cursor.y < 0 || cursor.y > kWriteDoc->lastLine()) return QString();
  cursor.x = kWriteDoc->textPos(kWriteDoc->textLine(cursor.y), kWriteView->xPos-2 + x);
  return kWriteDoc->getWord(cursor);
}

void KWrite::setText(const char *s) {
  kWriteDoc->setText(s);
  kWriteDoc->updateViews();
}

void KWrite::insertText(const char *s, int len) {
  VConfig c;
  kWriteView->getVConfig(c);
  kWriteDoc->insert(c, s, len);
  kWriteDoc->updateViews();
}

bool KWrite::hasMarkedText() {
  return kWriteDoc->hasMarkedText();
}

QString KWrite::markedText() {
  return kWriteDoc->markedText(configFlags);
}

int KWrite::tabWidth() {
  return kWriteDoc->tabWidth;
}


void KWrite::loadFile(QIODevice &dev, bool insert) {
  VConfig c;

  if (!insert) {
    kWriteDoc->loadFile(dev);
  } else {
    kWriteView->getVConfig(c);
    kWriteDoc->insertFile(c, dev);
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
	s.sprintf(i18n("Malformed URL\n%s"),url);
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
        i18n("KWrite is already waiting\n"
	     "for an internet job to finish\n"\
             "Please wait until it has finished\n"
	     "Alternatively stop the running one."),
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
    kfmFile.sprintf(_PATH_TMP"/kwrite%i",time(0L));
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
    kfmFile.sprintf(_PATH_TMP"/kwrite%i",time(0L));
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
//  string.sprintf(i18n("Finished '%s'"),tmpFile.data());
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


bool KWrite::hasFileName() {
  return kWriteDoc->hasFileName();
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
        if (save() == CANCEL) return false;
        if (isModified()) {
            query = QMessageBox::warning(this,
            i18n("Sorry"),
            i18n("Could not save the document.\nDiscard it and continue?"),
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

  url = KFileDialog::getOpenFileURL(kWriteDoc->fileName(),"*",this);
  if (url.isEmpty()) return;
//  kapp->processEvents();
  loadURL(url);
}

void KWrite::insertFile() {
  QString url;

  url = KFileDialog::getOpenFileURL(kWriteDoc->fileName(),"*",this);
  if (url.isEmpty()) return;
//  kapp->processEvents();
  loadURL(url,lfInsert);
}

KWrite::fileResult KWrite::save() {
  if (isModified()) {
    if (kWriteDoc->hasFileName()) {
      writeURL(kWriteDoc->fileName(),lfNoAutoHl);
    } else return saveAs();
  } else emit statusMsg(i18n("No changes need to be saved"));
  return OK;
}

KWrite::fileResult KWrite::saveAs() {
  QString url;
  int query;

  do {
    query = 0;
    url = KFileDialog::getSaveFileURL(kWriteDoc->fileName(),"*",this);
    if (url.isEmpty()) return CANCEL;

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
  return OK;
}

void KWrite::doCursorCommand(int cmdNum) {
  VConfig c;
  kWriteView->getVConfig(c);
  if (cmdNum & selectFlag) c.flags |= cfMark;
  if (cmdNum & multiSelectFlag) c.flags |= cfMark | cfKeepSelection;
  cmdNum &= ~(selectFlag | multiSelectFlag);
  kWriteView->doCursorCommand(c, cmdNum);
  kWriteDoc->updateViews();
}

void KWrite::doEditCommand(int cmdNum) {
  VConfig c;
  kWriteView->getVConfig(c);
  kWriteView->doEditCommand(c, cmdNum);
  kWriteDoc->updateViews();
}


void KWrite::clear() {
  kWriteDoc->clear();
  kWriteDoc->clearFileName();
  kWriteDoc->updateViews();
}
/*
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
  kWriteDoc->paste(kWriteView, c);
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
*/

void addToStrList(QStrList &list, const char *str) {
  if (list.find(str) != -1) list.remove();
  if (list.count() >= 16) list.removeLast();
  list.insert(0, str);
}

void KWrite::find() {
  SearchDialog *searchDialog;

  searchDialog = new SearchDialog(&searchForList, 0L,
    searchFlags & ~sfReplace,topLevelWidget());

  kWriteView->focusOutEvent(0L);// QT bug ?
  if (searchDialog->exec() == QDialog::Accepted) {
    addToStrList(searchForList, searchDialog->getSearchFor());
    searchFlags = searchDialog->getFlags() | (searchFlags & sfPrompt);
    initSearch(s, searchFlags);
    searchAgain(s);
  }
  delete searchDialog;
}

void KWrite::replace() {
  SearchDialog *searchDialog;

  searchDialog = new SearchDialog(&searchForList, &replaceWithList,
    searchFlags | sfReplace,topLevelWidget());

  kWriteView->focusOutEvent(0L);// QT bug ?
  if (searchDialog->exec() == QDialog::Accepted) {
    addToStrList(searchForList, searchDialog->getSearchFor());
    addToStrList(replaceWithList, searchDialog->getReplaceWith());
    searchFlags = searchDialog->getFlags();
    initSearch(s, searchFlags);
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

void KWrite::findAgain() {

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
    kWriteView->center();
    kWriteDoc->unmarkFound();
    kWriteView->updateView(ufUpdateOnScroll);
    kWriteDoc->updateViews(kWriteView); //uptade all other views except this one
  }
  delete dlg;
}


void KWrite::initSearch(SConfig &s, int flags) {
  const char *searchFor = searchForList.getFirst();

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
    s.startCursor.x -= strlen(searchFor);//.length();
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

 const char *searchFor = searchForList.getFirst();

  slen = strlen(searchFor);//.length();
  do {
    query = 1;
    if (kWriteDoc->doSearch(s,searchFor)) {
      cursor = s.cursor;
      if (!(s.flags & sfBackward)) s.cursor.x += slen;
      kWriteView->updateCursor(s.cursor);
      exposeFound(cursor,slen,(s.flags & sfAgain) ? 0 : ufUpdateOnScroll,false);
    } else {
      if (!(s.flags & sfFinished)) {
        // ask for continue
        if (!(s.flags & sfBackward)) {
          // forward search
          str = i18n("End of document reached.\n"
		     "Continue from the beginning?");
	  query = QMessageBox::information(this,
            i18n("Find"),
            str,
            i18n("Yes"),
            i18n("No"),
            "",0,1);
        } else {
          // backward search
          str = 
            i18n("Beginning of document reached.\n"
		 "Continue from the end");
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

  const char *searchFor = searchForList.getFirst();
  const char *replaceWith = replaceWithList.getFirst();
  slen = strlen(searchFor);//.length();
  rlen = strlen(replaceWith);//replaceWith.length();

  switch (result) {
    case srYes: //yes
      kWriteDoc->recordStart(s.cursor,true);
      kWriteDoc->recordReplace(s.cursor,slen,replaceWith,rlen);
      replaces++;
      if (s.cursor.y == s.startCursor.y && s.cursor.x < s.startCursor.x)
        s.startCursor.x += rlen - slen;
      if (!(s.flags & sfBackward)) s.cursor.x += rlen;
      kWriteDoc->recordEnd(kWriteView, s.cursor, configFlags | cfPersistent);
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

  textLine = kWriteDoc->textLine(cursor.y);
  x1 = kWriteDoc->textWidth(textLine,cursor.x)        -10;
  x2 = kWriteDoc->textWidth(textLine,cursor.x + slen) +20;
  y1 = kWriteDoc->fontHeight*cursor.y                 -10;
  y2 = y1 + kWriteDoc->fontHeight                     +30;

  xPos = kWriteView->xPos;
  yPos = kWriteView->yPos;

  if (x1 < 0) x1 = 0;
  if (replace) y2 += 90;

  if (x1 < xPos || x2 > xPos + kWriteView->width()) {
    xPos = x2 - kWriteView->width();
  }
  if (y1 < yPos || y2 > yPos + kWriteView->height()) {
    xPos = x2 - kWriteView->width();
    yPos = kWriteDoc->fontHeight*cursor.y - height()/3;
  }
  kWriteView->setPos(xPos, yPos);
  kWriteView->updateView(flags);// | ufPos,xPos,yPos);
  kWriteDoc->updateViews(kWriteView);
//  kWriteDoc->updateViews();
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
    str.sprintf(i18n("%d replace(s) made"), replaces);
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
    str.sprintf(i18n("%d replace(s) made.\n"
	       "End of document reached.\n"
	       "Continue from the beginning?"), replaces);
  query = QMessageBox::information(this,
      i18n("Replace"),
      str,
      i18n("Yes"),
      i18n("No"),
      "",0,1);
  } else {
    // backward search
    str.sprintf(i18n("%d replace(s) made.\n"
		"Beginning of document reached.\n"
		"Continue from the end?"),replaces);
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

void KWrite::installBMPopup(KGuiCmdPopup *p) {

  connect(p, SIGNAL(aboutToShow()), SLOT(updateBMPopup()));
//  connect(p,SIGNAL(activated(int)),SLOT(gotoBookmark(int)));
//  bmEntries = p->count();
}

void KWrite::setBookmark() {
  QPopupMenu *popup;
  int z;
  char s[8];

  popup = new QPopupMenu(0L);

  for (z = 1; z <= 10; z++) {
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

void KWrite::clearBookmarks() {
  bookmarks.clear();
}

void KWrite::setBookmark(int n) {
  KWBookmark *b;

 if (n >= 10) return;
  while ((int) bookmarks.count() <= n) bookmarks.append(new KWBookmark());
  b = bookmarks.at(n);
  b->xPos = kWriteView->xPos;
  b->yPos = kWriteView->yPos;
  b->cursor = kWriteView->cursor;
}

void KWrite::gotoBookmark(int n) {
  KWBookmark *b;

  if (n < 0 || n >= (int) bookmarks.count()) return;
  b = bookmarks.at(n);
  if (b->cursor.y == -1) return;
  kWriteView->updateCursor(b->cursor);
  kWriteView->setPos(b->xPos, b->yPos);
  kWriteDoc->unmarkFound();
//  kWriteView->updateView(ufPos, b->xPos, b->yPos);
//  kWriteDoc->updateViews(kWriteView); //uptade all other views except this one
  kWriteDoc->updateViews();
}

void KWrite::doBookmarkCommand(int cmdNum) {
  if (cmdNum == cmSetBookmark) {
    setBookmark();
  } else if (cmdNum == cmAddBookmark) {
    addBookmark();
  } else if (cmdNum == cmClearBookmarks) {
    clearBookmarks();
  } else if (cmdNum >= cmSetBookmarks && cmdNum < cmSetBookmarks +10) {
    setBookmark(cmdNum - cmSetBookmarks);
  } else if (cmdNum >= cmGotoBookmarks && cmdNum < cmGotoBookmarks +10) {
    gotoBookmark(cmdNum - cmGotoBookmarks);
  }
}

void KWrite::updateBMPopup() {
  KGuiCmdPopup *p;
  KWBookmark *b;
  char buf[64];
  int z, id;

  p = (KGuiCmdPopup *) sender();
  p->clear();
  p->addCommand(ctBookmarkCommands, cmSetBookmark);
  p->addCommand(ctBookmarkCommands, cmAddBookmark);
  p->addCommand(ctBookmarkCommands, cmClearBookmarks);
//  p->insertSeparator();
  for (z = 0; z < (int) bookmarks.count(); z++) {
    b = bookmarks.at(z);
    if (b->cursor.y >= 0) {
      if (p->count() == 3) p->insertSeparator();
      id = p->addCommand(ctBookmarkCommands, cmGotoBookmarks + z);
      sprintf(buf,i18n("Line %d"), b->cursor.y +1);
      p->setText(buf, id);
//      p->insertItem(buf,z);
//      if (z < 9) p->setAccel(ALT+keys[z],z);
    }
  }
/*
  while ((int) p->count() > bmEntries) {
    p->removeItemAt(p->count() - 1);
  }

  for (z = 0; z < (int) bookmarks.count(); z++) {
    b = bookmarks.at(z);
//  for (b = bookmarks.first(); b != 0L; b = bookmarks.next()) {
    if (b->cursor.y >= 0) {
      if ((int) p->count() == bmEntries) p->insertSeparator();
      sprintf(buf,i18n("Line %d"),b->cursor.y +1);
      p->insertItem(buf,z);
      if (z < 9) p->setAccel(ALT+keys[z],z);
    }
  }*/
}


void KWrite::readConfig(KConfig *config) {

  searchFlags = config->readNumEntry("SearchFlags",sfPrompt);
  configFlags = config->readNumEntry("ConfigFlags", cfAutoIndent
    | cfBackspaceIndent | cfRemoveSpaces | cfDelOnInput) & ~cfMark;
  wrapAt = config->readNumEntry("WrapAt",79);
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

  wrapAt = config->readNumEntry("WrapAt",79);
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
  wrapAt = config->readNumEntry("WrapAt",79);
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


  hlManager = kWriteDoc->hlManager;
  hlDataList.setAutoDelete(true);
  //this gets the data from the KConfig object
  hlManager->getHlDataList(hlDataList);
  dlg = new HighlightDialog(hlManager,&hlDataList,kWriteDoc->getHighlight(),this);
//  dlg->hlChanged(kWriteDoc->getHighlight());
  if (dlg->exec() == QDialog::Accepted) {
    //this stores the data into the KConfig object
    hlManager->setHlDataList(hlDataList);
  }
  delete dlg;
}

int KWrite::getHl() {
  return kWriteDoc->getHighlight();
}

void KWrite::setHl(int n) {
  kWriteDoc->setHighlight(n);
  kWriteDoc->updateViews();
}

int KWrite::getEol() {
  return kWriteDoc->eolMode;
}

void KWrite::setEol(int eol) {
  kWriteDoc->eolMode = eol;
  kWriteDoc->setModified(true);
}

void KWrite::print() {
  int fontHeight;
  int lines, pages;
  int line, c, p, l, y;
  QPrinter prt;

  if (prt.setup()) {
    QPaintDeviceMetrics pdm(&prt);
    QPainter paint;

    paint.begin(&prt);
    fontHeight = kWriteDoc->fontHeight;
    lines = pdm.height()/fontHeight;
    pages = kWriteDoc->numLines()/lines;
    for (c = 1; c <= prt.numCopies(); c++) {
      line = 0;
      for (p = 0; p <= pages; p++) {
        y = 0;
        if (prt.pageOrder() == QPrinter::LastPageFirst) line = (pages - p)*lines;
        for (l = 0; l < lines; l++) {
          kWriteDoc->printTextLine(paint, line, pdm.width(), y);
          line++;
          y += fontHeight;
        }
        if (p < pages || c < prt.numCopies()) {
          if (!prt.newPage()) goto abort;
        }
      }
    }
    abort:
    paint.end();
  }
}
/*
void KWrite::keyPressEvent(QKeyEvent *e) {
  int z;

  if (e->state() & AltButton) {
    for (z = 0; z < 9; z++) {
      if (keys[z] == e->key()) gotoBookmark(z);
    }
  }
}
*/
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
//kWriteView->resize(width() -20, height() -20);
  kWriteView->tagAll();
  kWriteView->updateView(0/*ufNoScroll*/);
}



