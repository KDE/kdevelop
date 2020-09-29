/* This file is part of KDevelop
*  Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.

*/
#include "astyle_formatter.h"

#include <QString>

#include <interfaces/isourceformatter.h>
#include <util/formattinghelpers.h>

#include "astyle_stringiterator.h"
#include "debug.h"

using namespace KDevelop;

AStyleFormatter::AStyleFormatter()
: ASFormatter()
{
}

QString AStyleFormatter::formatSource(const QString &text, const QString& leftContext, const QString& rightContext)
{
    QString useText = leftContext + text + rightContext;

    AStyleStringIterator is(useText);
    QString output;
    QTextStream os(&output, QIODevice::WriteOnly);

    init(&is);

    while(hasMoreLines())
        os << QString::fromUtf8(nextLine().c_str()) << QLatin1Char('\n');

    init(nullptr);

    return extractFormattedTextFromContext(output, text, leftContext, rightContext, m_options[QStringLiteral("FillCount")].toInt());
}

void AStyleFormatter::updateFormatter()
{
    qCDebug(KDEV_ASTYLE) << "Updating option with: " << ISourceFormatter::optionMapToString(m_options);
    // fill
    int wsCount = m_options[QStringLiteral("FillCount")].toInt();
    if(m_options[QStringLiteral("Fill")].toString() == QLatin1String("Tabs")) {
        ///TODO: rename FillForce somehow...
        bool force = m_options[QStringLiteral("FillForce")].toBool();
        AStyleFormatter::setTabSpaceConversionMode(false);
        AStyleFormatter::setTabIndentation(wsCount, force );
    } else {
        AStyleFormatter::setSpaceIndentation(wsCount);
        AStyleFormatter::setTabSpaceConversionMode(m_options[QStringLiteral("FillForce")].toBool());
    }

    AStyleFormatter::setEmptyLineFill(m_options[QStringLiteral("Fill_EmptyLines")].toBool());

    // indent
    AStyleFormatter::setSwitchIndent(m_options[QStringLiteral("IndentSwitches")].toBool());
    AStyleFormatter::setClassIndent(m_options[QStringLiteral("IndentClasses")].toBool());
    AStyleFormatter::setCaseIndent(m_options[QStringLiteral("IndentCases")].toBool());
    AStyleFormatter::setBracketIndent(m_options[QStringLiteral("IndentBrackets")].toBool());
    AStyleFormatter::setNamespaceIndent(m_options[QStringLiteral("IndentNamespaces")].toBool());
    AStyleFormatter::setLabelIndent(m_options[QStringLiteral("IndentLabels")].toBool());
    AStyleFormatter::setBlockIndent(m_options[QStringLiteral("IndentBlocks")].toBool());
    AStyleFormatter::setPreprocessorIndent(m_options[QStringLiteral("IndentPreprocessors")].toBool());
    AStyleFormatter::setAfterParens(m_options[QStringLiteral("AfterParens")].toBool());
    AStyleFormatter::setContinuation(m_options[QStringLiteral("Continuation")].toInt());

    // continuation
    AStyleFormatter::setMaxInStatementIndentLength(m_options[QStringLiteral("MaxStatement")].toInt());
    if(m_options[QStringLiteral("MinConditional")].toInt() != -1)
        AStyleFormatter::setMinConditionalIndentLength(m_options[QStringLiteral("MinConditional")].toInt());

    // brackets
    QString s = m_options[QStringLiteral("Brackets")].toString();
    if(s == QLatin1String("Break"))
        AStyleFormatter::setBracketFormatMode(astyle::BREAK_MODE);
    else if(s == QLatin1String("Attach"))
        AStyleFormatter::setBracketFormatMode(astyle::ATTACH_MODE);
    else if(s == QLatin1String("Linux"))
        AStyleFormatter::setBracketFormatMode(astyle::LINUX_MODE);
    else if(s == QLatin1String("Stroustrup"))
        // In astyle 2.06 BracketMode STROUSTRUP_MODE was removed and LINUX_MODE is the replacement
        AStyleFormatter::setBracketFormatMode(astyle::LINUX_MODE);
    else if(s == QLatin1String("Horstmann") || s == QLatin1String("RunInMode"))
        AStyleFormatter::setBracketFormatMode(astyle::RUN_IN_MODE);
    else
        AStyleFormatter::setBracketFormatMode(astyle::NONE_MODE);

    AStyleFormatter::setBreakClosingHeaderBracketsMode(m_options[QStringLiteral("BracketsCloseHeaders")].toBool());
    // blocks
    AStyleFormatter::setBreakBlocksMode(m_options[QStringLiteral("BlockBreak")].toBool());
    AStyleFormatter::setBreakClosingHeaderBlocksMode(m_options[QStringLiteral("BlockBreakAll")].toBool());
    AStyleFormatter::setBreakElseIfsMode(m_options[QStringLiteral("BlockIfElse")].toBool());

    // padding
    AStyleFormatter::setOperatorPaddingMode(m_options[QStringLiteral("PadOperators")].toBool());
    AStyleFormatter::setParensInsidePaddingMode(m_options[QStringLiteral("PadParenthesesIn")].toBool());
    AStyleFormatter::setParensOutsidePaddingMode(m_options[QStringLiteral("PadParenthesesOut")].toBool());
    AStyleFormatter::setParensHeaderPaddingMode(m_options[QStringLiteral("PadParenthesesHeader")].toBool());
    AStyleFormatter::setParensUnPaddingMode(m_options[QStringLiteral("PadParenthesesUn")].toBool());

    // oneliner
    AStyleFormatter::setBreakOneLineBlocksMode(!m_options[QStringLiteral("KeepBlocks")].toBool());
    AStyleFormatter::setBreakOneLineStatementsMode(!m_options[QStringLiteral("KeepStatements")].toBool());

    // pointer
    s = m_options[QStringLiteral("PointerAlign")].toString();
    if(s == QLatin1String("Name"))
        AStyleFormatter::setPointerAlignment(astyle::PTR_ALIGN_NAME);
    else if(s == QLatin1String("Middle"))
        AStyleFormatter::setPointerAlignment(astyle::PTR_ALIGN_MIDDLE);
    else if(s == QLatin1String("Type"))
        AStyleFormatter::setPointerAlignment(astyle::PTR_ALIGN_TYPE);
    else
        AStyleFormatter::setPointerAlignment(astyle::PTR_ALIGN_NONE);
}

