/*
    SPDX-FileCopyrightText: 2008 CÃ ©dric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASTYLEFORMATTER_H
#define ASTYLEFORMATTER_H

#include <QVariant>
#include <QString>

#include "astyle.h"

namespace AStyleOptionKey {
QString forceTabs();
QString tabSpaceConversion();
QString fillEmptyLines();
QString bracesAdd();
}

class AStyleFormatter : public astyle::ASFormatter
{
public:
    /** Creates an empty AStyleFormatter with C style by default.
    */
    AStyleFormatter();

    QString formatSource(const QString& text, const QString& leftContext = QString(), const QString& rightContext = QString());

    QVariant option(const QString &name) const;

    bool predefinedStyle(const QString &name);
    void loadStyle(const QString &content);
    QString saveStyle() const;

    // fill
    /// Indents using one tab per indentation and disables tab-space conversion.
    void setTabIndentation(int length, bool forceTabs);
    /// Indents using {@p length per indentation} spaces and sets tab-space conversion to @p tabSpaceConversion.
    void setSpaceIndentationAndTabSpaceConversion(int length, bool tabSpaceConversion);
    /// Indents using {@p length per indentation} spaces and disables tab-space conversion.
    void setSpaceIndentationNoConversion(int length);
    void setEmptyLineFill(bool on);
    // indent+continuation
    void setBlockIndent(bool on);
    void setBracketIndent(bool on);
    void setCaseIndent(bool on);
    void setClassIndent(bool on);
    void setLabelIndent(bool on);
    void setNamespaceIndent(bool on);
    void setPreprocessorIndent(bool on);
    void setSwitchIndent(bool on);
    void setMaxInStatementIndentLength(int max);
    void setMinConditionalIndentLength(int min);
    void setAfterParens(bool on);
    void setContinuation(int n);
    //brackets
    void setBracketFormatMode(astyle::BraceMode mode);
    void setBreakClosingHeaderBracketsMode(bool state);
    void setAddBracesMode(bool state);
    //blocks
    void setBreakBlocksMode(bool state);
    void setBreakElseIfsMode(bool state);
    void setBreakClosingHeaderBlocksMode(bool state);
    //padding
    void setOperatorPaddingMode(bool mode);
    void setParensOutsidePaddingMode(bool mode);
    void setParensInsidePaddingMode(bool mode);
    void setParensHeaderPaddingMode(bool mode);
    void setParensUnPaddingMode(bool state);
    //oneliners
    void setBreakOneLineBlocksMode(bool state);
    void setBreakOneLineStatementsMode(bool state);
    //pointer
    void setPointerAlignment(astyle::PointerAlign alignment);

private:
    void updateFormatter();
    void resetStyle();

    QVariantMap m_options;
};

#endif // ASTYLEFORMATTER_H
