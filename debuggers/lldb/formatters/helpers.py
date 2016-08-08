#
# LLDB data formatter helpers
# Copyright 2016 Aetf <aetf@unlimitedcodeworks.xyz>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License or (at your option) version 3 or any later version
# accepted by the membership of KDE e.V. (or its successor approved
# by the membership of KDE e.V.), which shall act as a proxy
# defined in Section 14 of version 3 of the license.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# BEGIN: Utilities for wrapping differences of Python 2.x and Python 3
# Inspired by http://pythonhosted.org/six/

import sys
# Useful for very coarse version differentiation.
PY2 = sys.version_info[0] == 2
PY3 = sys.version_info[0] == 3

# create Python 2.x & 3.x compatible iterator base
if PY3:
    Iterator = object
else:
    class Iterator(object):

        def next(self):
            return type(self).__next__(self)
if PY3:
    unichr = chr
else:
    unichr = unichr
# END

def quote(string, quote='"'):
    """Quote a string so it's suitable to be used in quote"""
    return '{q}{s}{q}'.format(s = string.replace('\\', '\\\\').replace(quote, '\\' + quote),
                              q = quote)

def unquote(string, quote='"'):
    """Unquote a string"""
    if string.startswith(quote) and string.endswith(quote):
        string = string.lstrip(quote).rstrip(quote)
        ls = []
        esc = False
        for idx in range(0, len(string)):
            ch = string[idx]
            if ch == '\\':
                if esc:
                    ls.append(ch)
                esc = not esc
            else:
                ls.append(ch)
        string = ''.join(ls)
    return string

class SummaryProvider(object):
    """A lldb synthetic provider that defaults return real children of the value"""
    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self._members = []
        self._name2idx = {}
        self._has_children = False
        self._num_children = 0

    def num_children(self):
        return self._num_children

    def get_child_index(self, name):
        if name in self._name2idx:
            return self._name2idx[name]
        return None

    def get_child_at_index(self, idx):
        if idx < 0 or idx >= self._num_children:
            return None
        return self._members[idx]

    def update(self):
        if not self.valobj.IsValid():
            self._has_children = False
        else:
            self._has_children = self.valobj.MightHaveChildren()
        self._num_children = 0
        self._members = []
        self._name2idx = {}

        # The valobj doesn't has children or is invalid, no need to proceed
        if self._has_children:
            self._num_children = self.valobj.GetNumChildren()
            self._members = [m for m in self.valobj]
            self._name2idx = {
                self._members[idx].GetName(): idx
                for idx in range(0, self._num_children)
            }

    def has_children(self):
        return self._has_children
