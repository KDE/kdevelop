/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKENAVIGATIONWIDGET_H
#define CMAKENAVIGATIONWIDGET_H

#include <language/duchain/navigation/abstractnavigationcontext.h>
#include <language/duchain/navigation/abstractnavigationwidget.h>
#include <interfaces/idocumentation.h>

namespace KDevelop { class IDocumentation; }

class CMakeNavigationWidget : public KDevelop::AbstractNavigationWidget
{
        Q_OBJECT
    public:
        CMakeNavigationWidget(const KDevelop::TopDUContextPointer& top, const KDevelop::IDocumentation::Ptr& doc);
        CMakeNavigationWidget(const KDevelop::TopDUContextPointer& top, KDevelop::Declaration* decl);
};

#endif // CMAKENAVIGATIONWIDGET_H
