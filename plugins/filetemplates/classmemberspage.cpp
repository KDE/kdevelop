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
#include "debug.h"

#include <KEditListWidget>
#include <KLineEdit>
#include <KLocalizedString>

#include <QVBoxLayout>

using namespace KDevelop;

class ClassMembersPagePrivate
{
public:
    KEditListWidget* editListWidget;
};

ClassMembersPage::ClassMembersPage(QWidget* parent)
: QWidget(parent)
, d(new ClassMembersPagePrivate)
{
    d->editListWidget = new KEditListWidget(this);
    d->editListWidget->lineEdit()->setPlaceholderText(i18n("Variable type and identifier"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(d->editListWidget);
    setLayout(layout);
}

ClassMembersPage::~ClassMembersPage()
{
    delete d;
}

void ClassMembersPage::setMembers(const VariableDescriptionList& members)
{
    QStringList memberItems;
    foreach (const VariableDescription& variable, members)
    {
        QStringList items;
        if (!variable.access.isEmpty())
        {
            items << variable.access;
        }
        if (!variable.type.isEmpty())
        {
            items << variable.type;
        }
        items << variable.name;
        memberItems << items.join(" ");
    }
    d->editListWidget->setItems(memberItems);
}

VariableDescriptionList ClassMembersPage::members() const
{
    VariableDescriptionList list;
    foreach (const QString& item, d->editListWidget->items())
    {
        VariableDescription var;
        QStringList parts = item.split(' ');
        switch (parts.size())
        {
            case 1:
                var.name = parts[0];
                break;

            case 2:
                var.type = parts[0];
                var.name = parts[1];
                break;

            case 3:
                var.access = parts[0];
                var.type = parts[1];
                var.name = parts[2];
                break;

            default:
                qCDebug(PLUGIN_FILETEMPLATES) << "Malformed class member" << item;
                break;
        }

        if (!var.name.isEmpty())
        {
            list << var;
        }
    }
    return list;
}
