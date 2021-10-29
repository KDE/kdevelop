/*
    SPDX-FileCopyrightText: 2015 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ITOOLVIEWACTIONLISTENER_H
#define KDEVPLATFORM_ITOOLVIEWACTIONLISTENER_H

#include "interfacesexport.h"

#include <QObject>

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
