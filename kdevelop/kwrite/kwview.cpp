#include "kwview.h"
#include "kwdoc.h"

#include <kapp.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <qstring.h>
#include <qregexp.h>
#include <qwidget.h>
#include <qfont.h>
#include <qpainter.h>
#include <qkeycode.h>
#include <qmsgbox.h>
#include <qpixmap.h>
#include <qfileinf.h>
#include <qfile.h>
#include <qevent.h>
#include <qdir.h>
#include <qprinter.h>
#include <qprintdialog.h>
#include <kprinter.h>
#include <qpaintdevicemetrics.h>
#include <qclipbrd.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

#include <ctype.h>
#include <time.h>

#include <X11/Xlib.h> //used to have XSetTransientForHint()

int kw_bookmark_keys[] = {Qt::Key_1,Qt::Key_2,Qt::Key_3,Qt::Key_4,Qt::Key_5,Qt::Key_6,Qt::Key_7,Qt::Key_8,Qt::Key_9};


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
  info = new BufferInfo;   // causes memory leak but don't know how to handle (Zu)
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
    if (info->user == user) 
    {
      bufferInfoList.remove(z);
      delete info;
    }
  }
  resizeBuffer(0,0,0);
}

KIconBorder::KIconBorder(KWrite *write, KWriteDoc *doc, KWriteView *view) :
  QWidget(write),
  kWrite(write),
  kWriteDoc(doc),
  kWriteView(view),
  LMBIsBreakpoint(true),
  cursorOnLine(0),
  menuId_LMBBrkpoint(0),
  menuId_LMBBookmark(0),
  menuId_editBrkpoint(0)
{
  setBackgroundColor(colorGroup().background());
  setGeometry ( 2, 2, iconBorderWidth, iconBorderHeight);

  selectMenu.insertTitle(i18n("Breakpoints/Bookmarks"));
  selectMenu.insertItem(i18n("Toggle bookmark"), this, SLOT(slotToggleBookmark()));
  selectMenu.insertItem(i18n("Clear all bookmarks"), kWrite, SLOT(clearBookmarks()));
  selectMenu.insertSeparator();
  selectMenu.insertItem(i18n("Toggle breakpoint"), kWrite, SLOT(slotToggleBreakpoint()));
  menuId_editBrkpoint   = selectMenu.insertItem(i18n("Edit breakpoint"), this, SLOT(slotEditBreakpoint()));
  menuId_enableBrkpoint = selectMenu.insertItem(i18n("Enable breakpoint"), this, SLOT(slotToggleBPEnabled()));
  selectMenu.insertSeparator();
  selectMenu.insertItem(i18n("Clear all breakpoints"), kWrite, SIGNAL(clearAllBreakpoints()));
  selectMenu.insertSeparator();
  menuId_LMBBrkpoint  = selectMenu.insertItem(i18n("LMB sets breakpoints"), this, SLOT(slotLMBMenuToggle()));
  menuId_LMBBookmark  = selectMenu.insertItem(i18n("LMB sets bookmarks"),   this, SLOT(slotLMBMenuToggle()));
  selectMenu.setCheckable(true);

  selectMenu.setItemChecked(menuId_LMBBrkpoint, LMBIsBreakpoint);
  selectMenu.setItemChecked(menuId_LMBBookmark, !LMBIsBreakpoint);
}


KIconBorder::~KIconBorder()
{
}

/**  */
/** Paints an icon to y */
void KIconBorder::showIcon(const QPixmap& icon, int y)
{
  QPainter paint;

  paint.begin(this);
	paint.drawPixmap(2,y,icon);
  paint.end();
}

void KIconBorder::clearPixelLines(int startPixelLine, int numberPixelLine)
{
	QPainter paint;

	paint.begin(this);

  paint.fillRect(0, startPixelLine, iconBorderWidth-2, numberPixelLine, colorGroup().background());

  paint.setPen(white);
  paint.drawLine(iconBorderWidth-2, startPixelLine, iconBorderWidth-2, startPixelLine + numberPixelLine);
  paint.setPen(QColor(colorGroup().background()).dark());
  paint.drawLine(iconBorderWidth-1, startPixelLine, iconBorderWidth-1, startPixelLine + numberPixelLine);
	paint.end();
}

void KIconBorder::clearLine(int line)
{
	int y = line * kWriteDoc->getFontHeight() - kWriteView->getYPos();

	clearPixelLines(y, kWriteDoc->getFontHeight());
}

void KIconBorder::clearAll()
{
	clearPixelLines(0, iconBorderHeight);
}

void KIconBorder::paintBookmark(int line)
{
  if (kWrite->bookmarked(line))
	{
    #include "pix/bookmark.xpm"
		QPixmap bookmarkPixmap(bookmark_xpm);
    showIcon(bookmarkPixmap, line * kWriteDoc->getFontHeight() - kWriteView->getYPos());
	}
}

void KIconBorder::paintBreakpoint(int line)
{
	if (line < 0 || line>kWriteDoc->lastLine())
		return;

  // A breakpoint is on this line - draw it
  TextLine* tLine = kWriteDoc->textLine(line);
	if (tLine && (tLine->getBPId() != 0))
	{
    QPixmap bpPix;
    if (!tLine->isBPEnabled())
    {
      #include "pix/breakpoint_gr.xpm"
      bpPix = QPixmap(breakpoint_gr_xpm);
    }
    else
    {
      if (tLine->isBPPending())
      {
        #include "pix/breakpoint_bl.xpm"
        bpPix = QPixmap(breakpoint_bl_xpm);
      }
      else
      {
        #include "pix/breakpoint.xpm"
        bpPix = QPixmap(breakpoint_xpm);
      }
    }

    showIcon(bpPix, line * kWriteDoc->getFontHeight() - kWriteView->getYPos());
  }
}

