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

class JavaSupportPart;

class CCConfigWidget : public CCConfigWidgetBase
{
    Q_OBJECT
    
public:
    CCConfigWidget( JavaSupportPart* part, QWidget* parent = 0, const char* name = 0 );
    ~CCConfigWidget( );

public slots:
    void slotEnableCC ( );
    
    void slotSelectTemplateGroup(const QString &);
    void interfaceFile();
    void implementationFile();
    void slotEnableChooseFiles(bool c);

    void accept( );
    
signals:
    void enableCodeCompletion( bool setEnable );

private:
    void initCCTab( );
    void initFTTab( );
    void saveCCTab( );
    void saveCSTab( );
    void saveFTTab( );
    
private:
    JavaSupportPart* m_pPart;
    bool m_bChangedCC;    
};

#endif
