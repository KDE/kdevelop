/* This file is part of the KDE project
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2003 John Firebaugh <jfirebaugh@kde.org>

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

#include "environmentvariableswidget.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qlistview.h>
#include "domutil.h"
#include "addenvvardlg.h"


void EnvironmentVariablesWidget::addVarClicked()
{
    AddEnvvarDialog dlg( this, "add env dialog" ) ;
    if (QListViewItem *Item = listview->selectedItem())
    {
        dlg.setvarname(Item->text(0));
        dlg.setvalue(Item->text(1));
    }
    if (!dlg.exec())
        return;

    (void) new QListViewItem(listview, dlg.varname(), dlg.value());
}


void EnvironmentVariablesWidget::editVarClicked()
{
    AddEnvvarDialog dlg( this, "edit env dialog" );
    QListViewItem *item = listview->selectedItem();
    if (  !item )
        return;
    dlg.setvarname(item->text(0));
    dlg.setvalue(item->text(1));
    if (!dlg.exec())
        return;

    item->setText(0,dlg.varname());
    item->setText(1,dlg.value());
}


void EnvironmentVariablesWidget::removeVarClicked()
{
    delete listview->selectedItem();
}


EnvironmentVariablesWidget::EnvironmentVariablesWidget(QDomDocument &dom, const QString &configGroup,
                                   QWidget *parent, const char *name)
    : EnvironmentVariablesWidgetBase(parent, name),
      m_dom(dom), m_configGroup(configGroup)
{
    readEnvironment(dom, configGroup);
    connect( listview, SIGNAL( doubleClicked ( QListViewItem *, const QPoint &, int ) ), this, SLOT( editVarClicked() ) );
}


EnvironmentVariablesWidget::~EnvironmentVariablesWidget()
{}

void EnvironmentVariablesWidget::readEnvironment(QDomDocument &dom, const QString &configGroup)
{
    m_dom = dom;
    m_configGroup = configGroup;

    listview->clear();

    DomUtil::PairList list =
        DomUtil::readPairListEntry(dom, m_configGroup, "envvar", "name", "value");

    QListViewItem *lastItem = 0;

    DomUtil::PairList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        QListViewItem *newItem = new QListViewItem(listview, (*it).first, (*it).second);
        if (lastItem)
            newItem->moveItem(lastItem);
        lastItem = newItem;
    }
}

void EnvironmentVariablesWidget::changeConfigGroup( const QString &configGroup)
{
    m_configGroup = configGroup;
}

void EnvironmentVariablesWidget::accept()
{
    DomUtil::PairList list;
    QListViewItem *item = listview->firstChild();
    while (item) {
        list << DomUtil::Pair(item->text(0), item->text(1));
        item = item->nextSibling();
    }

    DomUtil::writePairListEntry(m_dom, m_configGroup, "envvar", "name", "value", list);
}

#include "environmentvariableswidget.moc"
