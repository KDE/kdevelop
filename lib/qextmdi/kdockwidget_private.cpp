/* This file is part of the KDE libraries
   Copyright (C) 2000 Max Judin <novaprint@mtu-net.ru>
   Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "kdockwidget.h"
#include "kdockwidget_p.h"
#include "kdockwidget_private.h"

#include <qpainter.h>
#include <qcursor.h>
#include <kdebug.h>
#include <qtimer.h>

KDockSplitter::KDockSplitter(QWidget *parent, const char *name, Orientation orient, int pos, bool highResolution)
: QWidget(parent, name)
{
  divider = 0L;
  child0 = 0L;
  child1 = 0L;
  fixedWidth0=-1;
  fixedWidth1=-1;
  fixedHeight0=-1;
  fixedHeight1=-1;

  m_orientation = orient;
  mOpaqueResize = false;
  mKeepSize = false;
  mHighResolution = highResolution;
  setSeparatorPos( pos, false );
  initialised = false;
}

void KDockSplitter::activate(QWidget *c0, QWidget *c1)
{
  if ( c0 ) child0 = c0;
  if ( c1 ) child1 = c1;

  setupMinMaxSize();

  if (divider) delete divider;
  divider = new QFrame(this, "pannerdivider");
  divider->setFrameStyle(QFrame::Panel | QFrame::Raised);
  divider->setLineWidth(1);
  divider->raise();

  if (m_orientation == Horizontal)
    divider->setCursor(QCursor(sizeVerCursor));
  else
    divider->setCursor(QCursor(sizeHorCursor));

  divider->installEventFilter(this);

  initialised= true;

  updateName();

  divider->show();
  resizeEvent(0);
  if (fixedWidth0!=-1) restoreFromForcedFixedSize((KDockWidget*)child0);
  if (fixedWidth1!=-1) restoreFromForcedFixedSize((KDockWidget*)child1);
  if (((KDockWidget*)child0)->forcedFixedWidth()!=-1)
  {
  	setForcedFixedWidth(((KDockWidget*)child0),((KDockWidget*)child0)->forcedFixedWidth());
	//QTimer::singleShot(100,this,SLOT(delayedResize()));
  }
  else
  if (((KDockWidget*)child1)->forcedFixedWidth()!=-1)
  {
  	setForcedFixedWidth(((KDockWidget*)child1),((KDockWidget*)child1)->forcedFixedWidth());
	//QTimer::singleShot(100,this,SLOT(delayedResize()));
  }

  if (((KDockWidget*)child0)->forcedFixedHeight()!=-1)
  {
        setForcedFixedHeight(((KDockWidget*)child0),((KDockWidget*)child0)->forcedFixedHeight());
        //QTimer::singleShot(100,this,SLOT(delayedResize()));
  }
  else
  if (((KDockWidget*)child1)->forcedFixedHeight()!=-1)
  {
        setForcedFixedHeight(((KDockWidget*)child1),((KDockWidget*)child1)->forcedFixedHeight());
        //QTimer::singleShot(100,this,SLOT(delayedResize()));
  }


}

/*
void KDockSplitter::delayedResize()
{
	kdDebug()<<"*********************** DELAYED RESIZE !!!!!!!!!!!!!!!"<<endl;
	resizeEvent(0);
}*/

void KDockSplitter::setForcedFixedWidth(KDockWidget *dw,int w)
{
	int factor = (mHighResolution)? 10000:100;
	if (dw==child0)
	{
                fixedWidth0=w;
//		setupMinMaxSize();
                savedXPos=xpos;
		setSeparatorPos(w*factor/width(),true);
//		kdDebug()<<"Set forced fixed width for widget 0 :"<<w<<endl;
	}
        else
	{
                fixedWidth1=w;
                savedXPos=xpos;
		setSeparatorPos((width()-w)*factor/width(),true);
//		kdDebug()<<"Set forced fixed width for widget 1 :"<<w<<endl;
//		kdDebug()<<"Width() :"<<width()<<endl;
	}
}

