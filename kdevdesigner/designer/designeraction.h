/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
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
#ifndef DESIGNERACTION_H
#define DESIGNERACTION_H

#include <qaction.h>

class DesignerAction : public QAction
{
Q_OBJECT
public:
    DesignerAction(QObject *parent = 0, const char *name = 0)
            :QAction(parent, name) {}
    DesignerAction ( const QString & menuText, QKeySequence accel, QObject * parent, const char * name = 0 )
            :QAction(menuText, accel, parent, name) {}
    DesignerAction ( const QIconSet & icon, const QString & menuText, QKeySequence accel, QObject * parent, const char * name = 0 )
            :QAction(icon, menuText, accel, parent, name) {}
    DesignerAction ( const QString & text, const QIconSet & icon, const QString & menuText, QKeySequence accel, QObject * parent, const char * name = 0, bool toggle = FALSE ) 
            :QAction(text, icon, menuText, accel, parent, name, toggle) {}
    DesignerAction ( const QString & text, const QString & menuText, QKeySequence accel, QObject * parent, const char * name = 0, bool toggle = FALSE )
            :QAction(text, menuText, accel, parent, name, toggle) {}
    DesignerAction ( QObject * parent, const char * name, bool toggle ) 
            :QAction(parent, name, toggle) {}
    ~DesignerAction();
    
    virtual void setEnabled(bool e);
    virtual void setDisabled(bool d);
#if QT_VERSION < 0x030300
    virtual void activate();
#endif

signals:
    void actionEnabled(bool e);
    
};

#endif
