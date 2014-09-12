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

#ifndef KDEVPLATFORM_PLUGIN_APPLETSELECTOR_H
#define KDEVPLATFORM_PLUGIN_APPLETSELECTOR_H

#include <KDialog>
#include <KPluginInfo>

class QStandardItemModel;
class QModelIndex;
namespace Ui { class AppletSelector; }

class AppletSelector : public KDialog
{
    Q_OBJECT
    public:
        AppletSelector(const QString& parentApp, const QStringList& whitelist, QWidget* parent);
        virtual ~AppletSelector();
        
    public slots:
        void selected(const QModelIndex& idx);
        void addClicked();
        void canAdd();
        
    signals:
        void addApplet(const QString& name);
        
    private:
        void addPlugins(QStandardItemModel* model, const KPluginInfo::List& list);
        KPluginInfo::List filterByName(const QStringList& whitelist, const KPluginInfo::List& listAppletInfo);
        Ui::AppletSelector* m_ui;
};

#endif // KDEVPLATFORM_PLUGIN_APPLETSELECTOR_H