void KDockSplitter::setForcedFixedHeight(KDockWidget *dw,int h)
{
	int factor = (mHighResolution)? 10000:100;
	if (dw==child0)
	{
                fixedHeight0=h;
//		setupMinMaxSize();
		savedXPos=xpos;
		setSeparatorPos(h*factor/height(),true);
//		kdDebug()<<"Set forced fixed width for widget 0 :"<<h<<endl;
	}
        else
	{
                fixedHeight1=h;
		savedXPos=xpos;
		setSeparatorPos((height()-h)*factor/height(),true);
//		kdDebug()<<"Set forced fixed height for widget 1 :"<<h<<endl;
	}
}

void KDockSplitter::restoreFromForcedFixedSize(KDockWidget *dw)
{
	if (dw==child0)
	{
		fixedWidth0=-1;
		fixedHeight0=-1;
		setSeparatorPos(savedXPos,true);
	}
	else
	{
		fixedWidth1=-1;
		fixedHeight1=-1;
		setSeparatorPos(savedXPos,true);
	}
}


void KDockSplitter::setupMinMaxSize()
{
  // Set the minimum and maximum sizes
  int minx, maxx, miny, maxy;
  if (m_orientation == Horizontal) {
    miny = child0->minimumSize().height() + child1->minimumSize().height()+4;
    maxy = child0->maximumSize().height() + child1->maximumSize().height()+4;
    minx = (child0->minimumSize().width() > child1->minimumSize().width()) ? child0->minimumSize().width() : child1->minimumSize().width();
    maxx = (child0->maximumSize().width() > child1->maximumSize().width()) ? child0->maximumSize().width() : child1->maximumSize().width();

    miny = (miny > 4) ? miny : 4;
    maxy = (maxy < 32000) ? maxy : 32000;
    minx = (minx > 2) ? minx : 2;
    maxx = (maxx < 32000) ? maxx : 32000;
  } else {
    minx = child0->minimumSize().width() + child1->minimumSize().width()+4;
    maxx = child0->maximumSize().width() + child1->maximumSize().width()+4;
    miny = (child0->minimumSize().height() > child1->minimumSize().height()) ? child0->minimumSize().height() : child1->minimumSize().height();
    maxy = (child0->maximumSize().height() > child1->maximumSize().height()) ? child0->maximumSize().height() : child1->maximumSize().height();

    minx = (minx > 4) ? minx : 4;
    maxx = (maxx < 32000) ? maxx : 32000;
    miny = (miny > 2) ? miny : 2;
    maxy = (maxy < 32000) ? maxy : 32000;
  }
  setMinimumSize(minx, miny);
  setMaximumSize(maxx, maxy);
}

void KDockSplitter::deactivate()
{
  delete divider;
  divider = 0L;
  initialised= false;
}

void KDockSplitter::setSeparatorPos(int pos, bool do_resize)
{
  xpos = pos;
  if (do_resize)
    resizeEvent(0);
}

void KDockSplitter::setSeparatorPosX( int pos, bool do_resize )
{
  savedXPos = pos;
  setSeparatorPos( pos, do_resize );
}

int KDockSplitter::separatorPos() const
{
  return xpos;
}

