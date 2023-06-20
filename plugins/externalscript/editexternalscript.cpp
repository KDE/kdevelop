/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "editexternalscript.h"

#include "externalscriptitem.h"

#include <QAction>
#include <QDialogButtonBox>
#include <QPushButton>

#include <KLocalizedString>
#include <KShell>

EditExternalScript::EditExternalScript(ExternalScriptItem* item, QWidget* parent)
    : QDialog(parent)
    , m_item(item)
{
    setupUi(this);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &EditExternalScript::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &EditExternalScript::reject);
    shortcutWidget->layout()->setContentsMargins(0, 0, 0, 0);

    //BEGIN setup tooltips
    QString tooltip = i18nc("@info:tooltip",
        "<p>Defines the command that should be executed when this script is run. Basic shell features of your platform should be available.</p>\n"
        "<p>There are a few placeholders you can use in the command:</p>\n"
        "<dl>\n"
        "  <dt><code>%u</code></dt>\n"
        "  <dd>Gets replaced by the URL of the active document.</dd>\n"
        "  <dt><code>%f</code></dt>\n"
        "  <dd>Gets replaced by the local filepath to the active document.</dd>\n"
        "  <dt><code>%n</code></dt>\n"
        "  <dd>Gets replaced by the name of the active document, including its extension.</dd>\n"
        "  <dt><code>%b</code></dt>\n"
        "  <dd>Gets replaced by the name of the active document without its extension.</dd>\n"
        "  <dt><code>%d</code></dt>\n"
        "  <dd>Gets replaced by the path to the directory of the active document.</dd>\n"
        "  <dt><code>%p</code></dt>\n"
        "  <dd>Gets replaced by the URL to the project of the active document.</dd>\n"
        "  <dt><code>%s</code></dt>\n"
        "  <dd>Gets replaced with the shell escaped contents of the selection in the active document.</dd>\n"
        "  <dt><code>%i</code></dt>\n"
        "  <dd>Gets replaced with the PID of the currently running KDevelop process.</dd>\n"
        "</dl>\n"
        "<p><b>NOTE:</b>  It is your responsibility to prevent running hazardous commands that could lead to data loss.</p>\n"
                      );
    commandEdit->setToolTip(tooltip);
    commandLabel->setToolTip(tooltip);

    tooltip = i18nc("@info:tooltip",
        "<p>Defines what the external script should get as input (via <code>STDIN</code>).</p>"
              );
    stdinCombo->setToolTip(tooltip);
    stdinLabel->setToolTip(tooltip);

    tooltip = i18nc("@info:tooltip",
        "<p>Defines what should be done with the output (i.e. <code>STDOUT</code>) of the script.</p>"
              );
    stdoutCombo->setToolTip(tooltip);
    stdoutLabel->setToolTip(tooltip);

    tooltip = i18nc("@info:tooltip",
        "<p>Defines what should be done with the errors (i.e. <code>STDERR</code>) of the script.</p>"
        "<p>Note: if the action is the same as that chosen for the output, the channels will be merged "
        "and handled together.</p>"
              );
    stderrCombo->setToolTip(tooltip);
    stderrLabel->setToolTip(tooltip);

    tooltip = i18nc("@info:tooltip",
        "<p>Defines the name of the script. Just for displaying purposes.</p>"
              );
    nameEdit->setToolTip(tooltip);
    nameLabel->setToolTip(tooltip);

    tooltip = i18nc("@info:tooltip",
        "<p>Defines the shortcut(s) you can use to execute this external script.</p>"
              );
    shortcutLabel->setToolTip(tooltip);
    shortcutWidget->setToolTip(tooltip);

    tooltip = i18nc("@info:tooltip",
        "<p>Defines whether documents should be saved before the script gets executed.</p>"
              );
    saveLabel->setToolTip(tooltip);
    saveCombo->setToolTip(tooltip);

    tooltip = i18nc("@info:tooltip",
        "<p>Defines whether the output of the script should be shown in a tool view.</p>"
              );
    showOutputBox->setToolTip(tooltip);
    tooltip = i18nc("@info:tooltip",
        "<p>Defines what type of filtering should be applied to the output. E.g. to indicate errors by red text.</p>"
              );
    outputFilterLabel->setToolTip(tooltip);
    outputFilterCombo->setToolTip(tooltip);
    //END setup tooltips

    //BEGIN item to UI copying
    if (item->text().isEmpty()) {
        setWindowTitle(i18nc("@title:window", "Create New External Script"));
    } else {
        setWindowTitle(i18nc("@title:window", "Edit External Script '%1'", item->text()));
    }
    nameEdit->setText(item->text());
    commandEdit->setText(item->command());
    stdinCombo->setCurrentIndex(item->inputMode());
    stdoutCombo->setCurrentIndex(item->outputMode());
    stderrCombo->setCurrentIndex(item->errorMode());
    saveCombo->setCurrentIndex(item->saveMode());
    shortcutWidget->setShortcut(item->action()->shortcuts());
    showOutputBox->setChecked(item->showOutput());
    outputFilterCombo->setCurrentIndex(item->filterMode());
    //END item to UI copying

    validate();

    nameEdit->setFocus();

    connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &EditExternalScript::save);
    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &EditExternalScript::save);

    connect(nameEdit, &QLineEdit::textEdited, this, &EditExternalScript::validate);
    connect(commandEdit, &QLineEdit::textEdited, this, &EditExternalScript::validate);
}

EditExternalScript::~EditExternalScript()
{
}

void EditExternalScript::save()
{
    m_item->setText(nameEdit->text());
    m_item->setCommand(commandEdit->text());

    auto inputMode = static_cast<ExternalScriptItem::InputMode>(stdinCombo->currentIndex());
    m_item->setInputMode(inputMode);

    auto outputMode = static_cast<ExternalScriptItem::OutputMode>(stdoutCombo->currentIndex());
    m_item->setOutputMode(outputMode);

    auto errorMode = static_cast<ExternalScriptItem::ErrorMode>(stderrCombo->currentIndex());
    m_item->setErrorMode(errorMode);

    auto saveMode = static_cast<ExternalScriptItem::SaveMode>(saveCombo->currentIndex());
    m_item->setSaveMode(saveMode);

    m_item->setShowOutput(showOutputBox->isChecked());

    m_item->setFilterMode(outputFilterCombo->currentIndex());
    m_item->action()->setShortcuts(shortcutWidget->shortcut());
}

void EditExternalScript::validate()
{
    bool valid = !nameEdit->text().isEmpty() && !commandEdit->text().isEmpty();
    if (valid) {
        KShell::Errors errors = KShell::NoError;
        KShell::splitArgs(commandEdit->text(), KShell::TildeExpand, &errors);
        valid = errors == KShell::NoError;
    }

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(valid);
}

#include "moc_editexternalscript.cpp"
