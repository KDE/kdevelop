/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "astyle_formatter.h"

#include <QIODevice>
#include <QString>

#include <interfaces/isourceformatter.h>
#include <util/formattinghelpers.h>

#include "astyle_stringiterator.h"
#include "debug.h"

using namespace KDevelop;

namespace AStyleOptionKey {
QString forceTabs()
{
    return QStringLiteral("FillForce");
}

QString tabSpaceConversion()
{
    // The meaning of the "FillForce" key depends on whether tab or space indentation is used:
    //     * for tabs: the forceTabs argument to ASBeautifier::setTabIndentation();
    //     * for spaces: the state argument to ASFormatter::setTabSpaceConversionMode().
    return forceTabs();
}

QString fillEmptyLines()
{
    return QStringLiteral("FillEmptyLines");
}

QString bracesAdd()
{
    return QStringLiteral("BracesAdd");
}
}

AStyleFormatter::AStyleFormatter()
{
}

QString AStyleFormatter::formatSource(const QString &text, const QString& leftContext, const QString& rightContext)
{
    QString useText = leftContext + text + rightContext;

    AStyleStringIterator is(useText);
    QString output;
    QTextStream os(&output, QIODevice::WriteOnly);

    m_engine.init(&is);

    while (m_engine.hasMoreLines())
        os << QString::fromUtf8(m_engine.nextLine().c_str()) << QLatin1Char('\n');

    m_engine.init(nullptr);

    return extractFormattedTextFromContext(output, text, leftContext, rightContext, m_options[QStringLiteral("FillCount")].toInt());
}

void AStyleFormatter::updateFormatter()
{
    qCDebug(KDEV_ASTYLE) << "Updating option with: " << ISourceFormatter::optionMapToString(m_options);
    // fill
    int wsCount = m_options[QStringLiteral("FillCount")].toInt();
    if(m_options[QStringLiteral("Fill")].toString() == QLatin1String("Tabs")) {
        const bool forceTabs = m_options[AStyleOptionKey::forceTabs()].toBool();
        AStyleFormatter::setTabIndentation(wsCount, forceTabs);
    } else {
        const bool tabSpaceConversion = m_options[AStyleOptionKey::tabSpaceConversion()].toBool();
        AStyleFormatter::setSpaceIndentationAndTabSpaceConversion(wsCount, tabSpaceConversion);
    }

    AStyleFormatter::setEmptyLineFill(m_options[AStyleOptionKey::fillEmptyLines()].toBool());

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
    AStyleFormatter::setAddBracesMode(m_options[AStyleOptionKey::bracesAdd()].toBool());

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
    // fill
    setSpaceIndentationAndTabSpaceConversion(4, false);
    setEmptyLineFill(false);
    // brackets
    setBracketFormatMode(astyle::NONE_MODE);
    setBreakClosingHeaderBracketsMode(false);
    setAddBracesMode(false);
    // oneliner
    setBreakOneLineBlocksMode(true);
    setBreakOneLineStatementsMode(true);
    // blocks
    setBreakBlocksMode(false);
    setBreakClosingHeaderBlocksMode(false);
    setBreakElseIfsMode(false);
    // continuation
    setMaxInStatementIndentLength(40);
    setMinConditionalIndentLength(-1);
    // indent
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
    // padding
    setOperatorPaddingMode(false);
    setParensInsidePaddingMode(true);
    setParensOutsidePaddingMode(true);
    setParensHeaderPaddingMode(true);
    setParensUnPaddingMode(true);
    // pointer
    setPointerAlignment(astyle::PTR_ALIGN_NONE);
}

