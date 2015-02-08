/***************************************************************************
 *   Copyright 2015 Kevin Funk <kfunk@kde.org>                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_ITOOLVIEWACTIONLISTENER_H
#define KDEVPLATFORM_ITOOLVIEWACTIONLISTENER_H

#include "interfacesexport.h"

#include <QtCore/QObject>

namespace KDevelop {

/**
 * An interface for tool view widgets
 *
 * Implement this interface for the widgets which are being created by KDevelop::IToolViewFactory::create()
 *
 * This interface allows global shortcuts, such as "Jump to Next Output Mark", to map to the tool view widget
 * which was last recently activated ("raised").
 *
 * @sa KDevelop::IToolViewFactory::create()
 */
class KDEVPLATFORMINTERFACES_EXPORT IToolViewActionListener
{
public:
    virtual ~IToolViewActionListener();

public Q_SLOTS:
    virtual void selectPreviousItem() = 0;
    virtual void selectNextItem() = 0;
};

}

Q_DECLARE_INTERFACE(KDevelop::IToolViewActionListener, "org.kdevelop.IToolViewActionListener")

#endif
