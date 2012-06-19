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
    ClassDescriptionModel* model;
    Ui::ClassMembersPage* ui;
};

ClassMembersPage::ClassMembersPage(QWidget* parent)
: QWidget(parent)
, d(new ClassMembersPagePrivate)
{
    d->model = 0;

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
    d->model = new ClassDescriptionModel(description, this);
    d->ui->itemView->setModel(d->model);
    d->ui->itemView->setRootIndex(d->model->index(ClassDescriptionModel::MembersRow, 0));

    connect (d->ui->itemView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             this, SLOT(currentSelectionChanged(QItemSelection)));

    currentSelectionChanged(QItemSelection());
}

ClassDescription ClassMembersPage::description() const
{
    Q_ASSERT(d->model);
    return d->model->description();
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
    return d->model->rowCount(d->ui->itemView->rootIndex());
}


void ClassMembersPage::moveRowTo(int destination, bool relative)
{
    Q_ASSERT(d->model);
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

    d->model->moveRow(source, destination, d->ui->itemView->rootIndex());
    d->ui->itemView->setCurrentIndex(d->model->index(destination, 0, d->ui->itemView->rootIndex()));
}

void ClassMembersPage::moveTop()
{
    moveRowTo(0, false);
}

void ClassMembersPage::moveBottom()
{
    moveRowTo(d->model->rowCount(d->ui->itemView->rootIndex())-1, false);
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
    Q_ASSERT(d->model);
    d->model->insertRow(0, d->ui->itemView->rootIndex());
}

void ClassMembersPage::removeItem()
{
    Q_ASSERT(d->model);
    QModelIndexList indexes = d->ui->itemView->selectionModel()->selectedRows();
    if (indexes.isEmpty())
    {
        return;
    }

    d->model->removeRow(indexes.first().row(), d->ui->itemView->rootIndex());
}