void KDockSplitter::resizeEvent(QResizeEvent *ev)
{
//  kdDebug()<<"ResizeEvent :"<< ((initialised) ? "initialised":"not initialised")<<", "<< ((ev) ? "real event":"")<<
//	", "<<(isVisible() ?"visible":"")<<endl;
  if (initialised){
    int factor = (mHighResolution)? 10000:100;
    // real resize event, recalculate xpos
    if (ev && mKeepSize && isVisible()) {
//	kdDebug()<<"mKeepSize : "<< ((m_orientation == Horizontal) ? "Horizontal":"Vertical") <<endl;

      if (ev->oldSize().width() != ev->size().width())
      {
          if (m_orientation == Horizontal) {
          xpos = factor * checkValue( child0->height()+1 ) / height();
          } else {
          xpos = factor * checkValue( child0->width()+1 ) / width();
	  }

          }
      }
          else
          {
//	kdDebug()<<"!mKeepSize : "<< ((m_orientation == Horizontal) ? "Horizontal":"Vertical") <<endl;
	if (/*ev &&*/ isVisible()) {
		if (m_orientation == Horizontal) {
		/*	if (ev->oldSize().height() != ev->size().height())*/
			{
			  if (fixedHeight0!=-1)
				xpos=fixedHeight0*factor/height();
			  else
			  if (fixedHeight1!=-1)
				xpos=(height()-fixedHeight1)*factor/height();
			}
		}
		else
		{
/*	        	if (ev->oldSize().width() != ev->size().width()) */
			{
			  if (fixedWidth0!=-1)
				xpos=fixedWidth0*factor/width();
			  else
			  if (fixedWidth1!=-1)
				xpos=(width()-fixedWidth1)*factor/width();
			}
		}
	}
//	else kdDebug()<<"Something else happened"<<endl;
   }

    KDockContainer *dc;
    KDockWidget *c0=(KDockWidget*)child0;
    KDockWidget *c1=(KDockWidget*)child1;
    bool stdHandling=false;
    if ((fixedWidth0==-1) && (fixedWidth1==-1)) {
	    if ((c0->getWidget()) && (dc=dynamic_cast<KDockContainer*>(c0->getWidget()))
		 && (dc->m_overlapMode)) {
			int position= (m_orientation == Vertical ? width() : height()) * xpos/factor;
			position=checkValueOverlapped(position,child0);
			child0->raise();
			divider->raise();
	        	      if (m_orientation == Horizontal){
        	        	child0->setGeometry(0, 0, width(), position);
	                	child1->setGeometry(0, dc->m_nonOverlapSize+4, width(),
						height()-dc->m_nonOverlapSize-4);
	        	        divider->setGeometry(0, position, width(), 4);
	        	      } else {
        	        	child0->setGeometry(0, 0, position, height());
		                child1->setGeometry(dc->m_nonOverlapSize+4, 0,
						width()-dc->m_nonOverlapSize-4, height());
        		        divider->setGeometry(position, 0, 4, height());
		              }
	    } else {
		 if ((c1->getWidget()) && (dc=dynamic_cast<KDockContainer*>(c1->getWidget()))
        	 && (dc->m_overlapMode)) {
                	int position= (m_orientation == Vertical ? width() : height()) * xpos/factor;
			position=checkValueOverlapped(position,child1);
	                child1->raise();
        	        divider->raise();
	                      if (m_orientation == Horizontal){
        	                child0->setGeometry(0, 0, width(), height()-dc->m_nonOverlapSize-4);
                	        child1->setGeometry(0, position+4, width(),
	                                        height()-position-4);
        	                divider->setGeometry(0, position, width(), 4);
                	      } else {
                        	child0->setGeometry(0, 0, width()-dc->m_nonOverlapSize-4, height());
	                        child1->setGeometry(position+4, 0,
        	                                width()-position-4, height());
                	        divider->setGeometry(position, 0, 4, height());
	                      }
		}
		else stdHandling=true;
	      }
            }
	 else stdHandling=true;

	if (stdHandling) {
		      int position = checkValue( (m_orientation == Vertical ? width() : height()) * xpos/factor );
		      if (m_orientation == Horizontal){
        		child0->setGeometry(0, 0, width(), position);
		        child1->setGeometry(0, position+4, width(), height()-position-4);
        		divider->setGeometry(0, position, width(), 4);
		      } else {
        		child0->setGeometry(0, 0, position, height());
	        	child1->setGeometry(position+4, 0, width()-position-4, height());
	        	divider->setGeometry(position, 0, 4, height());
	}

	}

  }
}