void KIconBorder::paintDbgPosition(int line)
{
  // This line is the position in source the debugger has stopped at.
  if (kWrite->getStepLine() == line )
	{
    #include "pix/ddd.xpm"
		QPixmap dddPixmap(ddd_xpm);
		showIcon(dddPixmap, line * kWriteDoc->getFontHeight() - kWriteView->getYPos());
	}
}

void KIconBorder::paintLine(int line)
{
  if (line >= 0 && line < kWriteDoc->getTextLineCount())
  {
		clearLine(line);
		paintBookmark(line);
		paintBreakpoint(line);
		paintDbgPosition(line);
  }
}

void KIconBorder::paintEvent(QPaintEvent* e)
{
	int lineStart = 0;
	int lineEnd = 0;

	QRect updateR = e->rect();

	int h = kWriteDoc->getFontHeight();
	int yPos = kWriteView->getYPos();
	if (h) {
  	lineStart = (yPos + updateR.y()) / h;
	  lineEnd = (yPos + updateR.y() + updateR.height()) / h;
	}

	for(int line = lineStart; line <= lineEnd; line++)
	{
		paintLine(line);
	}
}

/** Checks MouseEvents and executes the popup
    All slots called from here (including the menu work off the "cursorOnLine" variable
    set at the line the user has clicked on. We cannot pass the cursorOnLine as a parameter
    because the menu items cannot have parameters
*/
void KIconBorder::mousePressEvent(QMouseEvent* e)
{
  kWriteView->placeCursor( 0, e->y(), 0 );
	cursorOnLine = ( e->y() + kWriteView->getYPos() ) / kWriteDoc->getFontHeight();

  switch (e->button())
  {
    case LeftButton:
    {
      if (LMBIsBreakpoint)
        kWrite->slotToggleBreakpoint();
      else
        slotToggleBookmark();
      break;
    }
    case RightButton:
    {
      if (TextLine* tline=kWriteDoc->textLine(cursorOnLine))
      {
        if (tline->getBPId())
        {
          selectMenu.setItemEnabled (menuId_editBrkpoint, true);
          selectMenu.setItemEnabled (menuId_enableBrkpoint, true);
          if (tline->isBPEnabled())
            selectMenu.changeItem(menuId_enableBrkpoint, i18n("Disable breakpoint"));
          else
            selectMenu.changeItem(menuId_enableBrkpoint, i18n("Enable breakpoint"));
        }
        else
        {
          selectMenu.setItemEnabled (menuId_editBrkpoint, false);
          selectMenu.setItemEnabled (menuId_enableBrkpoint, false);
          selectMenu.changeItem(menuId_enableBrkpoint, i18n("Enable breakpoint"));
        }
        selectMenu.exec(mapToGlobal(QPoint(e->x()-selectMenu.width(),e->y()-20)));
      }
      break;
    }

    case MidButton:
    {
      slotToggleBookmark();
      break;
    }
    default:
        break;
  }
}

void KIconBorder::slotLMBMenuToggle()
{
  LMBIsBreakpoint = !LMBIsBreakpoint;
	selectMenu.setItemChecked(menuId_LMBBrkpoint, LMBIsBreakpoint);
	selectMenu.setItemChecked(menuId_LMBBookmark, !LMBIsBreakpoint);
}

void KIconBorder::slotEditBreakpoint()
{
  emit kWrite->editBreakpoint(QString(kWriteDoc->fileName()), cursorOnLine+1);
}

void KIconBorder::slotToggleBPEnabled()
{
  emit kWrite->toggleBPEnabled(QString(kWriteDoc->fileName()), cursorOnLine+1);
}

/** toggles a bookmark */
void KIconBorder::slotToggleBookmark()
{
	debug("KIconBorder::slotToggleBookmark !\n");

	kWrite->toggleBookmark(cursorOnLine);
}

/** gets the Range of the function the cursor is in */
void KIconBorder::slotGetRange()
{
	kWriteView->getRange(kWriteView->cursor.y);
}

KWriteView::KWriteView(KWrite *write, KWriteDoc *doc) : QWidget(write) {
	m_hasFocus = false;
  kWrite = write;
  kWriteDoc = doc;
  bIsPainting = false;

  QWidget::setCursor(ibeamCursor);
  setMouseTracking(true);   //dbg
  setBackgroundMode(NoBackground);
  setFocusPolicy(ClickFocus);
#ifdef QT_I18N
  setInputMethodEnabled( TRUE );
  setInputMethodSpotLocation(0, kWriteDoc->fontAscent);
#endif
  move(iconBorderWidth+2,2);

  leftBorder = new KIconBorder(kWrite, kWriteDoc, this);

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

  if (doc)
    doc->registerView(this);
}

KWriteView::~KWriteView() {
  if(kWriteDoc) {
    kWriteDoc->removeView(this);
  }
  releaseBuffer(this);
}

QPoint KWriteView::getCursorCoordinates() const
{
    int h, y, x;

    h = kWriteDoc->fontHeight;
    y = h*(cursor.y+1) - yPos;
    x = cXPos - (xPos-2);

    QPoint pt( x, y );
    return pt;
}

bool KWriteView::event( QEvent *e )
{
  if ( e->type() == KeyPress ){
    QKeyEvent *k = (QKeyEvent *)e;
    if ( k->key() == Key_Tab ){
      keyPressEvent( k );
      return true;
    }
  }
  return QWidget::event( e );
}

bool KWriteView::focusNextPrevChild( bool )
{
  // avoids a focus out event on Tab key
  return false;
}

void KWriteView::cursorLeft(VConfig &c) {

  cursor.x--;
#ifdef QT_I18N
  if (cursor.x > 0 &&
      iseucchar(kWriteDoc->textLine(cursor.y)->getChar(cursor.x))) {
    cursor.x--;
  }
#endif
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
#ifdef QT_I18N
  if (cursor.x >= 0 &&
      iseucchar(kWriteDoc->textLine(cursor.y)->getChar(cursor.x))) {
    cursor.x++;
  }
#endif
  cursor.x++;
  cOldXPos = cXPos = kWriteDoc->textWidth(cursor);
  update(c);
}


