/*
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "editorconfigpage.h"

#include <QIcon>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <KTextEditor/Editor>

using namespace KDevelop;

namespace {

class KTextEditorConfigPageAdapter : public ConfigPage
{
    Q_OBJECT

public:
    explicit KTextEditorConfigPageAdapter(KTextEditor::ConfigPage* page, QWidget* parent = nullptr)
        : ConfigPage(nullptr, nullptr, parent), m_page(page)
    {
        page->setParent(this);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(page);
        setLayout(layout);

        connect(page, &KTextEditor::ConfigPage::changed,
                this, &ConfigPage::changed);
    }

    ~KTextEditorConfigPageAdapter() override {}

    QString name() const override
    {
        return m_page->name();
    }

    QIcon icon() const override
    {
        return m_page->icon();
    }

    QString fullName() const override
    {
        return m_page->fullName();
    }

public Q_SLOTS:
    void apply() override
    {
        m_page->apply();
    }
    void defaults() override
    {
        m_page->defaults();
    }
    void reset() override
    {
        m_page->reset();
    }

protected:
    bool needsResetDuringInitialization() const override
    {
        // Optimization: KateThemeConfigPage::reset() is very slow (takes about 600 ms);
        // neither KTextEditor nor Kate calls reset() after creating a KTextEditor::ConfigPage.
        return false;
    }

private:
    KTextEditor::ConfigPage* const m_page;
};

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

#include "editorconfigpage.moc"
