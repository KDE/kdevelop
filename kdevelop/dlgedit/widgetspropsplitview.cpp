/***************************************************************************
                          widgetspropsplitview.cpp  -  description
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

#include "widgetspropsplitview.h"

WidgetsPropSplitView::WidgetsPropSplitView(QWidget *parent, const char *name ) : QWidget(parent,name) {
    split = new QSplitter(this);
    widgets_view = new WidgetsView(split);
    prop_view = new PropertyView(split);
    split->setOrientation(QSplitter::Vertical);
}

void WidgetsPropSplitView::resizeEvent (QResizeEvent *e){
    split->resize(e->size());
}
WidgetsPropSplitView::~WidgetsPropSplitView(){
}
WidgetsView* WidgetsPropSplitView::getWidgetsView(){
    return widgets_view;
}
PropertyView* WidgetsPropSplitView::getPropertyView(){
    return prop_view;
}
