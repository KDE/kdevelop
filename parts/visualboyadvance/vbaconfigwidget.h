/***************************************************************************
                          vbaconfigwidget.h  -  VisualBoy Advance configuration Widget
                             -------------------
    begin                : Thu Nov 29 2001
    copyright            : (C) 2001 by Sandy Meier
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef VBACONFIGWIDGET_H
#define VBACONFIGWIDGET_H

#include <qwidget.h>
#include "vbaconfigwidgetbase.h"
#include "visualboyadvance_part.h"

namespace VisualBoyAdvance {
/**
  *@author 
  */

class VBAConfigWidget : public VBAConfigWidgetBase  {
   Q_OBJECT
public: 
	VBAConfigWidget(VisualBoyAdvancePart* part,QWidget *parent=0, const char *name=0);
	~VBAConfigWidget();
public slots:
    void accept();
 void emuPathButtonClicked();
 private:
	VisualBoyAdvancePart* m_part;
};
}
#endif
