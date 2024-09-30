/*
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "editorconfigpage.h"

#include <QIcon>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <QVBoxLayout>

#include <KTextEditor/Editor>

using namespace KDevelop;

KTextEditorConfigPageAdapter::KTextEditorConfigPageAdapter(KTextEditor::ConfigPage* page, QWidget* parent)
    : ConfigPage(nullptr, nullptr, parent)
    , m_page(page)
{
    page->setParent(this);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(page);
    setLayout(layout);

    connect(page, &KTextEditor::ConfigPage::changed, this, &ConfigPage::changed);
}

EditorConfigPage::EditorConfigPage(QWidget* parent)
    : ConfigPage(nullptr, nullptr, parent)
{
    setObjectName(QStringLiteral("editorconfig"));
}

EditorConfigPage::~EditorConfigPage() {}

QString EditorConfigPage::name() const
{
    return i18nc("@title:tab", "Editor");
}

QIcon EditorConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("accessories-text-editor"));
}

QString EditorConfigPage::fullName() const
{
    return i18n("Configure Text Editor");
}

int EditorConfigPage::childPages() const
{
    return KTextEditor::Editor::instance()->configPages();
}

ConfigPage* EditorConfigPage::childPage(int number)
{
    auto page = KTextEditor::Editor::instance()->configPage(number, this);
    if (page) {
        return new KTextEditorConfigPageAdapter(page, this);
    }
    return nullptr;
}

#include "moc_editorconfigpage.cpp"
