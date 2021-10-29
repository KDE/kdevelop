#
# LLDB data formatters for Qt types
#
# SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#

def __lldb_init_module(debugger, unused):
    debugger.HandleCommand('type summary add KDevelop::Path -w kdevelop-kde -F kde.KDevPathSummaryProvider')
    debugger.HandleCommand('type summary add KTextEditor::Cursor -w kdevelop-kde -F kde.KTextEditorCursorSummaryProvider')  # noqa: E501
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