void KWriteView::cursorLeftWord(VConfig &c) {

  TextLine* CurrLine;

  do{
    cursor.x--;
    if (cursor.x < 0) {
      if (c.flags & cfWrapCursor && cursor.y > 0) {
        cursor.y--;
        cursor.x = kWriteDoc->textLength(cursor.y);
      }else break;
    }
    CurrLine = kWriteDoc->textLine(cursor.y);

  }while( !isalnum(CurrLine->getChar(cursor.x)) ||
         isalnum(CurrLine->getChar(cursor.x - 1))
        );

  cOldXPos = cXPos = kWriteDoc->textWidth(cursor);
  update(c);
}

void KWriteView::cursorRightWord(VConfig &c) {

  TextLine* CurrLine;

  do{
    if (cursor.x >= kWriteDoc->textLength(cursor.y)) {
      if (c.flags & cfWrapCursor) {
        if (cursor.y == kWriteDoc->lastLine()) break;
        cursor.y++;
        cursor.x = -1;
      }else break;
    }

    cursor.x++;
    CurrLine = kWriteDoc->textLine(cursor.y);
  }while( isalnum(CurrLine->getChar(cursor.x - 1)) ||
         !isalnum(CurrLine->getChar(cursor.x))
        );

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
  if (QABS(dx) < width())
    scroll(dx,0);
  else
    QWidget::update();
  this->setFocus();
}

void KWriteView::changeYPos(int p) {
  int dy;

  //kdDebug() << "changeYPos1: " << p << " isPainting " << bIsPainting << endl;

  if ( bIsPainting )
    return;
  dy = yPos - p;
  yPos = p;
  startLine = yPos / kWriteDoc->fontHeight;
  endLine = (yPos + height() -1) / kWriteDoc->fontHeight;
  //kdDebug() << "changeYPos2: " << p << " dy " << dy << " height " << height() << endl;
  if (QABS(dy) < height())
  {
    leftBorder->scroll(0,dy);
    scroll(0,dy);
  }
  else
  {
    QWidget::update();
  }
  this->setFocus();
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
    tagLines(c.cursor.y, c.cursor.y);
    cursorOn = false;
  }
#ifdef QT_I18N
#if 0
  int h, x, y;
  h = kWriteDoc->fontHeight;
  y = h*cursor.y - yPos + kWriteDoc->fontAscent;
  x = cXPos - (xPos-2);
  setInputMethodSpotLocation(x, y);
#endif
#endif

  if (bm.sXPos < bm.eXPos) {
    tagLines(bm.cursor.y, bm.cursor.y);
  }
  // make new bracket mark
  if (c.flags & cfHighlightBrackets) {
    kWriteDoc->newBracketMark(cursor, bm);
  }

  if (c.flags & cfMark) {
    kWriteDoc->selectTo(c.cursor,cursor,c.flags);
  } else {
    if (!(c.flags & cfPersistent))
      kWriteDoc->deselectAll();
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
}

void KWriteView::delLine(int line)
{
  emit kWrite->deleteLine( line );

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
}

void KWriteView::wheelEvent(QWheelEvent *e)
{
    e->accept();
    yScroll->setValue(yScroll->value() - e->delta());
//    kdDebug() << yScroll->value() << " - " << e->delta() << " = " << yScroll->value() - e->delta() << endl;
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
  if (bm.sXPos < bm.eXPos) {
    tagLines(bm.cursor.y, bm.cursor.y);
  }
  if (kWrite->configFlags & cfHighlightBrackets) {
    kWriteDoc->newBracketMark(cursor, bm);
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

    xMax = kWriteDoc->textWidth() - w + iconBorderWidth;
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
    cXPosMin = xPos + 4;
    cXPosMax = xPos + w - 8 - iconBorderWidth;
    cYPosMin = yPos;
    cYPosMax = yPos + (h - fontHeight);

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

  if (fontHeight) {
    startLine = yPos / fontHeight;
    endLine = (yPos + h -1) / fontHeight;
  }

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
      repaint(0, 0, width(), height(), false);
    } else {
      if(dy) leftBorder->scroll(0,dy);

      if (updateState > 0) paintTextLines(oldXPos,oldYPos);

      if (dx || dy) {
        scroll(dx,dy);
      } else if (cursorOn) paintCursor();
      if (bm.eXPos > bm.sXPos) paintBracketMark();
    }
  }

  exposeCursor = false;
  updateState = 0;
#ifdef QT_I18N
  int x, y;
  h = kWriteDoc->fontHeight;
  y = h*cursor.y - yPos + kWriteDoc->fontAscent;
  x = cXPos - (xPos-2);
  setInputMethodSpotLocation(x, y);
#endif
}

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
    kWriteDoc->paintTextLine(paint,line,xStart,xEnd, kWrite->flags() & cfShowTabs);
    bitBlt(this,0,line*h - yPos,drawBuffer,0,0,width(),h);

   leftBorder->paintLine(line);
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
    paint.end();
  } else {
    if ( !drawBuffer->isNull() ){
  //debug("===============================maximus BUG begin");
      paint.begin(drawBuffer);
  //debug("===============================maximus BUG end");
      kWriteDoc->paintTextLine(paint,cursor.y,cXPos - 2, cXPos + 3, kWrite->flags() & cfShowTabs);
      bitBlt(this,x - 2,y,drawBuffer,0,0,5,h);
      paint.end();
    }
  }
}

