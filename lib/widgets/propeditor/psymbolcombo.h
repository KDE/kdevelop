/* This file is part of the KDE project
   Copyright (C) 2002 Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef PSYMBOLCOMBO_H
#define PSYMBOLCOMBO_H

#include <qwidget.h>
#include "propertywidget.h"

class PropertyEditor;
class QLineEdit;
class QPushButton;
class QHBoxLayout;

class PSymbolCombo: public QWidget, PropertyWidget{
    Q_OBJECT
public:
    PSymbolCombo(const PropertyEditor *editor, const QString name, const QVariant value, QWidget *parent = 0, const char *name = 0);

    virtual QVariant value() const;
    virtual void setValue(const QVariant value, bool emitChange=true);

public slots:
    void selectChar();

private:
    QLineEdit *edit;
    QPushButton *pbSelect;
    QHBoxLayout *l;
signals:
    void propertyChanged(QString name, QVariant newValue);
private slots:
    void updateProperty(const QString &val);
};

#endif
