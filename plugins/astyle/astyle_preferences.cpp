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
#include "astyle_preferences.h"

#include <KIcon>

#include "astyle_formatter.h"
#include "astyle_plugin.h"

#define STYLE_CUSTOM 0
#define STYLE_ANSI 1
#define STYLE_GNU 2
#define STYLE_JAVA 3
#define STYLE_KR 4
#define STYLE_LINUX 5

#define INDENT_BLOCK 0
#define INDENT_BRACKET 1
#define INDENT_CASE 2
#define INDENT_CLASS 3
#define INDENT_LABEL 4
#define INDENT_NAMESPACE 5
#define INDENT_PREPROCESSOR 6
#define INDENT_SWITCH 7

#define PADDING_NOCHANGE 0
#define PADDING_NO 1
#define PADDING_IN 2
#define PADDING_OUT 3
#define PADDING_INOUT 4

#define INDENT_TABS 0
#define INDENT_TABSFORCE 1
#define INDENT_SPACES 2

#define BRACKET_NOCHANGE 0
#define BRACKET_ATTACH 1
#define BRACKET_BREAK 2
#define BRACKET_LINUX 3

AStylePreferences::AStylePreferences(Language lang, QWidget *parent)
    : SettingsWidget(parent), m_lang(lang)
{
    setupUi(this);
    m_formatter = new AStyleFormatter();
//     m_enableWidgetSignals = true;
    init();
}

AStylePreferences::~AStylePreferences( )
{
}

void AStylePreferences::init()
{
    // setup list widget to have checked items
    for(int i = 0; i < listIdentObjects->count(); i++) {
        QListWidgetItem *item = listIdentObjects->item(i);
        item->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
        item->setCheckState(Qt::Checked);
    }
    
    connect(tabWidget, SIGNAL(currentChanged(QWidget*)), this, SLOT(currentTabChanged()));
    
    connect(cbIndentType, SIGNAL(currentIndexChanged(int)), this, SLOT(indentChanged()));
    connect(inpNuberSpaces, SIGNAL(valueChanged(int)), this, SLOT(indentChanged()));
    connect(chkConvertTabs, SIGNAL(stateChanged(int)), this, SLOT(indentChanged()));
    connect(chkFillEmptyLines, SIGNAL(stateChanged(int)), this, SLOT(indentChanged()));
    
    connect(listIdentObjects, SIGNAL(itemChanged(QListWidgetItem*)), 
             this, SLOT(indentObjectsChanged(QListWidgetItem*)));
             
    connect(inpMaxStatement, SIGNAL(valueChanged(int)), this, SLOT(minMaxValuesChanged()));
    connect(inpMinConditional, SIGNAL(valueChanged(int)), this, SLOT(minMaxValuesChanged()));
    
    connect(cbBrackets, SIGNAL(currentIndexChanged(int)), this, SLOT(bracketsChanged()));
    connect(chkBracketsCloseHeaders, SIGNAL(stateChanged(int)), this, SLOT(bracketsChanged()));
    
    connect(chkBlockBreak, SIGNAL(stateChanged(int)), this, SLOT(blocksChanged()));
    connect(chkBlockBreakAll, SIGNAL(stateChanged(int)), this, SLOT(blocksChanged()));
    connect(chkBlockIfElse, SIGNAL(stateChanged(int)), this, SLOT(blocksChanged()));
    
    connect(cbParenthesisPadding, SIGNAL(currentIndexChanged(int)), this, SLOT(paddingChanged()));
    connect(chkPadOperators, SIGNAL(stateChanged(int)), this, SLOT(paddingChanged()));
    
    connect(chkKeepStatements, SIGNAL(stateChanged(int)), this, SLOT(onelinersChanged()));
    connect(chkKeepBlocks, SIGNAL(stateChanged(int)), this, SLOT(onelinersChanged()));
}

void AStylePreferences::load(const QString &name, const QString &content)
{
    if(name.isEmpty())
        m_formatter->loadStyle(content);
    else
        m_formatter->predefinedStyle(name);

    updateWidgets();
}

QString AStylePreferences::save()
{
    return m_formatter->saveStyle();
}

