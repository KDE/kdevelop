/***************************************************************************
                         DockSplitter part of KDEStudio
                             -------------------
    copyright            : (C) 1999 by Judin Max
    email                : novaprint@mtu-net.ru
 ***************************************************************************/

#include "docksplitter.h"
#include "dockmanager.h"
#include <qapplication.h>

DockSplitter::DockSplitter(QWidget *parent, const char *name, Orientation orient, Units units, int pos)
: KNewPanner( parent, name, orient, units, pos )
{
  o = orient;
}

DockSplitter::~DockSplitter()
{
}

QWidget* DockSplitter::getAnother( QWidget* w )
{
  return ( w == first ) ? last : first;
}

void DockSplitter::activate( QWidget *c0, QWidget *c1 )
{
  if ( c0 != 0L ) first = c0;
  if ( c1 != 0L ) last = c1;

  updateName();

  KNewPanner::activate( first, last );
  resizeEvent(0);
}

void DockSplitter::setSeparatorPos( int pos )
{
  KNewPanner::setSeparatorPos( pos );
  resizeEvent(0);
}

void DockSplitter::updateName()
{
  parentWidget()->setName( QString( first->name() ) + "," + last->name() );
  parentWidget()->repaint( false );
  ((DockWidget*)parentWidget())->firstName = first->name();
  ((DockWidget*)parentWidget())->lastName = last->name();
  if ( o == Vertical )
    ((DockWidget*)parentWidget())->splitterOrientation = 0;
  else
    ((DockWidget*)parentWidget())->splitterOrientation = 1;

  QWidget* p = (DockSplitter*)parent()->parent();
  if ( p != 0L )
    if ( p->isA("DockSplitter") ) ((DockSplitter*)p)->updateName();

}


























