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
#include "autoform.h"
#include <kpushbutton.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qvariant.h>
#include <kdebug.h>
#include <propeditor/propertywidgetproxy.h>
#include <kdialog.h>

AutoForm::AutoForm(AutoPropertyMap *map, QWidget *parent, const char *name)
 : QScrollView(parent, name)
{
	m_dataForm = new DataForm(this, "dataForm");
	m_dataForm->setMap( (PropertyMap *)map );
	m_dataForm->setForm( this );
	m_mainBox = new QVBox( viewport() );
	addChild( m_mainBox );
	m_mainBox->setSpacing( KDialog::spacingHint() );
	m_mainBox->setMargin( KDialog::marginHint() );
	
	buildGUI( map );

	connect( m_dataForm, SIGNAL( mapChanged() ), this, SLOT( slotMapChanged() ) );
	m_dataForm->updateView();
}


AutoForm::~AutoForm()
{
}

void AutoForm::buildGUI( AutoPropertyMap *map )
{
	AutoPropertyMap::Iterator idx = map->begin();
	for( ; idx != map->end(); ++idx )
	{
		QHBox *line = new QHBox(m_mainBox);
		// Key Name | Property Edit
		QString name =  QString("%1 %2").arg( idx.key().widget ).arg( idx.key().property );
		new QLabel(idx.key().label, line, name.latin1() );
		PropertyLib::PropertyWidgetProxy *prop = new PropertyLib::PropertyWidgetProxy( line, idx.key().widget.latin1() );
		line->setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed ) );
		m_mainBox->setSpacing( KDialog::spacingHint() );
	}
	QHBox *line = new QHBox(m_mainBox);
	m_reset = new KPushButton("Reset", line);
	m_submit = new KPushButton("Submit", line);
	connect( m_reset, SIGNAL( clicked() ), m_dataForm, SLOT( resetView() ) );
	connect( m_submit, SIGNAL( clicked() ), m_dataForm, SLOT( updateData() ) );
	line->setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed ) );
}

void AutoForm::slotMapChanged( )
{
	emit mapChanged();
}

void AutoForm::viewportResizeEvent( QResizeEvent * ev )
{
	QSize viewSize = ev->size();
	QSize newSize = m_mainBox->size();
	if( m_mainBox->minimumSizeHint().height() < viewSize.height() )
		newSize.setHeight(viewSize.height());
	if( m_mainBox->minimumSizeHint().width() < viewSize.width() )
		newSize.setWidth(viewSize.width());
	m_mainBox->resize(newSize);
}


#include "autoform.moc"
