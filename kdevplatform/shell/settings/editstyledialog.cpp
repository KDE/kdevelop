/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "editstyledialog.h"

#include <interfaces/isourceformatter.h>

#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/Editor>
#include <KTextEditor/ConfigInterface>
#include <KLocalizedString>

#include <QPushButton>
#include <QVBoxLayout>
#include <QUrl>
#include <QDialogButtonBox>


using namespace KDevelop;

EditStyleDialog::EditStyleDialog(const ISourceFormatter& formatter, const QMimeType& mime,
                                 const SourceFormatterStyle& style, QWidget* parent)
    : QDialog(parent)
    , m_sourceFormatter(formatter)
    , m_mimeType(mime)
    , m_style(style)
{
    m_content = new QWidget();
    m_ui.setupUi(m_content);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_content);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    auto okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &EditStyleDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &EditStyleDialog::reject);
    mainLayout->addWidget(buttonBox);

    m_settingsWidget = m_sourceFormatter.editStyleWidget(m_mimeType).release();
    init();

    if (m_settingsWidget) {
        m_settingsWidget->load(style);
    }
}

EditStyleDialog::~EditStyleDialog()
{
}

void EditStyleDialog::init()
{
    // add plugin settings widget
    if (m_settingsWidget) {
        auto* layout = new QVBoxLayout(m_ui.settingsWidgetParent);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_settingsWidget);
        m_ui.settingsWidgetParent->setLayout(layout);
        connect(m_settingsWidget, &SettingsWidget::previewTextChanged,
                this, &EditStyleDialog::updatePreviewText);
    }

    m_document = KTextEditor::Editor::instance()->createDocument(this);
    m_document->setReadWrite(false);
    m_document->setHighlightingMode(m_style.modeForMimetype(m_mimeType));

    m_view = m_document->createView(m_ui.textEditor);
    auto* layout2 = new QVBoxLayout(m_ui.textEditor);
    layout2->setContentsMargins(0, 0, 0, 0);
    layout2->addWidget(m_view);
    m_ui.textEditor->setLayout(layout2);
    m_view->setStatusBarEnabled(false);
    m_view->show();

    KTextEditor::ConfigInterface* iface =
        qobject_cast<KTextEditor::ConfigInterface*>(m_view);
    if (iface) {
        iface->setConfigValue(QStringLiteral("dynamic-word-wrap"), false);
        iface->setConfigValue(QStringLiteral("icon-bar"), false);
        iface->setConfigValue(QStringLiteral("scrollbar-minimap"), false);
    }

    updatePreviewText(m_sourceFormatter.previewText(m_style, m_mimeType));
}

void EditStyleDialog::updatePreviewText(const QString &text)
{
    m_document->setReadWrite(true);
    m_style.setContent(content());
    m_document->setText(m_sourceFormatter.formatSourceWithStyle(m_style, text, QUrl(), m_mimeType));

    m_view->setCursorPosition(KTextEditor::Cursor(0, 0));
    m_document->setReadWrite(false);
}

QString EditStyleDialog::content()
{
    if (m_settingsWidget) {
        return m_settingsWidget->save();
    }
    return QString();
}

#include "moc_editstyledialog.cpp"
