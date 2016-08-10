#
# LLDB data formatters for Qt types
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

import lldb

from helpers import *

def __lldb_init_module(debugger, unused):
    debugger.HandleCommand('type summary add KDevelop::Path -w kdevelop-kde -F kde.KDevPathSummaryProvider')
    debugger.HandleCommand('type summary add KTextEditor::Cursor -w kdevelop-kde -F kde.KTextEditorCursorSummaryProvider')
    debugger.HandleCommand('type summary add KTextEditor::Range -w kdevelop-kde -F kde.KTextEditorRangeSummaryProvider')

    debugger.HandleCommand('type category enable kdevelop-kde')


def KDevPathSummaryProvider(valobj, internal_dict):
    segments = [seg.GetSummary() for seg in valobj.GetChildMemberWithName('m_data')]
    return '(' + ', '.join(segments) + ')' if segments else None


def KTextEditorCursorSummaryProvider(valobj, internal_dict):
    line = valobj.GetChildMemberWithName('m_line').GetValueAsUnsigned(0)
    col = valobj.GetChildMemberWithName('m_column').GetValueAsUnsigned(0)
    return '({}, {})'.format(line, col)


def KTextEditorRangeSummaryProvider(valobj, internal_dict):
    start = valobj.GetChildMemberWithName('m_start')
    end = valobj.GetChildMemberWithName('m_end')

    st_line = start.GetChildMemberWithName('m_line').GetValueAsUnsigned(0)
    st_col = start.GetChildMemberWithName('m_column').GetValueAsUnsigned(0)
    ed_line = end.GetChildMemberWithName('m_line').GetValueAsUnsigned(0)
    ed_col = end.GetChildMemberWithName('m_column').GetValueAsUnsigned(0)
    return '[({}, {}) -> ({}, {})]'.format(st_line, st_col, ed_line, ed_col)
