/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

module;

#include "test_module.hpp"
#include "test_submodule.hpp"

export module module_example;

export namespace module_example {
    using module_example::Module;
    using module_example::SubModule;
}
