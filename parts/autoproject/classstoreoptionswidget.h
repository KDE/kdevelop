/***************************************************************************
 *   Copyright (C) 2001 by Daniel Engelschalt                              *
 *   daniel.engelschalt@gmx.net                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CLASSSTOREOPTIONSWIDGET_H_
#define _CLASSSTOREOPTIONSWIDGET_H_

#include "classstoreoptionswidgetbase.h"

class AutoProjectPart;

class ClassStoreOptionsWidget : public ClassStoreOptionsWidgetBase
{
    Q_OBJECT
    
public:
    ClassStoreOptionsWidget( AutoProjectPart* part, QWidget* parent = 0, const char* name = 0 );
    ~ClassStoreOptionsWidget( );

public slots:
    void accept( );
    
protected slots:
    void slotEnablePCS( bool );
    void slotEnablePreParsing( );
    void slotAddPreParsingPath( );
    void slotRemovePreParsingPath( );

private:
    AutoProjectPart *m_pPart;
};

#endif
