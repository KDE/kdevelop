/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _APPOUTPUTWIDGET_H_
#define _APPOUTPUTWIDGET_H_

#include "processwidget.h"

class AppOutputViewPart;

class AppOutputWidget : public ProcessWidget
{
    Q_OBJECT
    
public:
    AppOutputWidget(AppOutputViewPart* part);
    ~AppOutputWidget();

public slots:
	void slotRowSelected(QListBoxItem* row);
	
private:
    virtual void childFinished(bool normal, int status);
	
	AppOutputViewPart* m_part;
};

#endif
