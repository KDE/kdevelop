/***************************************************************************
                          phphtmlview.h  -  description
                             -------------------
    begin                : Sat May 26 2001
    copyright            : (C) 2001 by 
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

#ifndef PHPHTMLVIEW_H
#define PHPHTMLVIEW_H

#include <kdevhtmlpart.h>

/**
  *@author 
  */
class PHPSupportPart;
  
class PHPHTMLView : public KDevHTMLPart  {
Q_OBJECT
public: 
	PHPHTMLView(PHPSupportPart *part);
	~PHPHTMLView();
protected slots:

    virtual void slotDuplicate();
    virtual void slotOpenInNewWindow(const KURL &url);
    
private:
    PHPSupportPart *m_part;
};

#endif
