/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

module;

#include "test_module.hpp"
#include "test_submodule.hpp"

export module module_example;

export namespace module_example {
    using module_example::Module;
    using module_example::SubModule;
}
