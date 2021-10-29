/*
    SPDX-FileCopyrightText: 2004 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "shellextension.h"

namespace KDevelop
{
ShellExtension *ShellExtension::s_instance = nullptr;

ShellExtension::ShellExtension()
{
}

ShellExtension *ShellExtension::getInstance()
{
    return s_instance;
}

}
