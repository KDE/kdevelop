/***************************************************************************
                         DockSplitter part of KDEStudio
                             -------------------
    copyright            : (C) 1999 by Judin Max
    email                : novaprint@mtu-net.ru
 ***************************************************************************/

#ifndef DOCK_SPLITTER_H
#define DOCK_SPLITTER_H

#include <knewpanner.h>

class DockSplitter : public KNewPanner
{Q_OBJECT
public:
	DockSplitter(QWidget *parent= 0, const char *name= 0,
            Orientation orient = Vertical,
            Units units= Percent,
            int pos= 50);
	~DockSplitter();

  void setSeparatorPos( int pos );
  void activate(QWidget *c0, QWidget *c1 = 0L);
  QWidget* getFirst(){ return first; }
  QWidget* getLast(){ return last; }
  QWidget* getAnother( QWidget* w );
  void updateName();

private:
  QWidget* first;
  QWidget* last;
  Orientation o;
};

#endif




















