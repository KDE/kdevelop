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

#ifndef _CCCONFIGWIDGET_H_
#define _CCCONFIGWIDGET_H_

#include "ccconfigwidgetbase.h"

class CppSupportPart;

class CCConfigWidget : public CCConfigWidgetBase
{
    Q_OBJECT
    
public:
    CCConfigWidget( CppSupportPart* part, QWidget* parent = 0, const char* name = 0 );
    ~CCConfigWidget( );


public slots:
    void accept( );
    void slotEnableCH( bool );
    void slotEnableCC( bool );
    void slotAddPPPath( void );
    void slotEnablePCS( bool );
    void slotEnablePP( bool );
    void slotRemovePPPath( void );
    void slotSetCHWindow( int );

    
signals:
    void enableCodeCompletion( bool setEnable );
    void enableCodeHinting( bool setEnable, bool setOutputView );


private:
    void initCCTab( );
    void initCSTab( );
    void saveCCTab( );
    void saveCSTab( );
    
    CppSupportPart* m_pPart;
};

#endif
