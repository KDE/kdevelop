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
class QButtonGroup;


class CCConfigWidget : public CCConfigWidgetBase
{
    Q_OBJECT
    
public:
    CCConfigWidget( CppSupportPart* part, QWidget* parent = 0, const char* name = 0 );
    ~CCConfigWidget( );

public slots:
    void accept( );
    void slotEnableCH( );
    
signals:
    void enableCodeCompletion( bool setEnable );
    void enableCodeHinting( bool setEnable, bool setOutputView );

private:
    CppSupportPart* m_pPart;
    QButtonGroup *bgCodeHinting;
};

#endif
