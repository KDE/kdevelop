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

#include <KI18n/KLocalizedString>
#include <kwidgetsaddons_version.h>

#include "ui_newclass.h"

using namespace KDevelop;

struct KDevelop::ClassIdentifierPagePrivate
{
    ClassIdentifierPagePrivate()
        : classid(nullptr)
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
    d->classid->identifierLineEdit->setPlaceholderText(i18n("Class name, including any namespaces"));
    d->classid->keditlistwidget->lineEdit()->setPlaceholderText(i18n("Inheritance type and base class name"));

    d->classid->inheritanceLabel->setBuddy(d->classid->keditlistwidget->lineEdit());

    connect(d->classid->identifierLineEdit, &QLineEdit::textChanged, this, &ClassIdentifierPage::checkIdentifier);

#if KWIDGETSADDONS_VERSION < QT_VERSION_CHECK(5,32,0)
    // workaround for KEditListWidget bug:
    // ensure keyboard focus is returned to edit line
    connect(d->classid->keditlistwidget, &KEditListWidget::added,
            d->classid->keditlistwidget->lineEdit(),
            static_cast<void(QWidget::*)()>(&QWidget::setFocus));
    connect(d->classid->keditlistwidget, &KEditListWidget::removed,
            d->classid->keditlistwidget->lineEdit(),
            static_cast<void(QWidget::*)()>(&QWidget::setFocus));
#endif

    emit isValid(false);
}

ClassIdentifierPage::~ClassIdentifierPage()
{
    delete d->classid;
    delete d;
}

QString ClassIdentifierPage::identifier() const
{
    return d->classid->identifierLineEdit->text();
}

void ClassIdentifierPage::checkIdentifier()
{
    emit isValid(!identifier().isEmpty());
}

QStringList ClassIdentifierPage::inheritanceList() const
{
    return d->classid->keditlistwidget->items();
}

void ClassIdentifierPage::setInheritanceList (const QStringList& list)
{
    d->classid->keditlistwidget->setItems(list);
}

void ClassIdentifierPage::setFocusToFirstEditWidget()
{
    d->classid->identifierLineEdit->setFocus();
}