void AStylePreferences::updateWidgets()
{   
    //indent
    if(m_formatter->option("Fill").toString() == "Tabs") {
        if(m_formatter->option("FillForce").toBool())
            cbIndentType->setCurrentIndex(INDENT_TABSFORCE);
        else
            cbIndentType->setCurrentIndex(INDENT_TABS);
        inpNuberSpaces->setValue(m_formatter->option("FillCount").toInt());
    } else {
        cbIndentType->setCurrentIndex(INDENT_SPACES);
        inpNuberSpaces->setValue(m_formatter->option("FillCount").toInt());
    }
    chkFillEmptyLines->setChecked(m_formatter->option("FillEmptyLines").toBool());
    chkConvertTabs->setChecked(m_formatter->option("FillForce").toBool());

    // indent objects
    setItemChecked(INDENT_BLOCK, m_formatter->option("IndentBlocks").toBool());
    setItemChecked(INDENT_BRACKET, m_formatter->option("IndentBrackets").toBool());
    setItemChecked(INDENT_CASE, m_formatter->option("IndentCases").toBool());
    setItemChecked(INDENT_CLASS, m_formatter->option("IndentClasses").toBool());
    setItemChecked(INDENT_LABEL, m_formatter->option("IndentLabels").toBool());
    setItemChecked(INDENT_NAMESPACE, m_formatter->option("IndentNamespaces").toBool());
    setItemChecked(INDENT_PREPROCESSOR, m_formatter->option("IndentClasses").toBool());
    setItemChecked(INDENT_SWITCH, m_formatter->option("IndentSwitches").toBool());
    
    inpMaxStatement->setValue(m_formatter->option("MaxStatement").toInt());
    inpMinConditional->setValue(m_formatter->option("MinConditional").toInt());

    // brackets
    QString s = m_formatter->option("Brackets").toString();
    if(s == "Attach")
        cbBrackets->setCurrentIndex(BRACKET_ATTACH);
    else if(s == "Break")
        cbBrackets->setCurrentIndex(BRACKET_BREAK);
    else if(s == "Linux")
        cbBrackets->setCurrentIndex(BRACKET_LINUX);
    else
        cbBrackets->setCurrentIndex(BRACKET_NOCHANGE);
    chkBracketsCloseHeaders->setChecked(
        m_formatter->option("BracketsCloseHeaders").toBool());

    // blocks
    chkBlockBreak->setChecked(m_formatter->option("BlockBreak").toBool());
    chkBlockBreakAll->setChecked(m_formatter->option("BlockBreakAll").toBool());
    chkBlockIfElse->setChecked(m_formatter->option("BlockIfElse").toBool());
    // enable or not chkBlockBreakAll
    chkBlockBreakAll->setEnabled(chkBlockBreak->isChecked());
    
    // padding
    bool padin = m_formatter->option("PadParenthesesIn").toBool();
    bool padout = m_formatter->option("PadParenthesesOut").toBool();
    bool unpad = m_formatter->option("PadParenthesesUn").toBool();
    if(unpad) {
        if(padin) {
            if(padout)
                cbParenthesisPadding->setCurrentIndex(PADDING_INOUT);
            else
                cbParenthesisPadding->setCurrentIndex(PADDING_IN);
        } else if(padout)
            cbParenthesisPadding->setCurrentIndex(PADDING_OUT);
        else
            cbParenthesisPadding->setCurrentIndex(PADDING_NO);
    } else
        cbParenthesisPadding->setCurrentIndex(PADDING_NOCHANGE);
        
    chkPadOperators->setChecked(m_formatter->option("PadOperators").toBool());
    // oneliner
    chkKeepStatements->setChecked(m_formatter->option("KeepStatements").toBool());
    chkKeepBlocks->setChecked(m_formatter->option("KeepBlocks").toBool());
}

void AStylePreferences::setItemChecked(int idx, bool checked)
{
    QListWidgetItem *item = listIdentObjects->item(idx);
    if(!item)
        return;
    
    if(checked)
        item->setCheckState(Qt::Checked);
    else
        item->setCheckState(Qt::Unchecked);    
}

void AStylePreferences::updatePreviewText(bool emitChangedSignal)
{
//     if(!m_enableWidgetSignals)
//         return;

    QString text;
    int id = tabWidget->currentIndex();
    if(id == 0)
        text = AStylePlugin::indentingSample();
    else
        text = AStylePlugin::formattingSample();

    QString output = m_formatter->formatSource(text);
    emit previewTextChanged(output);
}

void AStylePreferences::currentTabChanged()
{
    updatePreviewText(false);
}

