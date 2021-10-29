/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iuicontroller.h"

namespace KDevelop {

IUiController::IUiController()
{
}

IUiController::~IUiController()
{
}

}

void KDevelop::IToolViewFactory::viewCreated(Sublime::View * view)
{
    Q_UNUSED(view)
}
