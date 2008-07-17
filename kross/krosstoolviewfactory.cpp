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

#include "krosstoolviewfactory.h"
#include <KDebug>
#include <QWidget>

QWidget* KrossToolViewFactory::create(QWidget *parent)
{
    QVariant result=m_action->callFunction( m_method, QVariantList() << qVariantFromValue(parent));
    
    QWidget* ret= qobject_cast<QWidget*>(result.value<QObject*>());
    return ret;
}

Qt::DockWidgetArea KrossToolViewFactory::defaultPosition()
{
    return m_pos;
}

QString KrossToolViewFactory::id() const
{
    return m_id;
}