bool AStyleFormatter::predefinedStyle( const QString & style )
{
    if(style == QLatin1String("ANSI")) {
        resetStyle();
        setBracketIndent(false);
        setSpaceIndentationNoConversion(4);
        setBracketFormatMode(astyle::BREAK_MODE);
        setClassIndent(false);
        setSwitchIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if(style == QLatin1String("K&R")) {
        resetStyle();
        setBracketIndent(false);
        setSpaceIndentationNoConversion(4);
        setBracketFormatMode(astyle::ATTACH_MODE);
        setClassIndent(false);
        setSwitchIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if(style == QLatin1String("Linux")) {
        resetStyle();
        setBracketIndent(false);
        setSpaceIndentationNoConversion(8);
        setBracketFormatMode(astyle::LINUX_MODE);
        setClassIndent(false);
        setSwitchIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if(style == QLatin1String("GNU")) {
        resetStyle();
        setBlockIndent(true);
        setSpaceIndentationNoConversion(2);
        setBracketFormatMode(astyle::BREAK_MODE);
        setClassIndent(false);
        setSwitchIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if(style == QLatin1String("Java")) {
        resetStyle();
        setBracketIndent(false);
        setSpaceIndentationNoConversion(4);
        setBracketFormatMode(astyle::ATTACH_MODE);
        setSwitchIndent(false);
        return true;
    } else if (style == QLatin1String("Stroustrup")) {
        resetStyle();
        setBracketFormatMode(astyle::LINUX_MODE);
        setBlockIndent(false);
        setBracketIndent(false);
        setSpaceIndentationNoConversion(5);
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
        setSpaceIndentationNoConversion(3);
        setClassIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if (style == QLatin1String("Whitesmith")) {
        resetStyle();
        setSpaceIndentationNoConversion(4);
        setBracketFormatMode(astyle::BREAK_MODE);
        setBlockIndent(false);
        setBracketIndent(true);
        setClassIndent(true);
        setSwitchIndent(true);
        setNamespaceIndent(false);
        return true;
    } else if (style == QLatin1String("Banner")) {
        resetStyle();
        setSpaceIndentationNoConversion(4);
        setBracketFormatMode(astyle::ATTACH_MODE);
        setBlockIndent(false);
        setBracketIndent(true);
        setClassIndent(true);
        setSwitchIndent(true);
        setNamespaceIndent(false);
        return true;
    } else if (style == QLatin1String("1TBS")) {
        resetStyle();
        setSpaceIndentationNoConversion(4);
        setBracketFormatMode(astyle::LINUX_MODE);
        setAddBracesMode(true);
        setBlockIndent(false);
        setBracketIndent(false);
        setClassIndent(false);
        setSwitchIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if (style == QLatin1String("KDELibs")) {
        // https://community.kde.org/Policies/Frameworks_Coding_Style
        resetStyle();
        setSpaceIndentationAndTabSpaceConversion(4, true);
        setBracketFormatMode(astyle::LINUX_MODE);
        setPointerAlignment(astyle::PTR_ALIGN_NAME);
        setLabelIndent(true);
        setOperatorPaddingMode(true);
        setParensInsidePaddingMode(false);
        setParensOutsidePaddingMode(false);
        setParensHeaderPaddingMode(true);
        setParensUnPaddingMode(true);
        setBreakOneLineStatementsMode(false);
        setPreprocessorIndent(true);
        setSwitchIndent(false);
        setClassIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if (style == QLatin1String("Qt")) {
        // https://wiki.qt.io/Qt_Coding_Style
        resetStyle();
        setSpaceIndentationNoConversion(4);
        setPointerAlignment(astyle::PTR_ALIGN_NAME);
        setOperatorPaddingMode(true);
        setBracketFormatMode(astyle::LINUX_MODE);
        setSwitchIndent(false);
        setParensInsidePaddingMode(false);
        setParensOutsidePaddingMode(false);
        setParensHeaderPaddingMode(true);
        setParensUnPaddingMode(true);
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

void AStyleFormatter::setCStyle()
{
    m_engine.setCStyle();
}
void AStyleFormatter::setJavaStyle()
{
    m_engine.setJavaStyle();
}
void AStyleFormatter::setSharpStyle()
{
    m_engine.setSharpStyle();
}

void AStyleFormatter::setTabIndentation(int length, bool forceTabs)
{
    m_engine.setTabIndentation(length, forceTabs);
    m_options[QStringLiteral("Fill")] = QStringLiteral("Tabs");
    m_options[QStringLiteral("FillCount")] = length;
    m_options[AStyleOptionKey::forceTabs()] = forceTabs;

    m_engine.setTabSpaceConversionMode(false);
}

void AStyleFormatter::setSpaceIndentationAndTabSpaceConversion(int length, bool tabSpaceConversion)
{
    // set ASBeautifier::shouldForceTabIndentation to false
    m_engine.setTabIndentation(length, false);

    m_engine.setSpaceIndentation(length);
    m_options[QStringLiteral("Fill")] = QStringLiteral("Spaces");
    m_options[QStringLiteral("FillCount")] = length;

    m_options[AStyleOptionKey::tabSpaceConversion()] = tabSpaceConversion;
    m_engine.setTabSpaceConversionMode(tabSpaceConversion);
}

void AStyleFormatter::setSpaceIndentationNoConversion(int length)
{
    setSpaceIndentationAndTabSpaceConversion(length, false);
}

void AStyleFormatter::setEmptyLineFill(bool on)
{
    m_options[AStyleOptionKey::fillEmptyLines()] = on;
    m_engine.setEmptyLineFill(on);
}

void AStyleFormatter::setBlockIndent(bool on)
{
    m_options[QStringLiteral("IndentBlocks")] = on;
    m_engine.setBlockIndent(on);
}

void AStyleFormatter::setBracketIndent(bool on)
{
    m_options[QStringLiteral("IndentBrackets")] = on;
    m_engine.setBraceIndent(on);
}

void AStyleFormatter::setCaseIndent(bool on)
{
    m_options[QStringLiteral("IndentCases")] = on;
    m_engine.setCaseIndent(on);
}

void AStyleFormatter::setClassIndent(bool on)
{
    m_options[QStringLiteral("IndentClasses")] = on;
    m_engine.setClassIndent(on);
}

void AStyleFormatter::setLabelIndent(bool on)
{
    m_options[QStringLiteral("IndentLabels")] = on;
    m_engine.setLabelIndent(on);
}

void AStyleFormatter::setNamespaceIndent(bool on)
{
    m_options[QStringLiteral("IndentNamespaces")] = on;
    m_engine.setNamespaceIndent(on);
}

void AStyleFormatter::setPreprocessorIndent(bool on)
{
    m_options[QStringLiteral("IndentPreprocessors")] = on;
    m_engine.setPreprocDefineIndent(on);
}

void AStyleFormatter::setSwitchIndent(bool on)
{
    m_options[QStringLiteral("IndentSwitches")] = on;
    m_engine.setSwitchIndent(on);
}

void AStyleFormatter::setMaxInStatementIndentLength(int max)
{
    m_options[QStringLiteral("MaxStatement")] = max;
    m_engine.setMaxInStatementIndentLength(max);
}

void AStyleFormatter::setMinConditionalIndentLength(int min)
{
    m_options[QStringLiteral("MinConditional")] = min;
    m_engine.setMinConditionalIndentOption(min);
    m_engine.setMinConditionalIndentLength();
}

void AStyleFormatter::setAfterParens(bool on)
{
    m_options[QStringLiteral("AfterParens")] = on;
    m_engine.setAfterParenIndent(on);
}

void AStyleFormatter::setContinuation(int n)
{
    m_options[QStringLiteral("Continuation")] = n;
    m_engine.setContinuationIndentation(n);
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
    m_engine.setBraceFormatMode(mode);
}

void AStyleFormatter::setBreakClosingHeaderBracketsMode(bool state)
{
    m_options[QStringLiteral("BracketsCloseHeaders")] = state;
    m_engine.setBreakClosingHeaderBracketsMode(state);
}

void AStyleFormatter::setAddBracesMode(bool state)
{
    m_options[AStyleOptionKey::bracesAdd()] = state;
    m_engine.setAddBracesMode(state);
}

void AStyleFormatter::setBreakBlocksMode(bool state)
{
    m_options[QStringLiteral("BlockBreak")] = state;
    m_engine.setBreakBlocksMode(state);
}

void AStyleFormatter::setBreakElseIfsMode(bool state)
{
    m_options[QStringLiteral("BlockIfElse")] = state;
    m_engine.setBreakElseIfsMode(state);
}

void AStyleFormatter::setBreakClosingHeaderBlocksMode(bool state)
{
    m_options[QStringLiteral("BlockBreakAll")] = state;
    m_engine.setBreakClosingHeaderBlocksMode(state);
}

void AStyleFormatter::setOperatorPaddingMode(bool mode)
{
    m_options[QStringLiteral("PadOperators")] = mode;
    m_engine.setOperatorPaddingMode(mode);
}

void AStyleFormatter::setParensOutsidePaddingMode(bool mode)
{
    m_options[QStringLiteral("PadParenthesesOut")] = mode;
    m_engine.setParensOutsidePaddingMode(mode);
}

void AStyleFormatter::setParensInsidePaddingMode(bool mode)
{
    m_options[QStringLiteral("PadParenthesesIn")] = mode;
    m_engine.setParensInsidePaddingMode(mode);
}

void AStyleFormatter::setParensHeaderPaddingMode(bool mode) {
    m_options[QStringLiteral("PadParenthesesHeader")] = mode;
    m_engine.setParensHeaderPaddingMode(mode);
}

void AStyleFormatter::setParensUnPaddingMode(bool state)
{
    m_options[QStringLiteral("PadParenthesesUn")] = state;
    m_engine.setParensUnPaddingMode(state);
}

void AStyleFormatter::setBreakOneLineBlocksMode(bool state)
{
    m_options[QStringLiteral("KeepBlocks")] = !state;
    m_engine.setBreakOneLineBlocksMode(state);
}

void AStyleFormatter::setBreakOneLineStatementsMode(bool state)
{
    m_options[QStringLiteral("KeepStatements")] = !state;
    m_engine.setBreakOneLineStatementsMode(state);
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
    m_engine.setPointerAlignment(alignment);
}
