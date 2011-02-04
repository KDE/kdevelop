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

#include <QRadioButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QString>
#include <KDebug>

#include <interfaces/isourceformatter.h>
#include "astyle_stringiterator.h"

AStyleFormatter::AStyleFormatter()
: ASFormatter()
{
}

// AStyleFormatter::AStyleFormatter(const QMap<QString, QVariant>& options)
// {
//     setOptions(options);
// }

///Matches the given prefix to the given text, ignoring all whitespace, but not ignoring newlines
///Returns -1 if mismatched, else the position in @p text where the @p prefix match ends
int matchPrefixIgnoringWhitespace(QString text, QString prefix)
{
    int prefixPos = 0;
    int textPos = 0;
    while (prefixPos < prefix.length() && textPos < text.length()) {
        while (prefixPos < prefix.length() && prefix[prefixPos].isSpace() && prefix[prefixPos] != '\n')
            ++prefixPos;
        while (textPos < text.length() && text[textPos].isSpace() && prefix[prefixPos] != '\n')
            ++textPos;

        if(prefixPos == prefix.length() || textPos == text.length())
            break;

        if(prefix[prefixPos] != text[textPos])
            return -1;
        ++prefixPos;
        ++textPos;
    }
    return textPos;
}

//Returns the closest newline position before the actual text, or -1
int leadingNewLine(QString str) {
    int ret = -1;
    for(int a = 0; a < str.length(); ++a) {
        if(!str[a].isSpace())
            return ret;
        if(str[a] == '\n')
            ret = a;
    }
    return ret;
}

int firstNonWhiteSpace(QString str) {
    for(int a = 0; a < str.length(); ++a)
        if(!str[a].isSpace())
            return a;
    return -1;
}

static QString reverse( const QString& str ) {
  QString ret;
  for(int a = str.length()-1; a >= 0; --a)
      ret.append(str[a]);
  
  return ret;
}

///Removes parts of the white-space at the start that are in @p output but not in @p text
QString equalizeWhiteSpaceAtStart(QString original, QString output, bool removeIndent = false) {
    int outputNewline = leadingNewLine(output);
    if(outputNewline != -1) {
        if(leadingNewLine(original) != -1)
            return output.mid(outputNewline); //Exactly include the leading newline as in the original text
        else
            output = output.mid(outputNewline+1); //Skip the leading newline, the orginal had none as well
    }

    if(removeIndent && output[0].isSpace() && !original[0].isSpace()) {
        //The original text has no leading white space, remove all leading white space
        int nonWhite = firstNonWhiteSpace(output);
        if(nonWhite != -1)
            output = output.mid(nonWhite);
        else
            output.clear();
    }
    return output;
}

QString AStyleFormatter::formatSource(const QString &text, const QString& leftContext, const QString& rightContext)
{
    QString useText = leftContext + text + rightContext;

//     kDebug() << "left context:" << leftContext;
//     kDebug() << "right context:" << rightContext;

    QStringList textLines = text.split("\n");

    AStyleStringIterator is(useText);
    QString output;
    QTextStream os(&output, QIODevice::WriteOnly);

    init(&is);

    while(hasMoreLines()) {
//         os << indent; // add optional indentation
        os << QString::fromUtf8(nextLine().c_str()) << endl;
    }

    init(0);

    //Now remove "leftContext" and "rightContext" from the sides

    if(!leftContext.isEmpty()) {
        int endOfLeftContext = matchPrefixIgnoringWhitespace(output, leftContext);
        if(endOfLeftContext == -1) {
            kWarning() << "problem matching the left context";
            return formatSource(text); //Re-format without context
        }
        output = output.mid(endOfLeftContext);
        output = equalizeWhiteSpaceAtStart(text, output);
    }

    if(!rightContext.isEmpty()) {
        //Add a whitespace behind the text for matching, so that we definitely capture all trailing whitespace
        int endOfText = matchPrefixIgnoringWhitespace(output, text+" ");
        if(endOfText == -1) {
            kWarning() << "problem matching the text while formatting";
            return formatSource(text); //Re-format without context
        }
        output = output.left(endOfText);
        output = reverse(equalizeWhiteSpaceAtStart(reverse(text), reverse(output), true));
    }

    return output;
}

