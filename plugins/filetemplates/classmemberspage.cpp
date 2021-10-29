/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "classmemberspage.h"
#include "debug.h"

#include <KEditListWidget>
#include <QLineEdit>
#include <KLocalizedString>

#include <QVBoxLayout>

using namespace KDevelop;

class KDevelop::ClassMembersPagePrivate
{
public:
    KEditListWidget* editListWidget;
};

ClassMembersPage::ClassMembersPage(QWidget* parent)
: QWidget(parent)
, d(new ClassMembersPagePrivate)
{
    d->editListWidget = new KEditListWidget(this);
    d->editListWidget->lineEdit()->setPlaceholderText(i18nc("@info:placeholder", "Variable type and identifier"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
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
    memberItems.reserve(members.size());
    for (const VariableDescription& variable : members) {
        QStringList items;
        items.reserve(3);
        if (!variable.access.isEmpty())
        {
            items << variable.access;
        }
        if (!variable.type.isEmpty())
        {
            items << variable.type;
        }
        items << variable.name;
        memberItems << items.join(QLatin1Char(' '));
    }
    d->editListWidget->setItems(memberItems);
}

VariableDescriptionList ClassMembersPage::members() const
{
    VariableDescriptionList list;
    const auto items = d->editListWidget->items();
    for (const QString& item : items) {
        VariableDescription var;
        QStringList parts = item.split(QLatin1Char(' '));
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

void ClassMembersPage::setFocusToFirstEditWidget()
{
    d->editListWidget->lineEdit()->setFocus();
}
