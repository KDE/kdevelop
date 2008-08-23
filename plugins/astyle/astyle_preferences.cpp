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

#include <QVBoxLayout>
#include <QString>
#include <QInputDialog>
#include <QListWidgetItem>
#include <KGenericFactory>
#include <KTextEdit>
#include <KMessageBox>
#include <KIcon>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/configinterface.h>

#include "astyle_plugin.h"
#include "astyle_formatter.h"

#define STYLE_ROLE (Qt::UserRole+1)

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

K_PLUGIN_FACTORY(AStylePreferencesFactory, registerPlugin<AStylePreferences>();)
K_EXPORT_PLUGIN(AStylePreferencesFactory("kcm_kdev_astyle_preferences"))

AStylePreferences::AStylePreferences(QWidget *parent, const QVariantList &args )
: KCModule( AStylePreferencesFactory::componentData(), parent, args )
{
    setupUi(this);
    m_formatter = new AStyleFormatter();
    m_enableWidgetSignals = true;
    init();
}

AStylePreferences::~AStylePreferences( )
{
}

void AStylePreferences::init()
{
    m_bracketSample = "namespace foospace {\n\tint Foo(){\n\tif "
            "(isBar)\n{\nbar();\n\treturn 1; } else \nreturn 0;}}\n\nvoid test(){"
            "\n\tif (isFoo){\n\tbar();\n} else\n{\n\tbar();\n}\n}\n";
    m_indentSample = "#define foobar(A)\\\n{Foo();Bar();}\n#define"
            "anotherFoo(B)\\\nreturn Bar()\n\nnamespace Bar\n{\nclass Foo"
            "\n{public:\nFoo();\nvirtual ~Foo();\n};\nswitch (foo)\n"
            "{\ncase 1:\na+=1;\nbreak;\ncase 2:\n{\na += 2;\n break;\n}"
            "\n}\nif (isFoo)\n{\nbar();\n}\nelse\n{\nanotherBar();\n}"
            "\nint foo()\n\twhile(isFoo)\n\t\t{\n\t\t\t...\n\t\t\tgoto error;"
            "\n\t\t....\n\t\terror:\n\t\t\t...\n\t\t}\n\t}\nfooArray[]"
            "={ red,\n\tgreen,\n\tdarkblue};\nfooFunction(barArg1,"
            "\n\tbarArg2,\n\tbarArg3);\n";
    m_formattingSample = "void func(){\n\tif(isFoo(a,b))\n\tbar(a,b);\nif(isFoo)"
            "\n\ta=bar((b-c)*a,*d--);\nif(  isFoo( a,b ) )\n\tbar(a, b);"
            "\nif (isFoo) {isFoo=false;cat << isFoo <<endl;}\nif(isFoo)DoBar();"
            "if (isFoo){\n\tbar();\n}\n\telse if(isBar())"
            "{\n\tannotherBar();\n}\n}\n";
    m_fullSample = "\t//Tabs & Brackets\nnamespace foo{\n" + m_bracketSample +
            "}\n\t// Indentation\n" + m_indentSample + "\t// Formatting\n" 
            + m_formattingSample;

    // setup list widget to have checked items
    for(int i = 0; i < listIdentObjects->count(); i++) {
        QListWidgetItem *item = listIdentObjects->item(i);
        item->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
        item->setCheckState(Qt::Checked);
    }
    
    // add texteditor preview
    KTextEditor::Editor *editor = KTextEditor::EditorChooser::editor();
    if (!editor) {
      KMessageBox::error(this, i18n("A KDE text-editor component could not be found;\n"
                                    "please check your KDE installation."));
    }

    m_document = editor->createDocument(this);
    m_document->setText(m_fullSample);
    m_document->setHighlightingMode("C++");
    
    m_view = qobject_cast<KTextEditor::View*>(m_document->createView(textEditor));
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_view);
    textEditor->setLayout(layout);
    m_view->show();
    
    KTextEditor::ConfigInterface *iface = qobject_cast<KTextEditor::ConfigInterface*>(m_view);
    if(iface) {
        iface->setConfigValue("dynamic-word-wrap", false);
        iface->setConfigValue("icon-bar", false);
    }
    
    //setup list of styles
    addItemInStyleList(i18n("Custom"), QString());
    addItemInStyleList("ANSI", "ANSI");
    addItemInStyleList("GNU", "GNU");
    addItemInStyleList("Java", "Java");
    addItemInStyleList("Kernighan & Ritchie", "K&R");
    addItemInStyleList("Linux", "Linux");
    /// \todo load user style
    
    // set buttons icons
    btnDelStyle->setIcon(KIcon("edit-delete"));
    btnSaveStyle->setIcon(KIcon("document-save"));
    
    connect(listStyles, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), 
             this, SLOT(currentStyleChanged(QListWidgetItem*,QListWidgetItem*)));
    connect(btnDelStyle, SIGNAL(clicked()), this, SLOT(deleteCurrentStyle()));
    connect(btnSaveStyle, SIGNAL(clicked()), this, SLOT(addStyle())); 
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

void AStylePreferences::addItemInStyleList(const QString &caption, const QString &name)
{
    QListWidgetItem *item = new QListWidgetItem(caption);
    item->setData(STYLE_ROLE, name);
    listStyles->addItem(item);
}

