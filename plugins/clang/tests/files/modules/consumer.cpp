/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import module_example;

int main()
{
    auto& m = module_example::Module::create();
    module_example::Module::destroy(m);
    return 0;
}