int KDockSplitter::checkValueOverlapped(int position, QWidget *overlappingWidget) const {
	if (initialised) {
		if (m_orientation == Vertical) {
			if (child0==overlappingWidget) {
				if (position<(child0->minimumSize().width()))
					position=child0->minimumSize().width();
				if (position>width()) position=width()-4;
			} else if (position>(width()-(child1->minimumSize().width())-4)){
				position=width()-(child1->minimumSize().width())-4;
				if (position<0) position=0;
			}
		} else {// orientation  == Horizontal
			if (child0==overlappingWidget) {
				if (position<(child0->minimumSize().height()))
					position=child0->minimumSize().height();
				if (position>height()) position=height()-4;
			} else if (position>(height()-(child1->minimumSize().height())-4)){
				position=height()-(child1->minimumSize().height())-4;
				if (position<0) position=0;

			}
		}

	}
	return position;
}

int KDockSplitter::checkValue( int position ) const
{
  if (initialised){
    if (m_orientation == Vertical){
      if (position < (child0->minimumSize().width()))
        position = child0->minimumSize().width();
      if ((width()-4-position) < (child1->minimumSize().width()))
        position = width() - (child1->minimumSize().width()) -4;
    } else {
      if (position < (child0->minimumSize().height()))
        position = (child0->minimumSize().height());
      if ((height()-4-position) < (child1->minimumSize().height()))
        position = height() - (child1->minimumSize().height()) -4;
    }
  }

  if (position < 0) position = 0;

  if ((m_orientation == Vertical) && (position > width()))
    position = width();
  if ((m_orientation == Horizontal) && (position > height()))
    position = height();

  return position;
}

bool KDockSplitter::eventFilter(QObject *o, QEvent *e)
{
  QMouseEvent *mev;
  bool handled = false;
  int factor = (mHighResolution)? 10000:100;

  switch (e->type()) {
    case QEvent::MouseMove:
      mev= (QMouseEvent*)e;
      child0->setUpdatesEnabled(mOpaqueResize);
      child1->setUpdatesEnabled(mOpaqueResize);
      if (m_orientation == Horizontal) {
        if ((fixedHeight0!=-1) || (fixedHeight1!=-1))
        {
                handled=true; break;
        }

	if (!mOpaqueResize) {
          int position = checkValue( mapFromGlobal(mev->globalPos()).y() );
          divider->move( 0, position );
        } else {
          xpos = factor * checkValue( mapFromGlobal(mev->globalPos()).y() ) / height();
          resizeEvent(0);
          divider->repaint(true);
        }
      } else {
        if ((fixedWidth0!=-1) || (fixedWidth1!=-1))
        {
                handled=true; break;
        }
        if (!mOpaqueResize) {
          int position = checkValue( mapFromGlobal(QCursor::pos()).x() );
          divider->move( position, 0 );
        } else {
          xpos = factor * checkValue( mapFromGlobal( mev->globalPos()).x() ) / width();
          resizeEvent(0);
          divider->repaint(true);
        }
      }
      handled= true;
      break;
    case QEvent::MouseButtonRelease:
      child0->setUpdatesEnabled(true);
      child1->setUpdatesEnabled(true);
      mev= (QMouseEvent*)e;
      if (m_orientation == Horizontal){
        if ((fixedHeight0!=-1) || (fixedHeight1!=-1))
        {
                handled=true; break;
        }
        xpos = factor* checkValue( mapFromGlobal(mev->globalPos()).y() ) / height();
        resizeEvent(0);
        divider->repaint(true);
      } else {
        if ((fixedWidth0!=-1) || (fixedWidth1!=-1))
        {
                handled=true; break;
        }
        xpos = factor* checkValue( mapFromGlobal(mev->globalPos()).x() ) / width();
        resizeEvent(0);
        divider->repaint(true);
      }
      handled= true;
      break;
    default:
      break;
  }
  return (handled) ? true : QWidget::eventFilter( o, e );
}

bool KDockSplitter::event( QEvent* e )
{
  if ( e->type() == QEvent::LayoutHint ){
    // change children min/max size
    setupMinMaxSize();
    setSeparatorPos(xpos);
  }
  return QWidget::event(e);
}

QWidget* KDockSplitter::getAnother( QWidget* w ) const
{
  return ( w == child0 ) ? child1 : child0;
}

