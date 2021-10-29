/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef __NAVIGATIONWIDGET_H__
#define __NAVIGATIONWIDGET_H__

#include <language/duchain/navigation/abstractnavigationwidget.h>

#include "duchainexport.h"

namespace KDevelop {
    class IncludeItem;
}

namespace QmlJS {

class KDEVQMLJSDUCHAIN_EXPORT NavigationWidget : public KDevelop::AbstractNavigationWidget
{
    Q_OBJECT
public:
    NavigationWidget(KDevelop::Declaration* decl,
                     KDevelop::TopDUContext* topContext,
                     KDevelop::AbstractNavigationWidget::DisplayHints hints);
    NavigationWidget(const KDevelop::IncludeItem& includeItem,
                     const KDevelop::TopDUContextPointer& topContext,
                     KDevelop::AbstractNavigationWidget::DisplayHints hints);
};

}

#endif
