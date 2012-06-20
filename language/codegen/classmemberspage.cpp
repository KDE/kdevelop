/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "classmemberspage.h"
#include "ui_classmembers.h"

using namespace KDevelop;

class KDevelop::ClassMembersPagePrivate
{
public:
    Ui::ClassMembersPage* ui;
};

ClassMembersPage::ClassMembersPage(QWidget* parent)
: QWidget(parent)
, d(new ClassMembersPagePrivate)
{
    d->ui = new Ui::ClassMembersPage;
    d->ui->setupUi(this);

    connect (d->ui->topButton, SIGNAL(clicked(bool)), SLOT(moveTop()));
    connect (d->ui->upButton, SIGNAL(clicked(bool)), SLOT(moveUp()));
    connect (d->ui->downButton, SIGNAL(clicked(bool)), SLOT(moveDown()));
    connect (d->ui->bottomButton, SIGNAL(clicked(bool)), SLOT(moveBottom()));

    connect (d->ui->addItemButton, SIGNAL(clicked(bool)), SLOT(addItem()));
    connect (d->ui->removeItemButton, SIGNAL(clicked(bool)), SLOT(removeItem()));
}

ClassMembersPage::~ClassMembersPage()
{

}

void ClassMembersPage::setDescription(const ClassDescription& description)
{
    foreach (const VariableDescription& variable, description.members)
    {
        int i = 0;
        d->ui->itemView->insertRow(i);
        d->ui->itemView->setItem(i, 0, new QTableWidgetItem(variable.type));
        d->ui->itemView->setItem(i, 1, new QTableWidgetItem(variable.name));
    }

    connect (d->ui->itemView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             this, SLOT(currentSelectionChanged(QItemSelection)));

    currentSelectionChanged(QItemSelection());
}

ClassDescription ClassMembersPage::description() const
{
    ClassDescription description;
    for (int i = 0; i < d->ui->itemView->rowCount(); ++i)
    {
        VariableDescription var;
        var.type = d->ui->itemView->item(i, 0)->text();
        var.name = d->ui->itemView->item(i, 1)->text();
        description.members << var;
    }
    return description;
}

void ClassMembersPage::currentSelectionChanged(const QItemSelection& current)
{
    bool up = false;
    bool down = false;
    bool remove = false;

    if (!current.indexes().isEmpty())
    {
        up = current.indexes().first().row() > 0;
        down = current.indexes().first().row() < rows()-1;
        remove = true;
    }

    d->ui->topButton->setEnabled(up);
    d->ui->upButton->setEnabled(up);
    d->ui->downButton->setEnabled(down);
    d->ui->bottomButton->setEnabled(down);

    d->ui->removeItemButton->setEnabled(remove);
}

int ClassMembersPage::rows()
{
    return d->ui->itemView->rowCount();
}

void ClassMembersPage::moveRowTo(int destination, bool relative)
{
    Q_ASSERT(d->ui->itemView->selectionModel());
    QModelIndexList indexes = d->ui->itemView->selectionModel()->selectedRows();

    if (indexes.isEmpty())
    {
        return;
    }

    int source = indexes.first().row();

    if (relative)
    {
        destination = source + destination;
    }

    VariableDescription desc;
    desc.type = d->ui->itemView->item(source, 0)->text();
    desc.name = d->ui->itemView->item(source, 1)->text();

    d->ui->itemView->removeRow(source);
    d->ui->itemView->insertRow(destination);
    d->ui->itemView->setItem(destination, 0, new QTableWidgetItem(desc.type));
    d->ui->itemView->setItem(destination, 1, new QTableWidgetItem(desc.name));
    d->ui->itemView->setCurrentCell(destination, 0);
}

void ClassMembersPage::moveTop()
{
    moveRowTo(0, false);
}

void ClassMembersPage::moveBottom()
{
    moveRowTo(rows()-1, false);
}

void ClassMembersPage::moveUp()
{
    moveRowTo(-1, true);
}

void ClassMembersPage::moveDown()
{
    moveRowTo(1, true);
}

void ClassMembersPage::addItem()
{
    d->ui->itemView->insertRow(rows());
}

void ClassMembersPage::removeItem()
{
    QModelIndexList indexes = d->ui->itemView->selectionModel()->selectedRows();
    if (indexes.isEmpty())
    {
        return;
    }

    d->ui->itemView->removeRow(indexes.first().row());
}