void AStyleFormatter::setOption(const QString &key, const QVariant &value)
{
    m_options[key] = value;
}

// void AStyleFormatter::setOptions(const QMap<QString, QVariant> &options)
// {
//     m_options = options;
//     updateFormatter();
// }

void AStyleFormatter::updateFormatter()
{
    kDebug() << "Updating option with: " << KDevelop::ISourceFormatter::optionMapToString(m_options) << endl;
    // fill
    int wsCount = m_options["FillCount"].toInt();
    if(m_options["Fill"].toString() == "Tabs") {
        ///TODO: rename FillForce somehow...
        bool force = m_options["FillForce"].toBool();
        AStyleFormatter::setTabSpaceConversionMode(false);
        AStyleFormatter::setTabIndentation(wsCount, force );
        m_indentString = "\t";
    } else {
        AStyleFormatter::setSpaceIndentation(wsCount);
        m_indentString = "";
        m_indentString.fill(' ', wsCount);

        AStyleFormatter::setTabSpaceConversionMode(m_options["FillForce"].toBool());
    }

    AStyleFormatter::setEmptyLineFill(m_options["Fill_EmptyLines"].toBool());

    // indent
    AStyleFormatter::setSwitchIndent(m_options["IndentSwitches"].toBool());
    AStyleFormatter::setClassIndent(m_options["IndentClasses"].toBool());
    AStyleFormatter::setCaseIndent(m_options["IndentCases"].toBool());
    AStyleFormatter::setBracketIndent(m_options["IndentBrackets"].toBool());
    AStyleFormatter::setNamespaceIndent(m_options["IndentNamespaces"].toBool());
    AStyleFormatter::setLabelIndent(m_options["IndentLabels"].toBool());
    AStyleFormatter::setBlockIndent(m_options["IndentBlocks"].toBool());
    AStyleFormatter::setPreprocessorIndent(m_options["IndentPreprocessors"].toBool());

    // continuation
    AStyleFormatter::setMaxInStatementIndentLength(m_options["MaxStatement"].toInt());
    if(m_options["MinConditional"].toInt() != -1)
        AStyleFormatter::setMinConditionalIndentLength(m_options["MinConditional"].toInt());

    // brackets
    QString s = m_options["Brackets"].toString();
    if(s == "Break")
        AStyleFormatter::setBracketFormatMode(astyle::BREAK_MODE);
    else if(s == "Attach")
        AStyleFormatter::setBracketFormatMode(astyle::ATTACH_MODE);
    else if(s == "Linux")
        AStyleFormatter::setBracketFormatMode(astyle::BDAC_MODE);
    else if(s == "Stroustrup")
        AStyleFormatter::setBracketFormatMode(astyle::STROUSTRUP_MODE);
    else if(s == "Horstmann")
        AStyleFormatter::setBracketFormatMode(astyle::HORSTMANN_MODE);
    else
        AStyleFormatter::setBracketFormatMode(astyle::NONE_MODE);

    AStyleFormatter::setBreakClosingHeaderBracketsMode(m_options["BracketsCloseHeaders"].toBool());
    // blocks
    AStyleFormatter::setBreakBlocksMode(m_options["BlockBreak"].toBool());
    AStyleFormatter::setBreakClosingHeaderBlocksMode(m_options["BlockBreakAll"].toBool());
    AStyleFormatter::setBreakElseIfsMode(m_options["BlockIfElse"].toBool());

    // padding
    AStyleFormatter::setOperatorPaddingMode(m_options["PadOperators"].toBool());
    AStyleFormatter::setParensInsidePaddingMode(m_options["PadParenthesesIn"].toBool());
    AStyleFormatter::setParensOutsidePaddingMode(m_options["PadParenthesesOut"].toBool());
    AStyleFormatter::setParensUnPaddingMode(m_options["PadParenthesesUn"].toBool());

    // oneliner
    AStyleFormatter::setBreakOneLineBlocksMode(!m_options["KeepBlocks"].toBool());
    AStyleFormatter::setSingleStatementsMode(!m_options["KeepStatements"].toBool());

    // pointer
    s = m_options["PointerAlign"].toString();
    if(s == "Name")
        AStyleFormatter::setPointerAlignment(astyle::ALIGN_NAME);
    else if(s == "Middle")
        AStyleFormatter::setPointerAlignment(astyle::ALIGN_MIDDLE);
    else if(s == "Type")
        AStyleFormatter::setPointerAlignment(astyle::ALIGN_TYPE);
    else
        AStyleFormatter::setPointerAlignment(astyle::ALIGN_NONE);
}

