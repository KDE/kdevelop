/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@mksat.net                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef TROLLLISTVIEW_H
#define TROLLLISTVIEW_H

#include <klistview.h>

#include "trollprojectwidget.h"

class TrollListView : public KListView
{
Q_OBJECT
public:
    TrollListView(TrollProjectWidget *widget, QWidget *parent, TrollProjectWidget::TrollProjectView view, const char *name = 0);
    ~TrollListView();
protected:
    virtual void focusOutEvent(QFocusEvent *e);

private:
    TrollProjectWidget *m_widget;
    TrollProjectWidget::TrollProjectView m_view;
};

#endif
