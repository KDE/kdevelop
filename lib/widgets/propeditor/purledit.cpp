/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mskat.net                                                 *
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
#include "purledit.h"

#ifndef PURE_QT
#include <kurlrequester.h>
#else
#include <qpushbutton.h>
#include <qlineedit.h>
#endif
#include <qfiledialog.h>
#include <qlayout.h>

namespace PropertyLib{

PUrlEdit::PUrlEdit(Mode mode, MultiProperty* property, QWidget* parent, const char* name)
    :PropertyWidget(property, parent, name)
{
    QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
#ifndef PURE_QT
    m_edit = new KURLRequester(this);
    l->addWidget(m_edit);
    m_edit->setMode((KFile::Mode)mode);
    connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
#else
    m_edit = new QLineEdit(this);
    m_select = new QPushButton("...",this);
    l->addWidget(m_edit);
    l->addWidget(m_select);
    m_mode = mode;
    connect( m_select, SIGNAL(clicked()),this,SLOT(select()));
#endif
    m_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

QVariant PUrlEdit::value() const
{
#ifndef PURE_QT
     return QVariant(m_edit->url());
#else
     return QVariant(m_url);
#endif
}

void PUrlEdit::setValue(const QVariant& value, bool emitChange)
{
#ifndef PURE_QT
    disconnect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
    m_edit->setURL(value.toString());
    connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
#else
    m_edit->setText(value.toString());
#endif
    if (emitChange)
        emit propertyChanged(m_property, value);
}

void PUrlEdit::updateProperty(const QString &val)
{
    emit propertyChanged(m_property, QVariant(val));
}

void PUrlEdit::select()
{
    QString path = m_url;
    if( m_mode == Directory )
	m_url = QFileDialog::getExistingDirectory( m_url,this);
    else
        m_url = QFileDialog::getOpenFileName(m_url, QString::null, this);
    updateProperty(m_url);
    m_edit->setText(m_url);

}
	
}

#ifndef PURE_QT
#include "purledit.moc"
#endif
