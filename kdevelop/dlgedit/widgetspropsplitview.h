/***************************************************************************
                          widgetspropsplitview.h  -  description
                             -------------------
    begin                : Mon Jan 24 2000
    copyright            : (C) 2000 by 
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WIDGETSPROPSPLITVIEW_H
#define WIDGETSPROPSPLITVIEW_H

#include <qwidget.h>
#include <qsplitter.h>
#include "widgetsview.h"
#include "propertyview.h"

/**
  *@author 
  */

class WidgetsPropSplitView : public QWidget  {
   Q_OBJECT
public: 
   WidgetsPropSplitView(QWidget *parent=0, const char *name=0);
   ~WidgetsPropSplitView();
   PropertyView* getPropertyView();
   WidgetsView* getWidgetsView();
   
   virtual void resizeEvent (QResizeEvent *e);
 private:
   QSplitter* split;
   PropertyView* prop_view;
   WidgetsView* widgets_view;
   
};

#endif