void AStyleFormatter::resetStyle()
{
    setSpaceIndentation(4);
    setBracketFormatMode(astyle::NONE_MODE);
    setBreakOneLineBlocksMode(true);
    setSingleStatementsMode(true);
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
    //padding
    setOperatorPaddingMode(false);
    setParensInsidePaddingMode(true);
    setParensOutsidePaddingMode(true);
    setParensUnPaddingMode(true);
}

bool AStyleFormatter::predefinedStyle( const QString & style )
{
    if(style == "ANSI") {
        resetStyle();
        setBracketIndent(false);
        setSpaceIndentation(4);
        setBracketFormatMode(astyle::BREAK_MODE);
        setClassIndent(false);
        setSwitchIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if(style == "K&R") {
        resetStyle();
        setBracketIndent(false);
        setSpaceIndentation(4);
        setBracketFormatMode(astyle::ATTACH_MODE);
        setClassIndent(false);
        setSwitchIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if(style == "Linux") {
        resetStyle();
        setBracketIndent(false);
        setSpaceIndentation(8);
        setBracketFormatMode(astyle::BDAC_MODE);
        setClassIndent(false);
        setSwitchIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if(style == "GNU") {
        resetStyle();
        setBlockIndent(true);
        setSpaceIndentation(2);
        setBracketFormatMode(astyle::BREAK_MODE);
        setClassIndent(false);
        setSwitchIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if(style == "Java") {
        resetStyle();
        setJavaStyle();
        setBracketIndent(false);
        setSpaceIndentation(4);
        setBracketFormatMode(astyle::ATTACH_MODE);
        setSwitchIndent(false);
        return true;
    } else if (style == "Stroustrup") {
        resetStyle();
        setBracketFormatMode(astyle::STROUSTRUP_MODE);
        setBlockIndent(false);
        setBracketIndent(false);
        if (!getIndentManuallySet())
        {
            if (getIndentString() == "\t")
                setTabIndentation(5, getForceTabIndentation());
            else
                setSpaceIndentation(5);
        }
        setClassIndent(false);
        setSwitchIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if (style == "Horstmann") {
        resetStyle();
        setBracketFormatMode(astyle::HORSTMANN_MODE);
        setBlockIndent(false);
        setBracketIndent(false);
        setSwitchIndent(true);
        if (!getIndentManuallySet())
        {
            if (getIndentString() == "\t")
                setTabIndentation(3, getForceTabIndentation());
            else
                setSpaceIndentation(3);
        }
        setClassIndent(false);
        setNamespaceIndent(false);
        return true;
    } else if (style == "Whitesmith") {
        resetStyle();
        setSpaceIndentation(4);
        setBracketFormatMode(astyle::BREAK_MODE);
        setBlockIndent(false);
        setBracketIndent(true);
        setClassIndent(true);
        setSwitchIndent(true);
        setNamespaceIndent(false);
        return true;
    } else if (style == "Banner") {
        resetStyle();
        setSpaceIndentation(4);
        setBracketFormatMode(astyle::ATTACH_MODE);
        setBlockIndent(false);
        setBracketIndent(true);
        setClassIndent(true);
        setSwitchIndent(true);
        setNamespaceIndent(false);
        return true;
    } else if (style == "1TBS") {
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
    }

    return false;
}

QVariant AStyleFormatter::option(const QString &key)
{
    if(!m_options.contains(key))
        kDebug() << "Missing option name " << key << endl;
    return m_options[key];
}

QString AStyleFormatter::indentString()
{
    return QString(getIndentString().c_str());
}

//     name, "BlockBreak=0,BlockBreakAll=0,BlockIfElse=0,"
//     "Brackets=Break,BracketsCloseHeaders=0,FStyle=,Fill=Tabs,"
//     "FillCount=4,FillEmptyLines=0,FillForce=0,IndentBlocks=0,"
//     "IndentBrackets=0,IndentCases=0,IndentClasses=1,IndentLabels=1,"
//     "IndentNamespaces=1,IndentPreprocessors=0,IndentSwitches=1,"
//     "KeepBlocks=1,KeepStatements=1,MaxStatement=40,"
//     "MinConditional=-1,PadOperators=0,PadParenthesesIn=1,"
//     "PadParenthesesOut=1,PadParenthesesUn=1,");

void AStyleFormatter::loadStyle(const QString &content)
{
//     QStringList pairs = options.split(',', QString::SkipEmptyParts );
//     QStringList::Iterator it;
//     for ( it = pairs.begin(); it != pairs.end(); ++it ) {
//         QStringList bits = (*it).split('=');
//         m_options[bits[0]] = bits[1];
//     }
    m_options = KDevelop::ISourceFormatter::stringToOptionMap(content);
    updateFormatter();
}

QString AStyleFormatter::saveStyle()
{
    return KDevelop::ISourceFormatter::optionMapToString(m_options);
//     QMap<QString, QVariant>::const_iterator it = m_options.constBegin();
//     for(; it != m_options.constEnd(); it++) {
//         options += it.key();
//         options += '=';
//         options += it.value().toString();
//         options += ',';
//     }

//     KConfigGroup group = config->group("AStyle");
//     group.writeEntry(name, options);
//     group.writeEntry("Extensions", m_extensions.join(","));
//     group.sync();
//     kDebug() << "Saving config to" << name << " : "<< options << endl;
}

void AStyleFormatter::setTabIndentation(int length, bool forceTabs)
{
    ASFormatter::setTabIndentation(length, forceTabs);
    m_options["Fill"] = "Tabs";
    m_options["FillForce"] = forceTabs;
    m_options["FillCount"] = length;
}

void AStyleFormatter::setSpaceIndentation(int length)
{
    ASFormatter::setSpaceIndentation(length);
    m_options["Fill"] = "Spaces";
    m_options["FillCount"] = length;
}

void AStyleFormatter::setTabSpaceConversionMode(bool mode)
{
    m_options["FillForce"] = mode;
    ASFormatter::setTabSpaceConversionMode(mode);
}

void AStyleFormatter::setFillEmptyLines(bool on)
{
    m_options["FillEmptyLines"] = on;
    ASFormatter::setEmptyLineFill(on);
}

void AStyleFormatter::setBlockIndent(bool on)
{
    m_options["IndentBlocks"] = on;
    ASFormatter::setBlockIndent(on);
}

void AStyleFormatter::setBracketIndent(bool on)
{
    m_options["IndentBrackets"] = on;
    ASFormatter::setBracketIndent(on);
}

void AStyleFormatter::setCaseIndent(bool on)
{
    m_options["IndentCases"] = on;
    ASFormatter::setCaseIndent(on);
}

void AStyleFormatter::setClassIndent(bool on)
{
    m_options["IndentClasses"] = on;
    ASFormatter::setClassIndent(on);
}

void AStyleFormatter::setLabelIndent(bool on)
{
    m_options["IndentLabels"] = on;
    ASFormatter::setLabelIndent(on);
}

void AStyleFormatter::setNamespaceIndent(bool on)
{
    m_options["IndentNamespaces"] = on;
    ASFormatter::setNamespaceIndent(on);
}

void AStyleFormatter::setPreprocessorIndent(bool on)
{
    m_options["IndentPreprocessors"] = on;
    ASFormatter::setPreprocessorIndent(on);
}

void AStyleFormatter::setSwitchIndent(bool on)
{
    m_options["IndentSwitches"] = on;
    ASFormatter::setSwitchIndent(on);
}

void AStyleFormatter::setMaxInStatementIndentLength(int max)
{
    m_options["MaxStatement"] = max;
    ASFormatter::setMaxInStatementIndentLength(max);
}

void AStyleFormatter::setMinConditionalIndentLength(int min)
{
    m_options["MinConditional"] = min;
    ASFormatter::setMinConditionalIndentOption(min);
    ASFormatter::setMinConditionalIndentLength();
}

void AStyleFormatter::setBracketFormatMode(astyle::BracketMode mode)
{
    switch (mode) {
    case astyle::NONE_MODE:
        m_options["Brackets"] = "";
        break;
    case astyle::ATTACH_MODE:
        m_options["Brackets"] = "Attach";
        break;
    case astyle::BREAK_MODE:
        m_options["Brackets"] = "Break";
        break;
    case astyle::BDAC_MODE:
        m_options["Brackets"] = "Linux";
        break;
    case astyle::STROUSTRUP_MODE:
        m_options["Brackets"] = "Stroustrup";
        break;
    case astyle::HORSTMANN_MODE:
        m_options["Brackets"] = "Horstmann";
        break;
    }
    ASFormatter::setBracketFormatMode(mode);
}

void AStyleFormatter::setBreakClosingHeaderBracketsMode(bool state)
{
    m_options["BracketsCloseHeaders"] = state;
    ASFormatter::setBreakClosingHeaderBracketsMode(state);
}

void AStyleFormatter::setBreakBlocksMode(bool state)
{
    m_options["BlockBreak"] = state;
    ASFormatter::setBreakBlocksMode(state);
}

void AStyleFormatter::setBreakElseIfsMode(bool state)
{
    m_options["BlockIfElse"] = state;
    ASFormatter::setBreakElseIfsMode(state);
}

void AStyleFormatter::setBreakClosingHeaderBlocksMode(bool state)
{
    m_options["BlockBreakAll"] = state;
    ASFormatter::setBreakClosingHeaderBlocksMode(state);
}

void AStyleFormatter::setOperatorPaddingMode(bool mode)
{
    m_options["PadOperators"] = mode;
    ASFormatter::setOperatorPaddingMode(mode);
}

void AStyleFormatter::setParensOutsidePaddingMode(bool mode)
{
    m_options["PadParenthesesOut"] = mode;
    ASFormatter::setParensOutsidePaddingMode(mode);
}

void AStyleFormatter::setParensInsidePaddingMode(bool mode)
{
    m_options["PadParenthesesIn"] = mode;
    ASFormatter::setParensInsidePaddingMode(mode);
}

void AStyleFormatter::setParensUnPaddingMode(bool state)
{
    m_options["PadParenthesesUn"] = state;
    ASFormatter::setParensUnPaddingMode(state);
}

void AStyleFormatter::setBreakOneLineBlocksMode(bool state)
{
    m_options["KeepBlocks"] = !state;
    ASFormatter::setBreakOneLineBlocksMode(state);
}

void AStyleFormatter::setSingleStatementsMode(bool state)
{
    m_options["KeepStatements"] = !state;
    ASFormatter::setSingleStatementsMode(state);
}

void AStyleFormatter::setPointerAlignment(astyle::PointerAlign alignment)
{
    switch (alignment) {
        case astyle::ALIGN_NONE:
            m_options["PointerAlign"] = "None";
            break;
        case astyle::ALIGN_NAME:
            m_options["PointerAlign"] = "Name";
            break;
        case astyle::ALIGN_MIDDLE:
            m_options["PointerAlign"] = "Middle";
            break;
        case astyle::ALIGN_TYPE:
            m_options["PointerAlign"] = "Type";
            break;
    }
    ASFormatter::setPointerAlignment(alignment);
}
