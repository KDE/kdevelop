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
    void slotEnableCC ( );
    void slotEnablePCS( bool );    
    void slotEnablePP ( bool );
    void slotAddPPPath( );    
    void slotRemovePPPath( );    
    void slotEnableCH ( bool );
    void slotSetCHWindow( );
    void slotEnableChooseFiles(bool);
    
    void slotSelectTemplateGroup(const QString &);
    void interfaceFile();
    void implementationFile();

    void accept( );
    
signals:
    void enablePersistantClassStore( bool setEnable );
    void enablePreParsing ( bool setEnable );
    void changedPreParsingPath( );

    void enableCodeCompletion( bool setEnable );
    void enableCodeHinting( bool setEnable, bool setOutputView );

private:
    void initCCTab( );
    void initCSTab( );
    void initFTTab( );
    void saveCCTab( );
    void saveCSTab( );
    void saveFTTab( );
    
    CppSupportPart* m_pPart;
    
    bool m_bChangedCC;
    bool m_bChangedCH;
    bool m_bChangedCHWindow;
    bool m_bChangedPCS;
    bool m_bChangedPP;
    bool m_bChangedPPPath;
    int  m_iCHWindow;
    
};

#endif
