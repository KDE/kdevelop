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
#ifndef PURLEDIT_H
#define PURLEDIT_H

#include "propertywidget.h"

#ifndef PURE_QT
#include <kfile.h>
class KURLRequester;
#else
#include <klineedit.h>
class QPushButton;
#endif


namespace PropertyLib{

/**
@short %Property editor with an url editor to choose the location of file or directory.
*/
class PUrlEdit : public PropertyWidget
{
Q_OBJECT
public:
#ifndef PURE_QT
    enum Mode {File = KFile::File,Directory = KFile::Directory};
#else
    enum Mode {File,Directory};
#endif

    PUrlEdit(Mode mode, MultiProperty* property, QWidget* parent=0, const char* name=0);

    virtual QVariant value() const;
    virtual void setValue(const QVariant& value, bool emitChange);
    
private slots:
    void updateProperty(const QString &val);    
    void select();
private:
#ifndef PURE_QT
    KURLRequester *m_edit;
#else
    KLineEdit *m_edit;
    QPushButton *m_select;
    QString m_url;
    Mode m_mode;
#endif
};

}

#endif
