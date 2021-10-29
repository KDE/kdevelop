/*
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "classidentifierpage.h"
#include "language/duchain/identifier.h"

#include <KLocalizedString>

#include "ui_newclass.h"

using namespace KDevelop;

struct KDevelop::ClassIdentifierPagePrivate
{
    ClassIdentifierPagePrivate()
    {
    }

    Ui::NewClassDialog* classid = nullptr;
};

ClassIdentifierPage::ClassIdentifierPage(QWidget* parent)
: QWidget(parent)
, d(new ClassIdentifierPagePrivate())
{
    d->classid = new Ui::NewClassDialog;
    d->classid->setupUi(this);
    d->classid->identifierLineEdit->setPlaceholderText(i18nc("@info:placeholder", "Class name, including any namespaces"));
    d->classid->keditlistwidget->lineEdit()->setPlaceholderText(i18nc("@info:placeholder", "Inheritance type and base class name"));

    d->classid->inheritanceLabel->setBuddy(d->classid->keditlistwidget->lineEdit());

    connect(d->classid->identifierLineEdit, &QLineEdit::textChanged, this, &ClassIdentifierPage::checkIdentifier);
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