void KDockSplitter::updateName()
{
  if ( !initialised ) return;

  QString new_name = QString( child0->name() ) + "," + child1->name();
  parentWidget()->setName( new_name.latin1() );
  parentWidget()->setCaption( child0->caption() + "," + child1->caption() );
  parentWidget()->repaint( false );

  ((KDockWidget*)parentWidget())->firstName = child0->name();
  ((KDockWidget*)parentWidget())->lastName = child1->name();
  ((KDockWidget*)parentWidget())->splitterOrientation = m_orientation;

  QWidget* p = parentWidget()->parentWidget();
  if ( p != 0L && p->inherits("KDockSplitter" ) )
    ((KDockSplitter*)p)->updateName();
}

void KDockSplitter::setOpaqueResize(bool b)
{
  mOpaqueResize = b;
}

bool KDockSplitter::opaqueResize() const
{
  return mOpaqueResize;
}

void KDockSplitter::setKeepSize(bool b)
{
  mKeepSize = b;
}

bool KDockSplitter::keepSize() const
{
  return mKeepSize;
}

void KDockSplitter::setHighResolution(bool b)
{
  if (mHighResolution) {
    if (!b) xpos = xpos/100;
  } else {
    if (b) xpos = xpos*100;
  }
  mHighResolution = b;
}

bool KDockSplitter::highResolution() const
{
  return mHighResolution;
}


/*************************************************************************/
KDockButton_Private::KDockButton_Private( QWidget *parent, const char * name )
:QPushButton( parent, name )
{
  moveMouse = false;
  setFocusPolicy( NoFocus );
}

KDockButton_Private::~KDockButton_Private()
{
}

void KDockButton_Private::drawButton( QPainter* p )
{
  p->fillRect( 0,0, width(), height(), QBrush(colorGroup().brush(QColorGroup::Background)) );
  p->drawPixmap( (width() - pixmap()->width()) / 2, (height() - pixmap()->height()) / 2, *pixmap() );
  if ( moveMouse && !isDown() ){
    p->setPen( white );
    p->moveTo( 0, height() - 1 );
    p->lineTo( 0, 0 );
    p->lineTo( width() - 1, 0 );

    p->setPen( colorGroup().dark() );
    p->lineTo( width() - 1, height() - 1 );
    p->lineTo( 0, height() - 1 );
  }
  if ( isOn() || isDown() ){
    p->setPen( colorGroup().dark() );
    p->moveTo( 0, height() - 1 );
    p->lineTo( 0, 0 );
    p->lineTo( width() - 1, 0 );

    p->setPen( white );
    p->lineTo( width() - 1, height() - 1 );
    p->lineTo( 0, height() - 1 );
  }
}

void KDockButton_Private::enterEvent( QEvent * )
{
  moveMouse = true;
  repaint();
}

void KDockButton_Private::leaveEvent( QEvent * )
{
  moveMouse = false;
  repaint();
}

/*************************************************************************/
KDockWidgetPrivate::KDockWidgetPrivate()
  : QObject()
  ,index(-1)
  ,splitPosInPercent(50)
  ,pendingFocusInEvent(false)
  ,blockHasUndockedSignal(false)
  ,pendingDtor(false)
  ,forcedWidth(-1)
  ,forcedHeight(-1)
  ,isContainer(false)
  ,container(0)
  ,resizePos(0,0)
  ,resizing(false)
{
#ifndef NO_KDE2
  windowType = NET::Normal;
#endif

  _parent = 0L;
  transient = false;
}

KDockWidgetPrivate::~KDockWidgetPrivate()
{
}

void KDockWidgetPrivate::slotFocusEmbeddedWidget(QWidget* w)
{
   if (w) {
      QWidget* embeddedWdg = ((KDockWidget*)w)->getWidget();
      if (embeddedWdg && ((embeddedWdg->focusPolicy() == QWidget::ClickFocus) || (embeddedWdg->focusPolicy() == QWidget::StrongFocus))) {
         embeddedWdg->setFocus();
      }
   }
}

#ifndef NO_INCLUDE_MOCFILES // for Qt-only projects, because tmake doesn't take this name
#include "kdockwidget_private.moc"
#endif