void AStylePreferences::indentChanged()
{
    switch(cbIndentType->currentIndex()) {
        case INDENT_TABS: 
            m_formatter->setTabIndentation(inpNuberSpaces->value(), false);
            break;
        case INDENT_TABSFORCE: 
            m_formatter->setTabIndentation(inpNuberSpaces->value(), true);
            break;
        case INDENT_SPACES: 
            m_formatter->setSpaceIndentation(inpNuberSpaces->value()); 
            break;
    }
    
    updatePreviewText();
}

void AStylePreferences::indentObjectsChanged(QListWidgetItem *item)
{
    if(!item) 
        return;
    
    bool checked = (item->checkState() == Qt::Checked);
    switch(listIdentObjects->row(item)) {
        case INDENT_BLOCK: m_formatter->setBlockIndent(checked); break;
        case INDENT_BRACKET: m_formatter->setBracketIndent(checked); break;
        case INDENT_CASE: m_formatter->setCaseIndent(checked); break;
        case INDENT_CLASS: m_formatter->setClassIndent(checked); break;
        case INDENT_LABEL: m_formatter->setLabelIndent(checked); break;
        case INDENT_NAMESPACE: m_formatter->setNamespaceIndent(checked); break;
        case INDENT_PREPROCESSOR: m_formatter->setPreprocessorIndent(checked); break;
        case INDENT_SWITCH: m_formatter->setSwitchIndent(checked); break;
    }
    
    updatePreviewText();
}

void AStylePreferences::minMaxValuesChanged()
{
    m_formatter->setMaxInStatementIndentLength(inpMaxStatement->value());
    m_formatter->setMinConditionalIndentLength(inpMinConditional->value());
    
    updatePreviewText();
}

void AStylePreferences::bracketsChanged()
{
    switch(cbBrackets->currentIndex()) {
        case BRACKET_NOCHANGE: m_formatter->setBracketFormatMode(astyle::NONE_MODE); break;
        case BRACKET_ATTACH: m_formatter->setBracketFormatMode(astyle::ATTACH_MODE); break;
        case BRACKET_BREAK: m_formatter->setBracketFormatMode(astyle::BREAK_MODE); break;
        case BRACKET_LINUX: m_formatter->setBracketFormatMode(astyle::BDAC_MODE); break;
    }
    
    m_formatter->setBreakClosingHeaderBracketsMode(chkBracketsCloseHeaders->isChecked());
    
    updatePreviewText();
}

void AStylePreferences::blocksChanged()
{
    m_formatter->setBreakBlocksMode(chkBlockBreak->isChecked());
    m_formatter->setBreakClosingHeaderBlocksMode(chkBlockBreakAll->isChecked());
    m_formatter->setBreakElseIfsMode(chkBlockIfElse->isChecked());
    
    chkBlockBreakAll->setEnabled(chkBlockBreak->isChecked());
    
    updatePreviewText();
}

void AStylePreferences::paddingChanged()
{
    switch(cbParenthesisPadding->currentIndex()) {
        case PADDING_NOCHANGE: 
            m_formatter->setParensUnPaddingMode(false); 
            m_formatter->setParensInsidePaddingMode(false);
            m_formatter->setParensOutsidePaddingMode(false);
            break;
        case PADDING_NO: 
            m_formatter->setParensUnPaddingMode(true); 
            m_formatter->setParensInsidePaddingMode(false);
            m_formatter->setParensOutsidePaddingMode(false);
            break;
        case PADDING_IN:
            m_formatter->setParensUnPaddingMode(true); 
            m_formatter->setParensInsidePaddingMode(true);
            m_formatter->setParensOutsidePaddingMode(false);
            break;
        case PADDING_OUT:
            m_formatter->setParensUnPaddingMode(true); 
            m_formatter->setParensInsidePaddingMode(false);
            m_formatter->setParensOutsidePaddingMode(true);
            break;
        case PADDING_INOUT:
            m_formatter->setParensUnPaddingMode(true); 
            m_formatter->setParensInsidePaddingMode(true);
            m_formatter->setParensOutsidePaddingMode(true);
            break;
    }
    
    m_formatter->setOperatorPaddingMode(chkPadOperators->isChecked());
    
    updatePreviewText();
}

void AStylePreferences::onelinersChanged()
{
    m_formatter->setSingleStatementsMode(!chkKeepStatements->isChecked());
    m_formatter->setBreakOneLineBlocksMode(!chkKeepBlocks->isChecked());
    
    updatePreviewText();
}

#include "astyle_preferences.moc"
