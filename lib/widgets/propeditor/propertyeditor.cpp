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
#ifndef PURE_QT
#include <klocale.h>
#else
#include "qlocale.h"
#endif
 
#include <qdockwindow.h>
#include <qtable.h>
#include <qlayout.h>

#include "propertyeditor.h"
#include "property.h"

PropertyEditor::PropertyEditor( Place p, QWidget * parent, const char * name, WFlags f):
    QDockWindow(p, parent, name, f)
{
    setCloseMode(QDockWindow::Always);
    setResizeEnabled(true);

    QWidget *container = new QWidget(this);

    table = new QTable(container);
    table->setNumRows(0);
    table->setNumCols(2);
    table->horizontalHeader()->setLabel(0, i18n("Property"));
    table->horizontalHeader()->setLabel(1, i18n("Value"));
    table->setLeftMargin(0);
    table->setColumnReadOnly(0, TRUE);

    QGridLayout *gl = new QGridLayout(container);
    gl->addWidget(table, 0, 0);
    setWidget(container);

    props = 0;
}


PropertyEditor::~PropertyEditor(){
}

void PropertyEditor::populateProperties(QMap<QString, PropPtr > *v_props)
{
    table->hide();
    props = v_props;

    table->setNumRows(0);

    for (QMap<QString, PropPtr >::const_iterator it = props->begin(); it != props->end(); ++it)
    {
        int row = table->numRows() + 1;
        table->setNumRows(row);

        table->setText(row-1, 0, it.key()); //it.data()->description()); //it.key());

        QWidget *w = it.data()->editorOfType(this);
        table->setCellWidget(row-1, 1, w);
    }
    table->show();
}

void PropertyEditor::clearProperties()
{
    props->clear();
    delete props;
    props = 0;
    table->setNumRows(0);
}

void PropertyEditor::emitPropertyChange(QString name, QVariant newValue)
{
    qWarning("editor: assign %s to %s", name.latin1(), newValue.toString().latin1());
    emit propertyChanged(name, newValue);
}

#include "propertyeditor.moc"
