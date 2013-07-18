/* KDevPlatform Kross Support
 *
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_KROSSTOOLVIEWFACTORY_H
#define KDEVPLATFORM_KROSSTOOLVIEWFACTORY_H

#include <kross/core/action.h>
#include <interfaces/iuicontroller.h>

class KrossToolViewFactory: public KDevelop::IToolViewFactory, public QObject
{
public:
    KrossToolViewFactory(QObject* parent, Kross::Action* action, const QString& method, const QString& id, Qt::DockWidgetArea pos)
        : QObject(parent), m_action(action), m_method(method), m_id(id), m_pos(pos) {}
    
    virtual QWidget* create(QWidget *parent = 0);
    
    virtual Qt::DockWidgetArea defaultPosition();
    
    virtual QString id() const;
    
private:
    Kross::Action* m_action;
    QString m_method;
    QString m_id;
    Qt::DockWidgetArea m_pos;
};

#endif
