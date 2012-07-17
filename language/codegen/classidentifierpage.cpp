/* This file is part of KDevelop
    Copyright 2008 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "classidentifierpage.h"
#include "language/duchain/identifier.h"

#include "ui_newclass.h"

namespace KDevelop {

struct ClassIdentifierPagePrivate
{
    ClassIdentifierPagePrivate()
        : classid(0)
    {
    }

    Ui::NewClassDialog* classid;
};

ClassIdentifierPage::ClassIdentifierPage(QWidget* parent)
: QWidget(parent)
, d(new ClassIdentifierPagePrivate())
{
    d->classid = new Ui::NewClassDialog;
    d->classid->setupUi(this);
    d->classid->keditlistwidget->setContentsMargins(0, 0, 0, 0);
    d->classid->keditlistwidget->layout()->setContentsMargins(0, 0, 0, 0);
    d->classid->keditlistwidget->lineEdit()->setPlaceholderText(i18n("Inheritance type and base class name"));

    connect(d->classid->identifierLineEdit, SIGNAL(textChanged(QString)), this, SLOT(checkIdentifier()));

    emit isValid(false);
}

ClassIdentifierPage::~ClassIdentifierPage()
{
    delete d;
}

KLineEdit* ClassIdentifierPage::identifierLineEdit() const
{
    return d->classid->identifierLineEdit;
}

KLineEdit* ClassIdentifierPage::inheritanceLineEdit() const
{
    return d->classid->keditlistwidget->lineEdit();
}

void ClassIdentifierPage::checkIdentifier()
{
    emit isValid(!identifierLineEdit()->userText().isEmpty());
}

QualifiedIdentifier ClassIdentifierPage::parseParentClassId(const QString& inheritedObject)
{
    return QualifiedIdentifier(inheritedObject);
}

QStringList ClassIdentifierPage::inheritanceList() const
{
    return d->classid->keditlistwidget->items();
}

}