void KWriteView::paintBracketMark() {

  if (!(kWrite->configFlags & cfHighlightBrackets)) {
    return;
  }

  if ((bm.cursor.x < 0) || (bm.cursor.y < 0)) {
    return;
  }

  int y = kWriteDoc->fontHeight*(bm.cursor.y +1) - yPos -1;

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

void KWriteView::focusInEvent(QFocusEvent *e)
{
  // when we open a popup menu FocusIn and FocusOut is called for KWriteView which makes it very slow because of the triggered repaint
  // (Falk: it seems to be useless that Qt does it that way!?! -> maybe bug of QT)
  if (e && ((e->reason())==QFocusEvent::Popup)) {
    return;
  }

  if (m_hasFocus)
    return;
  else
    m_hasFocus = true;

//  printf("got focus %d\n",cursorTimer);

  QString text=kapp->clipboard()->text();
  //dbg - start
  kWrite->newCurPos();
  kWrite->newStatus();
  kWrite->newUndo();
  // dbg - end
  if (!cursorTimer) {
    cursorTimer = startTimer(500);
    cursorOn = true;
    paintCursor();
  }

  emit kWrite->clipboardStatus(this, !text.isEmpty());
}

void KWriteView::focusOutEvent(QFocusEvent *e) {
//  printf("lost focus\n");

  // when we open a popup menu FocusIn and FocusOut is called for KWriteView which makes it very slow because of the triggered repaint
  // (Falk: it seems to be useless that Qt does it that way!?! -> maybe bug of QT)
  if (e && ((e->reason())==QFocusEvent::Popup)) {
    return;
  }

  if (cursorTimer) {
    killTimer(cursorTimer);
    cursorTimer = 0;
  }

  if (cursorOn) {
    cursorOn = false;
    paintCursor();
  }
	m_hasFocus = false;
}

void KWriteView::scroll( int dx, int dy ) {
  bIsPainting = true;
  QWidget::scroll( dx, dy );
  bIsPainting = false;
}

void KWriteView::keyPressEvent(QKeyEvent *e) {
  VConfig c;
  bool t;

  if (e->state() & AltButton) {
    e->ignore();
    return;
  }

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

#ifdef QT_I18N
  if ( (e->key() == 0) && (e->text().length() > 0) ) {
    if (c.flags & cfDelOnInput) {
      kWriteDoc->delMarkedText(this,c);
      getVConfig(c);
    }
    int i, len;
    len = e->text().length();
    char* mbuf = e->text().data();
    for (i = 0; i < len; i++) {
	if (iseucchar(*mbuf)) {
	    kWriteDoc->insertChar(this,c, mbuf, 2);
	    mbuf += 2;
	    i++;
	} else {
	    kWriteDoc->insertChar(this,c, *mbuf++);
	}
    }
  } else
#endif
  if ((e->ascii() >= 32 )  // removed: || e->ascii() == '\t'
       && e->key() != Key_Delete && e->key() != Key_Backspace) {
       // process normal characters
        //    printf("input %d\n",e->ascii());
    if (c.flags & cfDelOnInput) {
      // delete on input
      kWriteDoc->delMarkedText(this,c);
      getVConfig(c);
    }
    // insert the character at the current cursor.
    kWriteDoc->insertChar(this,c,e->ascii());
    if (c.flags & cfHighlightBrackets)
       kWriteDoc->newBracketMark(c.cursor, bm);
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
        case Key_Home:
            top(c);
            home(c);
            break;
        case Key_End:
            bottom(c);
            end(c);
            break;
        case Key_Delete:
//        case Key_X:
            kWriteDoc->cut(this,c);
            break;
       	case Key_Left:
            cursorLeftWord(c);
            break;
       	case Key_Right:
            cursorRightWord(c);
            break;
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
            if ((c.flags & cfDelOnInput) && kWriteDoc->hasMarkedText())
              kWriteDoc->delMarkedText(this,c);
            else kWriteDoc->backspace(this,c);
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
            if ((c.flags & cfDelOnInput) && kWriteDoc->hasMarkedText())
              kWriteDoc->delMarkedText(this,c);
            kWriteDoc->newLine(this,c);
            //emit returnPressed();
            //e->ignore();
            break;
        case Key_Insert:
            if (e->state() & ShiftButton) kWriteDoc->paste(this,c);
              else kWrite->toggleOverwrite();
            break;
        case Key_Tab:
            kWriteDoc->tab(this,c);
            break;
        case Key_BackTab:
            kWriteDoc->shiftTab(this,c);
            break;
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
  else if (e->button() == MidButton) {
    placeCursor(e->x(),e->y(),0);
    kWrite->paste();
  }

  if (m_hasFocus && (e->button() != RightButton)) {
    kWrite->newCurPos();
    kWrite->newStatus();
    kWrite->newUndo();
  }

  // Call CEditWidget::mousePressEvent to handle the RightButton case
  QMouseEvent ee(QEvent::MouseButtonPress, mapToParent(e->pos()), e->button(), e->state());
  kWrite->mousePressEvent(&ee);
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
    VConfig c;
    getVConfig(c);

    if (c.flags & cfAutoCopy) {
      kWrite->copy();
    }
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
  int h, y, yEnd;
  int line = 0;
  bIsPainting = true;		// toggle scrolling off
//  bool isVisible;

  QRect updateR = e->rect();
//  printf("update rect  = ( %i, %i, %i, %i )\n",
//    updateR.x(),updateR.y(), updateR.width(), updateR.height() );

  QPainter paint;
  paint.begin(drawBuffer);

  xStart = xPos-2 + updateR.x();
  xEnd = xStart + updateR.width();

  h = kWriteDoc->fontHeight;
  if (h) {
    line = (yPos + updateR.y()) / h;
  }
  y = line*h - yPos;
  yEnd = updateR.y() + updateR.height();

  int textLineCount = kWriteDoc->getTextLineCount()-1;
  while (y < yEnd)
  {
    if (textLineCount>line)
    {
      int ctxNum = 0;
      TextLine *textLine = kWriteDoc->textLine(line);
      if (line > 0)
        ctxNum = kWriteDoc->textLine(line - 1)->getContext();
      if ( CTX_UNDEF == ctxNum )
        ctxNum = 0;
      if (kWriteDoc->highlight)
        ctxNum = kWriteDoc->highlight->doHighlight(ctxNum,textLine);
      if (textLine)
        textLine->setContext(ctxNum);
    }

    kWriteDoc->paintTextLine(paint,line,xStart,xEnd, kWrite->flags() & cfShowTabs);
//    if (cursorOn && line == cursor.y) paintCursor(paint,cXPos - xStart,h);
    bitBlt(this,updateR.x(),y,drawBuffer,0,0,updateR.width(),h);

    leftBorder->paintLine(line);

    line++;
    y += h;
    if (!h)
      break;  // otherwise there's an infinite loop ;(
  }

  paint.end();
  if (cursorOn) paintCursor();
  if (bm.eXPos > bm.sXPos) paintBracketMark();
  bIsPainting = false;		// toggle scrolling on
}

void KWriteView::resizeEvent(QResizeEvent *) {
//  printf("KWriteView::resize\n");
  resizeBuffer(this,width(),kWriteDoc->fontHeight);
  leftBorder->resize(leftBorder->width(), height());
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

/** returns the range of a hidden function */
int KWriteView::getRange(int midline){
	int counter=0;
	int n;
	QString textline;
	int line=midline;

//	midline=cursor.y;
debug("Midline: %i",midline);

	textline=kWriteDoc->contents.at(midline)->getString();

debug(textline);

	counter=textline.contains("{")-textline.contains("}");

debug("First Counter: %i",counter);

	while (counter<1) {
		line--;
		if (line==0) break;
		textline=kWriteDoc->contents.at(line)->getString();
		counter+=textline.contains("{")-textline.contains("}");
		debug("Line in first while: %i",line);
	}

	debug("Counter: %i at %i",counter,line);
	line++;

	while	(counter != 0) {
		textline=kWriteDoc->contents.at(line)->getString();
		counter+=textline.contains("{")-textline.contains("}");
		line++;
	}
	debug("Range is: %i to %i",midline, line );

	for (n=midline;n<=line;n++) {
		kWriteDoc->contents.at(n)->setVisible(false);
	}

	return line;
}

KWrite::KWrite(KWriteDoc *doc, QWidget *parent, const char *name)
  : QWidget(parent, name) {
  stepLine = -1;  // dbg
  kWriteDoc = doc;
  kWriteView = new KWriteView(this,doc);

  configFlags = 0;
  wrapAt = 78;
  searchFlags = 0;
  replacePrompt = 0L;

  kWriteView->setFocus();
}

KWrite::~KWrite() {
  delete kWriteView;
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
  kWriteView->updateView(ufPos, 0, line*kWriteDoc->fontHeight - height()/2);
  kWriteDoc->updateViews(kWriteView); //update all other views except this one
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

  dlg = new SettingsDialog(configFlags, wrapAt, kWriteDoc->tabChars,
                           kWriteDoc->indentLength, kWriteDoc->undoSteps,
                           topLevelWidget());

  if (dlg->exec() == QDialog::Accepted) {
    bool showTabsChanged = (dlg->getFlags() & cfShowTabs)
                           != (configFlags & cfShowTabs);
    setConfig(dlg->getFlags() | (configFlags & cfOvr));
    wrapAt = dlg->getWrapAt();
    kWriteDoc->setTabWidth(dlg->getTabWidth());
    kWriteDoc->setIndentLength(dlg->getIndentLength());
    kWriteDoc->setUndoSteps(dlg->getUndoSteps());
    if (showTabsChanged)
      kWriteView->tagAll();
    kWriteDoc->updateViews();
  }
  delete dlg;
}

void KWrite::colDlg() {
  ColorDialog *dlg;

  dlg = new ColorDialog(kWriteDoc->colors,topLevelWidget());
  if (dlg->exec() == QDialog::Accepted) {
    dlg->getColors(kWriteDoc->colors);
    kWriteDoc->tagAll();
    kWriteDoc->updateViews();
  }
  delete dlg;
}

void KWrite::toggleVertical() {
  setConfig(configFlags ^ cfVerticalSelect);
  emit statusMsg(configFlags & cfVerticalSelect ? i18n("Vertical Selections On") : i18n("Vertical Selections Off"));
}

void KWrite::toggleOverwrite() {
  setConfig(configFlags ^ cfOvr);
}

QCString KWrite::text() {
  return kWriteDoc->text();
}

QString KWrite::markedText() {
  return kWriteDoc->markedText(configFlags);
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
}

void KWrite::writeFile(QIODevice &dev) {
  kWriteDoc->writeFile(dev);
  kWriteDoc->updateViews();
}


bool KWrite::loadFile(const QString& name, int flags) {
  QFileInfo info(name);
  if (!info.exists()) {
    if (flags & lfNewFile)
      return true;
    KMessageBox::sorry(this, i18n("The specified File does not exist"));
    return false;
  }
  if (info.isDir()) {
    KMessageBox::sorry(this, i18n("You have specified a directory"));
    return false;
  }
  if (!info.isReadable()) {
    KMessageBox::sorry(this, i18n("You do not have read permission to this file:\n" + name));
    return false;
  }

  QFile f(name);
  if (f.open(IO_ReadOnly)) {
    loadFile(f,flags & lfInsert);
    f.close();
    return true;
  }
  KMessageBox::sorry(this, i18n("An Error occured while trying to open this Document"));
  return false;
}

bool KWrite::writeFile(const QString& name) {

  QFileInfo info(name);
  if(info.exists() && !info.isWritable()) {
    KMessageBox::sorry(this, i18n("You do not have write permission to this file:\n" + name));
    return false;
  }

  QFile f(name);
  if (f.open(IO_WriteOnly | IO_Truncate)) {
    writeFile(f);
    f.close();
    return true;//kWriteDoc->setFileName(name);
  }
  KMessageBox::sorry(this,  i18n("An Error occured while trying to open this Document"));
  return false;
}


void KWrite::loadURL(const KURL& url, int flags)
{
  if (url.isMalformed())
  {
    KMessageBox::sorry(this, i18n("Malformed URL\n")+url.prettyURL());
    return;
  }

  if (url.isLocalFile())
  {
    emit statusMsg(i18n("Loading..."));

    QString name(url.path());
    if (loadFile(name,flags)) {
      name = url.url();
      if (flags & lfInsert)
      {
        name.prepend(i18n("Inserted: "));
      }
      else
      {
        if (!(flags & lfNoAutoHl))
          kWriteDoc->setFileName(name);
        else
          kWriteDoc->updateLines();
        name.prepend(i18n("Read: "));
      }
      emit statusMsg(name);
    }
  }
}


void KWrite::writeURL(const KURL& url, int flags)
{
  if (url.isLocalFile())
  {
    emit statusMsg(i18n("Saving..."));

    QString name(url.path());
    if (writeFile(name))
    {
      if (!(flags & lfNoAutoHl))
        kWriteDoc->setFileName(url.url());
      name.prepend(i18n("Wrote: "));
      emit statusMsg(name);
      setModified(false);
    }
  }
}

void KWrite::kfmFinished() {
  if (kfmAction == GET ) {

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

void KWrite::setFileName(const QString& s) {
  kWriteDoc->setFileName(s);
}

bool KWrite::canDiscard() {
  int query;

  if (isModified()) {
    query = KMessageBox::questionYesNo(this,
      i18n("The current Document has been modified.\nWould you like to save it?"),
      i18n("Warning"));

    switch (query)
    {
      case KMessageBox::Yes:
      {
        save();
        if (isModified())
        {
          query = KMessageBox::questionYesNo(this,
            i18n("Could not save the document.\nOpen a new document anyways?"),
            i18n("Sorry"));
          if (query == KMessageBox::No)
            return false; //no
        }
        break;
      }

      default:
        return false;
    }
  }
  return true;
}

void KWrite::newDoc() {

  if (canDiscard())
    clear();
}

void KWrite::open() {
  KURL url;

  if (!canDiscard()) return;

  url = KFileDialog::getOpenURL(kWriteDoc->fileName(),"*");
  if (url.isEmpty())
    return;

  loadURL(url);
}

void KWrite::insertFile() {
  KURL url;

  url = KFileDialog::getOpenURL(kWriteDoc->fileName(),"*");
  if (url.isEmpty())
    return;

  loadURL(url, lfInsert);
}

void KWrite::save() {
  if (isModified()) {
    if (kWriteDoc->hasFileName()) {
      writeURL(kWriteDoc->fileName(),lfNoAutoHl);
    } else saveAs();
  } else emit statusMsg(i18n("No changes need to be saved"));
}

void KWrite::saveAs() {
  KURL url;
  int query;

  do {
    query = 0;
    url = KFileDialog::getSaveURL(kWriteDoc->fileName(),"*");
    if (url.isEmpty())
      return;

    KURL u(url);
    if (u.isLocalFile()) {
      QFileInfo info;
      QString name(u.path());
      info.setFile(name);
      if (info.exists())
      {
        query = KMessageBox::questionYesNo(this,
          i18n("A Document with this Name already exists.\nDo you want to overwrite it?"),
          i18n("Warning"));
      }
    }
  } while (query == KMessageBox::Yes);

  writeURL(url);
}

void KWrite::clear() {
	// Write bookmarks, breakpoints, ...
	kWriteDoc->writeFileConfig();

  kWriteDoc->clear();
  kWriteDoc->clearFileName();
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

void KWrite::comment() {
  VConfig c;
  kWriteView->getVConfig(c);
  kWriteDoc->comment(kWriteView,c);
  kWriteDoc->updateViews();
}

void KWrite::unComment() {
  VConfig c;
  kWriteView->getVConfig(c);
  kWriteDoc->unComment(kWriteView,c);
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

void addToStrList(QStrList &list, const char *str) {
  if (list.find(str) != -1) list.remove();
  if (list.count() >= 16) list.removeLast();
  list.insert(0, str);
}

void KWrite::search() {
  SearchDialog *searchDialog;

  searchDialog = new SearchDialog(&searchForList, 0L,
    searchFlags & ~sfReplace,topLevelWidget());

  // If the user has marked some text we use that otherwise
  // use the word under the cursor.
  QString str;
  if (kWriteDoc && kWriteDoc->hasMarkedText())
    str = markedText();
  if (str.isEmpty())
    str = currentWord();

  if (!str.isEmpty()) {
    str.replace(QRegExp("^\n"), "");
    int pos=str.find("\n");
    if (pos>-1)
     str=str.left(pos);
    searchDialog->setSearchText( str );
  }

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
  if ( kWriteDoc->hasMarkedText() ) {
    QString str = markedText();
    str.replace(QRegExp("^\n"), "");
    int pos=str.find("\n");
    if (pos>-1)
     str=str.left(pos);
    searchDialog->setSearchText( str );
  }

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

void KWrite::searchAgain(bool back) {
  if ( searchForList.isEmpty() ) {
    // search again wanted without searching first...
    search();
    return;
  }
  bool b= (searchFlags & sfBackward) > 0;
  initSearch(s, (searchFlags & ((b==back)?~sfBackward:~0))  // clear flag for forward searching
                | sfFromCursor | sfPrompt | sfAgain | ((b!=back)?sfBackward:0) );
  if (s.flags & sfReplace) replaceAgain(); else searchAgain(s);
}

void KWrite::gotoLine() {
  GotoLineDialog *dlg;

  dlg = new GotoLineDialog(kWriteView->cursor.y + 1,this);
  if (dlg->exec() == QDialog::Accepted) {
		gotoPos(0, dlg->getLine() - 1);
  }
  delete dlg;
}

void KWrite::gotoPos(int cursorXPos, int cursorYPos)
{
  PointStruc cursor;
  cursor.x = cursorXPos;
  cursor.y = cursorYPos;

  kWriteView->updateCursor(cursor);
  kWriteDoc->unmarkFound();
  kWriteView->updateView(ufUpdateOnScroll);
  kWriteDoc->updateViews(kWriteView); //uptade all other views except this one
}

void KWrite::initSearch(SConfig &s, int flags) {
 const char *searchFor = searchForList.getFirst();
 if ( !searchFor )
   return;
  s.flags = flags;
  if (s.flags & sfFromCursor) {
    s.cursor = kWriteView->cursor;
    TextLine *textLine= kWriteDoc->textLine(s.cursor.y);
    const char *line = textLine->getText();
    int (*cmpfct)(const char *, const char *, unsigned int);
    if ( s.flags & sfCaseSensitive )
      cmpfct= qstrncmp;
    else
      cmpfct= qstrnicmp;
    if ( s.flags & sfBackward )
    {
      if ( static_cast<int>(s.cursor.x)-static_cast<int>(strlen(searchFor)) >= 0 )
        if ( cmpfct(line+s.cursor.x-strlen(searchFor), searchFor, strlen(searchFor)) == 0 )
          s.cursor.x-= strlen(searchFor);
    }
    else
    {
      if ( cmpfct(line+s.cursor.x, searchFor, strlen(searchFor)) == 0 )
        s.cursor.x+= strlen(searchFor);
    }
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
  if (slen == 0) {
    search();
    return;
  }
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
          str.sprintf("%s.\n%s?",
            i18n("End of document reached").data(),
            i18n("Continue from the beginning").data());
          query = QMessageBox::information(this,
            i18n("Find"),
            str,
            i18n("Yes"),
            i18n("No"),
            "",0,1);
        } else {
          // backward search
          str.sprintf("%s.\n%s?",
            i18n("Beginning of document reached").data(),
            i18n("Continue from the end").data());
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

	if(xPos < 0) xPos = 0;

  kWriteView->updateView(flags | ufPos,xPos,yPos);
  kWriteDoc->updateViews(kWriteView);
}

void KWrite::deleteReplacePrompt() {
  kWriteDoc->setPseudoModal(0L);
}

bool KWrite::askReplaceEnd() {
  QString str;
  int query;

  kWriteDoc->updateViews();
  if (s.flags & sfFinished) {
    // replace finished
    str.sprintf("%d %s.",
      replaces,
      (i18n("replace(s) made")).data());
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
      replaces,
      i18n("replace(s) made").data(),
      i18n("End of document reached").data(),
      i18n("Continue from the beginning").data());
    query = QMessageBox::information(this,
      i18n("Replace"),
      str,
      i18n("Yes"),
      i18n("No"),
      "",0,1);
  } else {
    // backward search
    str.sprintf("%d %s.\n%s.\n%s?",
      replaces,
      i18n("replace(s) made").data(),
      i18n("Beginning of document reached").data(),
      i18n("Continue from the end").data());
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

void KWrite::installBMPopup(QPopupMenu *p/*KWBookPopup *p*/)
{
  debug("KWrite::installBMPopup");

  connect(p,SIGNAL(aboutToShow()),SLOT(updateBMPopup()));
  connect(p,SIGNAL(activated(int)),SLOT(gotoBookmark(int)));
}

void KWrite::toggleBookmark()
{
	toggleBookmark(kWriteView->cursor.y);
}

void KWrite::toggleBookmark(int line)
{
	TextLine* textline = kWriteDoc->textLine(line);
	if(textline != NULL) textline->toggleBookmark();

  kWriteDoc->tagLines(line, line);
  kWriteDoc->updateViews();
  kWriteView->setFocus();
}

void KWrite::nextBookmark()
{
  int startLine = kWriteView->cursor.y;
 	int lineCount = kWriteDoc->lastLine();
 	int line = startLine;
 	int normalizedLine;
 	do
 	{
 		line++;
 		normalizedLine = line % lineCount;
 	}
 	while(normalizedLine != startLine && !bookmarked(normalizedLine));

 	if(normalizedLine != startLine) gotoPos(0, normalizedLine);
}

void KWrite::previousBookmark()
{
  int startLine = kWriteView->cursor.y;
 	int lineCount = kWriteDoc->lastLine();
 	int line = startLine + lineCount;
 	int normalizedLine;
 	do
 	{
 		line--;
 		normalizedLine = line % lineCount;
 	}
 	while(normalizedLine != startLine && !bookmarked(normalizedLine));

 	if(normalizedLine != startLine) gotoPos(0, normalizedLine);
}

bool KWrite::bookmarked(int line)
{
	TextLine* textline = kWriteDoc->textLine(line);
	if(textline != NULL) return (textline->isBookmarked());

	return false;
}

void KWrite::gotoBookmark(int n)
{
  int currentBookmark = 1;
  if(n >= 1)
    {
      for(int line = 0; line < kWriteDoc->getTextLineCount(); line++)
	{
	  TextLine* textline = kWriteDoc->textLine(line);
	  if(textline != NULL)
	    {
	      if(textline->isBookmarked())
		{
		  if(n == currentBookmark)
		    {
		      gotoPos(0, line);
		      return;
		    }
		  else
		    {
		      currentBookmark++;
		    }
		}
	    }
	}
    }
}

void KWrite::clearBookmarks()
{
  kWriteDoc->clearBookmarks();
}

void KWrite::updateBMPopup()
{
  QPopupMenu* popup = (QPopupMenu *) sender();

  while ((int) popup->count() > 0) {
    popup->removeItemAt(popup->count() - 1);
  }

  popup->insertSeparator();

  for(int line = 0; line < kWriteDoc->getTextLineCount(); line++)
    {
      TextLine* textline = kWriteDoc->textLine(line);
      if(textline != NULL)
	if(textline->isBookmarked())
	  {
	    char buf[64];
	    sprintf(buf,"%s %d",i18n("Line").data(), line + 1);
	    int z = popup->count();
	    popup->insertItem(buf,z);
	    if (z < 9) popup->setAccel(ALT+kw_bookmark_keys[z],z);
	  }
    }
}


void KWrite::readConfig(KConfig *config) {

  searchFlags = config->readNumEntry("SearchFlags",sfPrompt);
  configFlags = config->readNumEntry("ConfigFlags",
    cfPersistent | cfRemoveSpaces | cfAutoIndent | cfIndentBraces | cfBackspaceIndent) & ~cfMark;
  wrapAt = config->readNumEntry("WrapAt",78);
}

void KWrite::writeConfig(KConfig *config) {

  config->writeEntry("SearchFlags",searchFlags);
  config->writeEntry("ConfigFlags",configFlags);
  config->writeEntry("WrapAt",wrapAt);
}

void KWrite::readSessionConfig(KConfig *config) {
  PointStruc cursor;
  QString s2;

  readConfig(config);

  kWriteView->xPos = config->readNumEntry("XPos");
  kWriteView->yPos = config->readNumEntry("YPos");
  cursor.x = config->readNumEntry("CursorX");
  cursor.y = config->readNumEntry("CursorY");
  kWriteView->updateCursor(cursor);
}

void KWrite::writeSessionConfig(KConfig *config) {
  writeConfig(config);

  config->writeEntry("XPos",kWriteView->xPos);
  config->writeEntry("YPos",kWriteView->yPos);
  config->writeEntry("CursorX",kWriteView->cursor.x);
  config->writeEntry("CursorY",kWriteView->cursor.y);
}

void KWrite::hlDef() {
  DefaultsDialog *dlg;
  HlManager *hlManager;
  ItemStyleList defaultStyleList;
  ItemFont defaultFont;

  hlManager = kWriteDoc->hlManager;
  defaultStyleList.setAutoDelete(true);

  hlManager->getDefaults(defaultStyleList,defaultFont);
  dlg = new DefaultsDialog(hlManager,&defaultStyleList,&defaultFont,topLevelWidget());
  if (dlg->exec() == QDialog::Accepted) {
    hlManager->setDefaults(defaultStyleList,defaultFont);
  }
  delete dlg;
}


void KWrite::hlDlg() {
  HighlightDialog *dlg;
  HlManager *hlManager;
  HlDataList hlDataList;

  hlManager = kWriteDoc->hlManager;
  hlDataList.setAutoDelete(true);
  hlManager->getHlDataList(hlDataList);
  dlg = new HighlightDialog(hlManager,&hlDataList,kWriteDoc->getHighlight(),topLevelWidget());
  if (dlg->exec() == QDialog::Accepted) {
    hlManager->setHlDataList(hlDataList);
  }
  delete dlg;
}

int KWrite::getHl() {
  return kWriteDoc->getHighlight();
}

void KWrite::setHl(int n) {
  kWriteDoc->setPreHighlight(n);
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
  KPrinter prt;

  prt.setFullPage(false);

  if (prt.setup()) {
    QPainter paint;
    QPaintDeviceMetrics pdm(&prt);
    fontHeight = kWriteDoc->printFontHeight;
    lines = pdm.height()/fontHeight;
    pages = (kWriteDoc->lastLine() +1)/lines;

    paint.begin(&prt);
    for (c = 1; c <= prt.numCopies(); c++) {
      line = 0;
      
      for (p =  0 ; p <= pages; p++) {
        y = 0;
        if (prt.pageOrder() == KPrinter::LastPageFirst) 
          line = (pages - p)*lines;
        
        for (l = 0; l < lines; l++) {
          kWriteDoc->paintTextLine(paint, line, y, 0, pdm.width(), flags() & cfShowTabs, true);
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

void KWrite::keyPressEvent(QKeyEvent *e) {
  int z;

  if (e->state() & AltButton) {
    for (z = 0; z < 9; z++) {
      if (kw_bookmark_keys[z] == e->key()) gotoBookmark(z+1);
    }
  }
}

void KWrite::paintEvent(QPaintEvent *e) {
  int x, y;

  QRect updateR = e->rect();                    // update rectangle

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

void KWrite::setBreakpoint( int line, int id, bool enabled, bool pending )
{
  TextLine *tx = kWriteDoc->textLine( line );
  if ( tx == 0L )
    return;
  tx->setBPId( id, enabled, pending );
  kWriteDoc->tagLines( line, line );
  kWriteDoc->updateViews();
}

void KWrite::delBreakpoint( int line )
{
  TextLine *tx = kWriteDoc->textLine( line );
  if ( tx == 0L )
    return;
  tx->delBPId();
  kWriteDoc->tagLines( line, line );
  kWriteDoc->updateViews();
}

void KWrite::setStepLine( int line )
{
  PointStruc cursor;
  cursor.x = 0;
  cursor.y = line;
  stepLine = line;

  kWriteView->updateCursor(cursor);
  kWriteDoc->unmarkFound();
  kWriteDoc->tagLines( line, line );
  kWriteDoc->updateViews();
}

void KWrite::clearStepLine()
{
  kWriteDoc->tagLines( stepLine, stepLine );
  stepLine = -1;
  kWriteDoc->updateViews();
}

void KWrite::slotToggleBreakpoint()
{
  emit toggleBreakpoint(QString(kWriteDoc->fileName()), currentLine()+1);
}

void KWrite::pasteStr(QString s)
{
  VConfig c;
  kWriteView->getVConfig(c);
  kWriteDoc->insert(kWriteView,c,s);
  kWriteDoc->updateViews();
}

void KWrite::delMarkedText()
{
  VConfig c;
  kWriteView->getVConfig(c);
  kWriteDoc->delMarkedText(kWriteView,c);
  kWriteDoc->updateViews();
}

QString KWrite::textLine( int i ) const
{
    KWrite* e = (KWrite*) this;
    TextLine* line = e->doc()->textLine( i );
    if( !line )
        return QString::null;

    QString txt = QString::fromLatin1( line->getText(), line->length() );
    return txt;
}

void KWrite::insertText( const QString& s, bool /*mark*/ )
{
    VConfig c;
    kWriteView->getVConfig( c );
    kWriteDoc->insert( kWriteView, c, s );
    kWriteDoc->updateViews();
}

QPoint KWrite::getCursorCoordinates() const
{
    return kWriteView->getCursorCoordinates();
}

#include "kwview.moc"
