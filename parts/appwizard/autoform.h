/***************************************************************************
 *   Copyright (C) 2004 by Ian Reinhart Geiser                             *
 *   geiseri@kde.org                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef AUTOFORM_H
#define AUTOFORM_H

#include <qscrollview.h>
#include <dataform.h>
class KPushButton;
class QVBox;

class autoKey : public key
{
public:
	autoKey( const QString &w="", const QString &l = "") : key(w,"value") , label(l) 
	{ if( label.isEmpty() ) label = QString( "Value for %1" ).arg(w);}
	QString label;
};

typedef QMap<autoKey,QVariant> AutoPropertyMap;

/**
A QWidget that automaticly constructs a form from a PropertyMap.

@author Ian Reinhart Geiser
*/
class AutoForm : public QScrollView
{
Q_OBJECT
public:
    AutoForm(AutoPropertyMap *map, QWidget *parent = 0, const char *name = 0);
    ~AutoForm();
    
    /**
    * Provides access to the internal @ref dataForm
    */
    DataForm *dataForm() const { return m_dataForm; }

signals:
    void mapChanged();
    
private slots:
    void slotMapChanged();
protected:
	void viewportResizeEvent( QResizeEvent *ev );
private:
    void buildGUI( AutoPropertyMap *map );
    KPushButton *m_reset;
    KPushButton *m_submit;
    DataForm *m_dataForm;
    QVBox *m_mainBox;

};

#endif
