/***************************************************************************
                             editorconfigwidget.h
                             ----------------------
    copyright            : (C) 2000 by KDevelop team
    email                : kdevelop_team@kdevelop.org

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef _EDITORCONFIGWIDGET_H_
#define _EDITORCONFIGWIDGET_H_


#include <qwidget.h>


class EditorConfigWidget : public QWidget
{
    Q_OBJECT
	
public: 
    EditorConfigWidget( QWidget *parent, const char *name=0 );
    ~EditorConfigWidget();
};

#endif
