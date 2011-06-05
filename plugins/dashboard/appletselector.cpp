/* This file is part of KDevelop
  Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#include "appletselector.h"
#include "ui_appletselector.h"
#include <plasma/applet.h>
#include <interfaces/icore.h>
#include <QStandardItemModel>

using namespace KDevelop;

AppletSelector::AppletSelector(const KPluginInfo::List& plugins, QWidget* parent)
    : KDialog(parent)
{
    setButtons(Close);
    QWidget* w = new QWidget(this);
    
    m_ui = new Ui::AppletSelector;
    m_ui->setupUi(w);
    
    m_ui->plugins->header()->setSortIndicator(0, Qt::AscendingOrder);
    
    setMainWidget(w);
    
    QStandardItemModel* model = new QStandardItemModel(this);
    KPluginInfo::List list=Plasma::Applet::listAppletInfo();
    list.append(plugins);
    
    foreach(const KPluginInfo& info, list) {
        QStandardItem* item = new QStandardItem(KIcon(info.icon()), info.name());
        item->setEditable(false);
        item->setToolTip(info.comment());
        item->setData(info.pluginName(), Qt::UserRole+1);
        
        model->appendRow(item);
    }
    
    m_ui->plugins->setModel(model);
    
    connect(m_ui->plugins, SIGNAL(doubleClicked(QModelIndex)), SLOT(selected(QModelIndex)));
}

void AppletSelector::selected(const QModelIndex& idx)
{
    emit addApplet(idx.data(Qt::UserRole+1).toString());
}
