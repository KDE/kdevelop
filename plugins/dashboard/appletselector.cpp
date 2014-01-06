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

AppletSelector::AppletSelector(const QString& parentApp, const QStringList& whitelist, QWidget* parent)
    : KDialog(parent)
{
    setButtons(Close);
    QWidget* w = new QWidget(this);
    
    m_ui = new Ui::AppletSelector;
    m_ui->setupUi(w);
    
    m_ui->plugins->header()->setSortIndicator(0, Qt::AscendingOrder);
    m_ui->addButton->setIcon(QIcon::fromTheme("list-add"));
    
    setMainWidget(w);
    
    QStandardItemModel* model = new QStandardItemModel(this);
    
    addPlugins(model, Plasma::Applet::listAppletInfo(QString(), parentApp));
    addPlugins(model, filterByName(whitelist, Plasma::Applet::listAppletInfo()));
    
    m_ui->plugins->setModel(model);
    
    connect(m_ui->plugins, SIGNAL(activated(QModelIndex)), SLOT(canAdd()));
    connect(m_ui->plugins, SIGNAL(doubleClicked(QModelIndex)), SLOT(selected(QModelIndex)));
    connect(m_ui->addButton, SIGNAL(clicked(bool)), SLOT(addClicked()));
}

AppletSelector::~AppletSelector()
{
    delete m_ui;
}

void AppletSelector::addPlugins(QStandardItemModel* model, const KPluginInfo::List& list)
{
    foreach(const KPluginInfo& info, list) {
        QStandardItem* item = new QStandardItem(QIcon::fromTheme(info.icon()), info.name());
        item->setEditable(false);
        item->setToolTip(info.comment());
        item->setData(info.pluginName(), Qt::UserRole+1);
        
        model->appendRow(item);
    }
}

KPluginInfo::List AppletSelector::filterByName(const QStringList& whitelist, const KPluginInfo::List& listAppletInfo)
{
    KPluginInfo::List ret;
    foreach(const KPluginInfo& plugin, listAppletInfo) {
        if(whitelist.contains(plugin.pluginName()))
            ret += plugin;
    }
    return ret;
}


void AppletSelector::selected(const QModelIndex& idx)
{
    emit addApplet(idx.data(Qt::UserRole+1).toString());
}

void AppletSelector::addClicked()
{
    selected(m_ui->plugins->selectionModel()->currentIndex());
}

void AppletSelector::canAdd()
{
    m_ui->addButton->setEnabled(true);
}