void AStyleFormatter::resetStyle()
{
    setSpaceIndentation(4);
    setBracketFormatMode(astyle::NONE_MODE);
    setBreakOneLineBlocksMode(true);
    setBreakOneLineStatementsMode(true);
    // blocks
    setBreakBlocksMode(false);
    setBreakClosingHeaderBlocksMode(false);
    setBreakElseIfsMode(false);
    setBreakClosingHeaderBracketsMode(false);
    //indent
    setTabIndentation(4, false);
    setEmptyLineFill(false);
    setMaxInStatementIndentLength(40);
    setMinConditionalIndentLength(-1);
    setSwitchIndent(true);
    setClassIndent(true);
    setCaseIndent(false);
    setBracketIndent(false);
    setNamespaceIndent(true);
    setLabelIndent(true);
    setBlockIndent(false);
    setPreprocessorIndent(false);
    setAfterParens(false);
    setContinuation(1);
    //padding
    setOperatorPaddingMode(false);
    setParensInsidePaddingMode(true);
    setParensOutsidePaddingMode(true);
    setParensHeaderPaddingMode(true);
    setParensUnPaddingMode(true);
}

bool AStyleFormatter::predefinedStyle( const QString & style )
{
    if(style == QLatin1String("ANSI")) {
        resetStyle();
        setBracketIndent(false);
        setSpaceIndentation(4);
        setBracketFormatMode(astyle::BREAK_MODE);
        setClassIndent(false);
        setSwitchIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if(style == QLatin1String("K&R")) {
        resetStyle();
        setBracketIndent(false);
        setSpaceIndentation(4);
        setBracketFormatMode(astyle::ATTACH_MODE);
        setClassIndent(false);
        setSwitchIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if(style == QLatin1String("Linux")) {
        resetStyle();
        setBracketIndent(false);
        setSpaceIndentation(8);
        setBracketFormatMode(astyle::LINUX_MODE);
        setClassIndent(false);
        setSwitchIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if(style == QLatin1String("GNU")) {
        resetStyle();
        setBlockIndent(true);
        setSpaceIndentation(2);
        setBracketFormatMode(astyle::BREAK_MODE);
        setClassIndent(false);
        setSwitchIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if(style == QLatin1String("Java")) {
        resetStyle();
        setJavaStyle();
        setBracketIndent(false);
        setSpaceIndentation(4);
        setBracketFormatMode(astyle::ATTACH_MODE);
        setSwitchIndent(false);
        return true;
    } else if (style == QLatin1String("Stroustrup")) {
        resetStyle();
        setBracketFormatMode(astyle::LINUX_MODE);
        setBlockIndent(false);
        setBracketIndent(false);
        setSpaceIndentation(5);
        setClassIndent(false);
        setSwitchIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if (style == QLatin1String("Horstmann")) {
        resetStyle();
        setBracketFormatMode(astyle::RUN_IN_MODE);
        setBlockIndent(false);
        setBracketIndent(false);
        setSwitchIndent(true);
        setSpaceIndentation(3);
        setClassIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if (style == QLatin1String("Whitesmith")) {
        resetStyle();
        setSpaceIndentation(4);
        setBracketFormatMode(astyle::BREAK_MODE);
        setBlockIndent(false);
        setBracketIndent(true);
        setClassIndent(true);
        setSwitchIndent(true);
        setNamespaceIndent(false);
        return true;
    } else if (style == QLatin1String("Banner")) {
        resetStyle();
        setSpaceIndentation(4);
        setBracketFormatMode(astyle::ATTACH_MODE);
        setBlockIndent(false);
        setBracketIndent(true);
        setClassIndent(true);
        setSwitchIndent(true);
        setNamespaceIndent(false);
        return true;
    } else if (style == QLatin1String("1TBS")) {
        resetStyle();
        setSpaceIndentation(4);
        setBracketFormatMode(astyle::LINUX_MODE);
        setBlockIndent(false);
        setBracketIndent(false);
        setAddBracketsMode(true);
        setClassIndent(false);
        setSwitchIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if (style == QLatin1String("KDELibs")) {
        // https://community.kde.org/Policies/Frameworks_Coding_Style
        resetStyle();
        setSpaceIndentation(4);
        setBracketFormatMode(astyle::LINUX_MODE);
        setPointerAlignment(astyle::PTR_ALIGN_NAME);
        setLabelIndent(true);
        setOperatorPaddingMode(true);
        setParensInsidePaddingMode(false);
        setParensOutsidePaddingMode(false);
        setParensHeaderPaddingMode(true);
        setParensUnPaddingMode(true);
        setBreakOneLineStatementsMode(false);
        setTabSpaceConversionMode(true);
        setPreprocessorIndent(true);
        setSwitchIndent(false);
        setClassIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if (style == QLatin1String("Qt")) {
        // https://wiki.qt.io/Qt_Coding_Style
        resetStyle();
        setPointerAlignment(astyle::PTR_ALIGN_NAME);
        setOperatorPaddingMode(true);
        setBracketFormatMode(astyle::LINUX_MODE);
        setSwitchIndent(false);
        setParensInsidePaddingMode(false);
        setParensOutsidePaddingMode(false);
        setParensHeaderPaddingMode(true);
        setParensUnPaddingMode(true);
        setSpaceIndentation(4);
        setClassIndent(false);
        setNamespaceIndent(false);
        return true;
    }

    return false;
}

QVariant AStyleFormatter::option(const QString &key) const
{
    if(!m_options.contains(key))
        qCDebug(KDEV_ASTYLE) << "Missing option name " << key;
    return m_options[key];
}

void AStyleFormatter::loadStyle(const QString &content)
{
    m_options = ISourceFormatter::stringToOptionMap(content);
    updateFormatter();
}

QString AStyleFormatter::saveStyle() const
{
    return ISourceFormatter::optionMapToString(m_options);
}

void AStyleFormatter::setTabIndentation(int length, bool forceTabs)
{
    ASFormatter::setTabIndentation(length, forceTabs);
    m_options[QStringLiteral("Fill")] = QStringLiteral("Tabs");
    m_options[QStringLiteral("FillForce")] = forceTabs;
    m_options[QStringLiteral("FillCount")] = length;
}

void AStyleFormatter::setSpaceIndentation(int length)
{
    ASFormatter::setSpaceIndentation(length);
    m_options[QStringLiteral("Fill")] = QStringLiteral("Spaces");
    m_options[QStringLiteral("FillCount")] = length;
}

void AStyleFormatter::setTabSpaceConversionMode(bool mode)
{
    m_options[QStringLiteral("FillForce")] = mode;
    ASFormatter::setTabSpaceConversionMode(mode);
}

void AStyleFormatter::setFillEmptyLines(bool on)
{
    m_options[QStringLiteral("FillEmptyLines")] = on;
    ASFormatter::setEmptyLineFill(on);
}

void AStyleFormatter::setBlockIndent(bool on)
{
    m_options[QStringLiteral("IndentBlocks")] = on;
    ASFormatter::setBlockIndent(on);
}

void AStyleFormatter::setBracketIndent(bool on)
{
    m_options[QStringLiteral("IndentBrackets")] = on;
    ASFormatter::setBraceIndent(on);
}

void AStyleFormatter::setCaseIndent(bool on)
{
    m_options[QStringLiteral("IndentCases")] = on;
    ASFormatter::setCaseIndent(on);
}

void AStyleFormatter::setClassIndent(bool on)
{
    m_options[QStringLiteral("IndentClasses")] = on;
    ASFormatter::setClassIndent(on);
}

void AStyleFormatter::setLabelIndent(bool on)
{
    m_options[QStringLiteral("IndentLabels")] = on;
    ASFormatter::setLabelIndent(on);
}

void AStyleFormatter::setNamespaceIndent(bool on)
{
    m_options[QStringLiteral("IndentNamespaces")] = on;
    ASFormatter::setNamespaceIndent(on);
}

void AStyleFormatter::setPreprocessorIndent(bool on)
{
    m_options[QStringLiteral("IndentPreprocessors")] = on;
    ASFormatter::setPreprocDefineIndent(on);
}

void AStyleFormatter::setSwitchIndent(bool on)
{
    m_options[QStringLiteral("IndentSwitches")] = on;
    ASFormatter::setSwitchIndent(on);
}

void AStyleFormatter::setMaxInStatementIndentLength(int max)
{
    m_options[QStringLiteral("MaxStatement")] = max;
    ASFormatter::setMaxInStatementIndentLength(max);
}

void AStyleFormatter::setMinConditionalIndentLength(int min)
{
    m_options[QStringLiteral("MinConditional")] = min;
    ASFormatter::setMinConditionalIndentOption(min);
    ASFormatter::setMinConditionalIndentLength();
}

void AStyleFormatter::setAfterParens(bool on)
{
    m_options[QStringLiteral("AfterParens")] = on;
    ASFormatter::setAfterParenIndent(on);
}

void AStyleFormatter::setContinuation(int n)
{
    m_options[QStringLiteral("Continuation")] = n;
    ASFormatter::setContinuationIndentation(n);
}

void AStyleFormatter::setBracketFormatMode(astyle::BraceMode mode)
{
    switch (mode) {
    case astyle::NONE_MODE:
        m_options[QStringLiteral("Brackets")] = QString();
        break;
    case astyle::ATTACH_MODE:
        m_options[QStringLiteral("Brackets")] = QStringLiteral("Attach");
        break;
    case astyle::BREAK_MODE:
        m_options[QStringLiteral("Brackets")] = QStringLiteral("Break");
        break;
    case astyle::LINUX_MODE:
        m_options[QStringLiteral("Brackets")] = QStringLiteral("Linux");
        break;
    case astyle::RUN_IN_MODE:
        m_options[QStringLiteral("Brackets")] = QStringLiteral("RunInMode");
        break;
    }
    ASFormatter::setBraceFormatMode(mode);
}

void AStyleFormatter::setBreakClosingHeaderBracketsMode(bool state)
{
    m_options[QStringLiteral("BracketsCloseHeaders")] = state;
    ASFormatter::setBreakClosingHeaderBracketsMode(state);
}

void AStyleFormatter::setBreakBlocksMode(bool state)
{
    m_options[QStringLiteral("BlockBreak")] = state;
    ASFormatter::setBreakBlocksMode(state);
}

void AStyleFormatter::setBreakElseIfsMode(bool state)
{
    m_options[QStringLiteral("BlockIfElse")] = state;
    ASFormatter::setBreakElseIfsMode(state);
}

void AStyleFormatter::setBreakClosingHeaderBlocksMode(bool state)
{
    m_options[QStringLiteral("BlockBreakAll")] = state;
    ASFormatter::setBreakClosingHeaderBlocksMode(state);
}

void AStyleFormatter::setOperatorPaddingMode(bool mode)
{
    m_options[QStringLiteral("PadOperators")] = mode;
    ASFormatter::setOperatorPaddingMode(mode);
}

void AStyleFormatter::setParensOutsidePaddingMode(bool mode)
{
    m_options[QStringLiteral("PadParenthesesOut")] = mode;
    ASFormatter::setParensOutsidePaddingMode(mode);
}

void AStyleFormatter::setParensInsidePaddingMode(bool mode)
{
    m_options[QStringLiteral("PadParenthesesIn")] = mode;
    ASFormatter::setParensInsidePaddingMode(mode);
}

void AStyleFormatter::setParensHeaderPaddingMode(bool mode) {
    m_options[QStringLiteral("PadParenthesesHeader")] = mode;
    ASFormatter::setParensHeaderPaddingMode(mode);
}

void AStyleFormatter::setParensUnPaddingMode(bool state)
{
    m_options[QStringLiteral("PadParenthesesUn")] = state;
    ASFormatter::setParensUnPaddingMode(state);
}

void AStyleFormatter::setBreakOneLineBlocksMode(bool state)
{
    m_options[QStringLiteral("KeepBlocks")] = !state;
    ASFormatter::setBreakOneLineBlocksMode(state);
}

void AStyleFormatter::setBreakOneLineStatementsMode(bool state)
{
    m_options[QStringLiteral("KeepStatements")] = !state;
    ASFormatter::setBreakOneLineStatementsMode(state);
}

void AStyleFormatter::setPointerAlignment(astyle::PointerAlign alignment)
{
    switch (alignment) {
        case astyle::PTR_ALIGN_NONE:
            m_options[QStringLiteral("PointerAlign")] = QStringLiteral("None");
            break;
        case astyle::PTR_ALIGN_NAME:
            m_options[QStringLiteral("PointerAlign")] = QStringLiteral("Name");
            break;
        case astyle::PTR_ALIGN_MIDDLE:
            m_options[QStringLiteral("PointerAlign")] = QStringLiteral("Middle");
            break;
        case astyle::PTR_ALIGN_TYPE:
            m_options[QStringLiteral("PointerAlign")] = QStringLiteral("Type");
            break;
    }
    ASFormatter::setPointerAlignment(alignment);
}
