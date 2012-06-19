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
    d->classid->addInheritancePushButton->setIcon(KIcon("list-add"));
    d->classid->removeInheritancePushButton->setIcon(KIcon("list-remove"));
    d->classid->moveDownPushButton->setIcon(KIcon("go-down"));
    d->classid->moveUpPushButton->setIcon(KIcon("go-up"));

    connect(d->classid->identifierLineEdit, SIGNAL(textChanged(QString)), this, SLOT(checkIdentifier()));
    connect(d->classid->addInheritancePushButton, SIGNAL(pressed()), this, SLOT(addInheritance()));
    connect(d->classid->removeInheritancePushButton, SIGNAL(pressed()), this, SLOT(removeInheritance()));
    connect(d->classid->moveUpPushButton, SIGNAL(pressed()), this, SLOT(moveUpInheritance()));
    connect(d->classid->moveDownPushButton, SIGNAL(pressed()), this, SLOT(moveDownInheritance()));
    connect(d->classid->inheritanceList, SIGNAL(currentRowChanged(int)), this, SLOT(checkMoveButtonState()));

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
    return d->classid->inheritanceLineEdit;
}

void ClassIdentifierPage::checkIdentifier()
{
    emit isValid(!identifierLineEdit()->userText().isEmpty());
}

void ClassIdentifierPage::addInheritance()
{
    d->classid->inheritanceList->addItem(d->classid->inheritanceLineEdit->text());

    d->classid->inheritanceLineEdit->clear();

    d->classid->removeInheritancePushButton->setEnabled(true);

    if (d->classid->inheritanceList->count() > 1) {
        checkMoveButtonState();
    }

    emit inheritanceChanged();
}

void ClassIdentifierPage::removeInheritance()
{
    delete d->classid->inheritanceList->takeItem(d->classid->inheritanceList->currentRow());

    if (d->classid->inheritanceList->count() == 0) {
        d->classid->removeInheritancePushButton->setEnabled(false);
    }

    checkMoveButtonState();

    emit inheritanceChanged();
}

void ClassIdentifierPage::moveUpInheritance()
{
    int currentRow = d->classid->inheritanceList->currentRow();

    Q_ASSERT(currentRow > 0);
    if (currentRow <= 0) {
        return;
    }

    QListWidgetItem* item = d->classid->inheritanceList->takeItem(currentRow);
    d->classid->inheritanceList->insertItem(currentRow - 1, item);
    d->classid->inheritanceList->setCurrentItem(item);

    emit inheritanceChanged();
}

void ClassIdentifierPage::moveDownInheritance()
{
    int currentRow = d->classid->inheritanceList->currentRow();

    Q_ASSERT(currentRow != -1 && currentRow < d->classid->inheritanceList->count() - 1);
    if (currentRow == -1 || currentRow >= d->classid->inheritanceList->count() - 1) {
        return;
    }

    QListWidgetItem* item = d->classid->inheritanceList->takeItem(currentRow);
    d->classid->inheritanceList->insertItem(currentRow + 1, item);
    d->classid->inheritanceList->setCurrentItem(item);

    emit inheritanceChanged();
}

QualifiedIdentifier ClassIdentifierPage::parseParentClassId(const QString& inheritedObject)
{
    return QualifiedIdentifier(inheritedObject);
}

void ClassIdentifierPage::checkMoveButtonState()
{
    int currentRow = d->classid->inheritanceList->currentRow();
    d->classid->moveUpPushButton->setEnabled(currentRow > 0);
    d->classid->moveDownPushButton->setEnabled(currentRow >= 0 && currentRow < d->classid->inheritanceList->count() - 1);
}

QStringList ClassIdentifierPage::inheritanceList() const
{
    QStringList ret;
    for (int i = 0; i < d->classid->inheritanceList->count(); ++i)
        ret << d->classid->inheritanceList->item(i)->text();

    return ret;
}

}
