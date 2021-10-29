#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

# Print the license of the generated file (the same as the one of this file)
def license():
    print("""/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
""")

def basicTypes(d):
    d["_mixed"] = '_mixed'       # This variable does not exist and will therefore yield the type "mixed"
    d["_string"] = '""'
    d["_bool"] = 'true'
    d["_int"] = '1'
    d["_float"] = '1.0'
    d["_array"] = '[]'
    d["_void"] = ''

def require(module):
    print('var %s = require("%s");\n' % (module, module))
