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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef PPIXMAPEDIT_H
#define PPIXMAPEDIT_H

#include "propertywidget.h"

class QLabel;
class QPushButton;

namespace PropertyLib{

/**
@short %Property editor which shows a pixmap and allows to load it from file.
*/
class PPixmapEdit : public PropertyWidget
{
    Q_OBJECT
public:
    PPixmapEdit(MultiProperty* property, QWidget* parent = 0, const char* name = 0);

    virtual QVariant value() const;
    virtual void drawViewer(QPainter* p, const QColorGroup& cg, const QRect& r, const QVariant& value);
    virtual void setValue(const QVariant& value, bool emitChange);

    virtual void resizeEvent(QResizeEvent *ev);
    virtual bool eventFilter(QObject *o, QEvent *ev);
    
protected slots:
    void updateProperty();

private:
    QLabel *m_edit;
    QLabel *m_popup;
    QPushButton *m_button;
};

}

#endif
