/***************************************************************************
                          phphtmlview.cpp  -  description
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

#include "phphtmlview.h"

#include "phpsupportpart.h"
#include "kdevpartcontroller.h"

PHPHTMLView::PHPHTMLView(PHPSupportPart *part):m_part(part) {
   setOptions(CanOpenInNewWindow);
}
PHPHTMLView::~PHPHTMLView(){
}

void PHPHTMLView::slotDuplicate( )
{
   m_part->partController()->showDocument(url(), true);
}

void PHPHTMLView::slotOpenInNewWindow( const KURL & url )
{
   m_part->partController()->showDocument(url, true);
}

#include "phphtmlview.moc"
