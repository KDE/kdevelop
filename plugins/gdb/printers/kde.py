# -*- coding: utf-8 -*-
# Pretty-printers for KDE4.

# SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>
# SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

import gdb
import itertools
import re

import qt

from helper import *

class KDevelop_Path:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        iterator = qt.QListPrinter(self.val['m_data'], 'QList', 'QString').children()
        pathSegments = [str(it[1]) for it in iterator]
        return "(" + ", ".join(pathSegments) + ")" if pathSegments else None

class KTextEditor_CursorPrinter:
    "Pretty Printer for KTextEditor::Cursor"

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "[%d, %d]" % (self.val['m_line'], self.val['m_column'])

class KTextEditor_RangePrinter:
    "Pretty Printer for KTextEditor::Range"

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "[(%d, %d) -> (%d, %d)]" % (self.val['m_start']['m_line'], self.val['m_start']['m_column'],
                                           self.val['m_end']['m_line'], self.val['m_end']['m_column'])

pretty_printers_dict = {}

def register_kde_printers (obj):
    if obj == None:
        obj = gdb

    obj.pretty_printers.append(FunctionLookup(gdb, pretty_printers_dict))

def build_dictionary ():
    pretty_printers_dict[re.compile('^KDevelop::Path$')] = lambda val: KDevelop_Path(val)

    pretty_printers_dict[re.compile('^KTextEditor::Cursor$')] = lambda val: KTextEditor_CursorPrinter(val)
    pretty_printers_dict[re.compile('^KTextEditor::Range$')] = lambda val: KTextEditor_RangePrinter(val)

build_dictionary ()
