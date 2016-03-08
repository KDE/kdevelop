# -*- coding: utf-8 -*-
# Pretty-printers for KDE4.

# Copyright (C) 2009 Milian Wolff <mail@milianw.de>
# Copyright (C) 2014 Kevin Funk <kfunk@kde.org>

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import gdb
import itertools
import re

import qt

from helper import *

class KDevelop_Path:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        iterator = qt.QVectorPrinter(self.val['m_data'], 'QVector').children()
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
