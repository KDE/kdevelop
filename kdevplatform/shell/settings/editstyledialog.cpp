/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "editstyledialog.h"

#include <interfaces/isourceformatter.h>

#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/Editor>
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
    Q_ASSERT_X(formatter.hasEditStyleWidget(), Q_FUNC_INFO, "Precondition");

    auto* const mainWidget = new QWidget();
    m_ui.setupUi(mainWidget);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mainWidget);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    auto okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &EditStyleDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &EditStyleDialog::reject);
    mainLayout->addWidget(buttonBox);

    init();

    m_settingsWidget->load(style);
}

EditStyleDialog::~EditStyleDialog()
{
}

void EditStyleDialog::init()
{
    // add plugin settings widget
    auto* layout = new QVBoxLayout(m_ui.settingsWidgetParent);
    layout->setContentsMargins(0, 0, 0, 0);

    m_settingsWidget = m_sourceFormatter.editStyleWidget(m_mimeType).release();
    Q_ASSERT_X(m_settingsWidget, Q_FUNC_INFO, "hasEditStyleWidget() is inconsistent with editStyleWidget().");
    layout->addWidget(m_settingsWidget);
    Q_ASSERT_X(m_settingsWidget->parent(), Q_FUNC_INFO, "QBoxLayout::addWidget must reparent its argument.");

    m_ui.settingsWidgetParent->setLayout(layout);

    connect(m_settingsWidget, &SettingsWidget::previewTextChanged,
            this, &EditStyleDialog::updatePreviewText);

    m_ui.showPreviewCheckBox->setChecked(m_style.usePreview());
    connect(m_ui.showPreviewCheckBox, &QCheckBox::toggled, this, [this](bool show) {
        m_style.setUsePreview(show);
        previewVisibilityChanged(show);
    });

    previewVisibilityChanged(m_style.usePreview());
}

void EditStyleDialog::initPreview()
{
    m_document = KTextEditor::Editor::instance()->createDocument(this);
    m_document->setReadWrite(false);
    m_document->setHighlightingMode(m_style.modeForMimetype(m_mimeType));

    m_view = m_document->createView(m_ui.textEditor);
    auto* layout2 = new QVBoxLayout(m_ui.textEditor);
    layout2->setContentsMargins(0, 0, 0, 0);
    layout2->addWidget(m_view);
    m_ui.textEditor->setLayout(layout2);

    m_view->setStatusBarEnabled(false);
    m_view->setConfigValue(QStringLiteral("dynamic-word-wrap"), false);
    m_view->setConfigValue(QStringLiteral("icon-bar"), false);
    m_view->setConfigValue(QStringLiteral("scrollbar-minimap"), false);
    m_view->show();
}

void EditStyleDialog::updatePreviewText(const QString &text)
{
    Q_ASSERT_X(!text.isEmpty(), Q_FUNC_INFO, "Empty m_pendingPreviewText has a special meaning.");
    if (m_style.usePreview()) {
        showPreview(text);
    } else {
        m_pendingPreviewText = text;
    }
}

void EditStyleDialog::showPreview(const QString& text)
{
    m_document->setReadWrite(true);
    m_style.setContent(content());
    m_document->setText(m_sourceFormatter.formatSourceWithStyle(m_style, text, QUrl(), m_mimeType));

    m_view->setCursorPosition(KTextEditor::Cursor(0, 0));
    m_document->setReadWrite(false);
}

void EditStyleDialog::previewVisibilityChanged(bool visible)
{
    Q_ASSERT(m_style.usePreview() == visible);
    if (visible) {
        if (!m_document) {
            initPreview();
        }
        if (!m_pendingPreviewText.isEmpty()) {
            showPreview(m_pendingPreviewText);
            m_pendingPreviewText = QString{};
        }
    }
    m_ui.previewArea->setVisible(visible);
}

QString EditStyleDialog::content() const
{
    return m_settingsWidget->save();
}

bool EditStyleDialog::usePreview() const
{
    return m_style.usePreview();
}

#include "moc_editstyledialog.cpp"
