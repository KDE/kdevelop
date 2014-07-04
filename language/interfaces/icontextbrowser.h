/*
 * This file is part of KDevelop
 *
 * Copyright 2012 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_ICONTEXTBROWSER_H
#define KDEVPLATFORM_ICONTEXTBROWSER_H

#include "languageexport.h"

#include <language/duchain/duchainpointer.h>

#include <QObject>

namespace KDevelop {

/**
 * @brief Interface to the context browser plugin.
 */
class KDEVPLATFORMLANGUAGE_EXPORT IContextBrowser
{
public:
    virtual ~IContextBrowser();

    /**
     * Show a widget with the uses of @p declaration.
     */
    virtual void showUses(const DeclarationPointer& declaration) = 0;
};

}

Q_DECLARE_INTERFACE( KDevelop::IContextBrowser, "org.kdevelop.IContextBrowser" )

#endif // KDEVPLATFORM_ICONTEXTBROWSER_H
