/***************************************************************************
 *   Copyright (C) 2004 by Tobias Glaesser                                 *
 *   tobi.web@gmx.de                                                       *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef FIND_DOCUMENTATIONOPTIONS_H
#define FIND_DOCUMENTATIONOPTIONS_H

#include "find_documentation_optionsbase.h"

class QCheckListItem;
class KConfig;

class FindDocumentationOptions : public FindDocumentationOptionsBase
{
Q_OBJECT
public:
    FindDocumentationOptions(QWidget* parent = 0, const char* name = 0, bool modal = FALSE);
    ~FindDocumentationOptions();
    /*$PUBLIC_FUNCTIONS$*/
    virtual bool isContents( QCheckListItem * item );
    virtual bool isGoogle( QCheckListItem * item );
    virtual bool isIndex( QCheckListItem * item );
    virtual bool isInfo( QCheckListItem * item );
    virtual bool isMan( QCheckListItem * item );

public slots:
    /*$PUBLIC_SLOTS$*/
    virtual void sourceMoveDown();
    virtual void sourceMoveUp();
    virtual void writeOptions();
    virtual void readOptions();

protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/

private:
    QCheckListItem *m_man_item;
    QCheckListItem *m_info_item;
    QCheckListItem *m_index_item;
    QCheckListItem *m_google_item;
    QCheckListItem *m_contents_item;
    
    KConfig* config;    
};

#endif