void AStylePreferences::load()
{   
    KConfigGroup g = KGlobal::config()->group("AStyle");
    
    //create list items for each saved theme
    QStringList keyList = g.keyList();
    QStringList::const_iterator it = keyList.constBegin();
    for(; it != keyList.constEnd(); ++it) {
        if((*it).startsWith("User")) { // style definition
            QString caption = g.readEntry("Caption" + (*it).mid(4));
            addItemInStyleList(caption, *it);
            kDebug() << "Adding item in list: user" << (*it).mid(4) 
                << "  " << caption << endl; 
        }
    }
    
    QString s = g.readEntry("Style");
    int id = 0;
    if(s.isEmpty()) id = STYLE_CUSTOM;
    else if(s == "ANSI") id = STYLE_ANSI;
    else if(s == "GNU") id = STYLE_GNU;
    else if(s == "Java") id = STYLE_JAVA;
    else if(s == "K&R") id = STYLE_KR;
    else if(s == "Linux") id = STYLE_LINUX;
    else { // saved theme "User" + number
        int number = s.mid(4).toInt();
        id = number + STYLE_LINUX;
    }
    
    if(id == STYLE_CUSTOM)
        m_formatter->loadConfig(KGlobal::config());
    else if(id > STYLE_LINUX)
        m_formatter->loadConfig(KGlobal::config(), s);
    else
        m_formatter->predefinedStyle(s);
//     m_formatter->option("FStyle").toString();
    updateWidgets();
    // style
    listStyles->setCurrentRow(id);

    if(id != 0) {
        btnDelStyle->setEnabled(false);
        btnSaveStyle->setEnabled(false);
    } else {
        btnDelStyle->setEnabled(false);
        btnSaveStyle->setEnabled(true);
    }
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
        } else
            cbParenthesisPadding->setCurrentIndex(PADDING_OUT);
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

void AStylePreferences::save()
{
    QListWidgetItem *item = listStyles->currentItem();
    KConfigGroup g = KGlobal::config()->group("AStyle");

    if(item && !item->data(STYLE_ROLE).toString().isEmpty() && !btnSaveStyle->isEnabled()) { // only save style name
        g.writeEntry("Style", item->data(STYLE_ROLE).toString());
        g.writeEntry("Custom", ""); // clear it
    } else { 
        g.writeEntry("Style", "");
        m_formatter->saveConfig(KGlobal::config());
    }
    g.sync();
}

void AStylePreferences::updatePreviewText(bool emitChangedSignal)
{
    if(!m_enableWidgetSignals)
        return;
    
    QString text;
    int id = tabWidget->currentIndex();
    if(id == 1)
        text = m_indentSample;
    else if(id == 2)
        text = m_formattingSample;
    else
        text = m_fullSample;
    
    m_document->setText(m_formatter->formatSource(text));
    
    if(emitChangedSignal) {
        // style was modified, so switch to custom style if using predefined style
        int idx = listStyles->currentRow();
        if(idx < STYLE_LINUX) // ie predefined style
            listStyles->setCurrentRow(STYLE_CUSTOM);
        else // custom style was changed, enable the save button
            btnSaveStyle->setEnabled(true);
        emit(changed(true));
    }
}

void AStylePreferences::currentStyleChanged(QListWidgetItem *current, QListWidgetItem *)
{
    int idx = listStyles->currentRow();
    if(!current)
        return;

    if(idx < STYLE_LINUX) {
        btnDelStyle->setEnabled(false);
        btnSaveStyle->setEnabled(idx == STYLE_CUSTOM);
        m_formatter->predefinedStyle(current->data(STYLE_ROLE).toString());
        if(idx != STYLE_CUSTOM) {
            m_enableWidgetSignals = false;
            updateWidgets();
            m_enableWidgetSignals = true;
        }
    }
    else {
        btnDelStyle->setEnabled(true);
        btnSaveStyle->setEnabled(false);
        m_formatter->loadConfig(KGlobal::config(),
            current->data(STYLE_ROLE).toString());
        //not necessary but avoid to reload ten times the editor's text
        m_enableWidgetSignals = false;
        updateWidgets();
        m_enableWidgetSignals = true;
    }

    m_document->setText(m_formatter->formatSource(m_fullSample));
    emit(changed(true));
}

void AStylePreferences::deleteCurrentStyle()
{
    int res = KMessageBox::questionYesNo( this, i18n("Are you sure you"
        " want to delete this style?", i18n("Delete style")) );
    if(res == KMessageBox::No)
        return;
    
    // remove from the list
    QListWidgetItem *item = listStyles->takeItem(listStyles->currentRow());
    if(!item)
        return;
    
    // remove from config file
    KConfigGroup g = KGlobal::config()->group("AStyle");
    QString name = item->data(STYLE_ROLE).toString();
    g.deleteEntry(name);
    g.deleteEntry("Caption" + name.mid(4));
    g.sync();
}

void AStylePreferences::addStyle()
{   
    KConfigGroup g = KGlobal::config()->group("AStyle");
    int currentRow = listStyles->currentRow();
    
    // check if we save an already existing style
    if(currentRow != STYLE_CUSTOM) {
        QListWidgetItem *item = listStyles->currentItem();
        if(item) {
            QString s = item->data(STYLE_ROLE).toString();
            m_formatter->saveConfig(KGlobal::config(), s);
        }
        btnSaveStyle->setEnabled(false);
        emit(changed(true));
        return;
    }
    
    // find available number
    int idx = 1;
    QString s = "User" + QString::number(idx);
    while(g.hasKey(s)) {
        ++idx;
        s = "User" + QString::number(idx);
    }
 
    // ask for caption
    bool ok;
    QString caption = QInputDialog::getText( this, 
        i18n("New style"), i18n("Please enter a name for the new style"),
        QLineEdit::Normal, i18n("Custom Style") + QString::number(idx),
        &ok);
    if(!ok) // dialog aborted
        return;
    
    // save in config file
    m_formatter->saveConfig(KGlobal::config(), s);
    g.writeEntry("Caption" + QString::number(idx), caption);
    //add item in list
    addItemInStyleList(caption, s);
    listStyles->setCurrentRow(listStyles->count() - 1);
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
