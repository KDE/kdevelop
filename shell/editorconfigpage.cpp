/*
 * This file is part of KDevelop
 * Copyright 2014 Alex Richardson <arichardson.kde@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
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
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(page);
        setLayout(layout);
    }

    virtual ~KTextEditorConfigPageAdapter() {}

    virtual QString name() const override
    {
        return m_page->name();
    }

    virtual QIcon icon() const override
    {
        return m_page->icon();
    }

    virtual QString fullName() const override
    {
        return m_page->fullName();
    }

public Q_SLOTS:
    virtual void apply() override
    {
        m_page->apply();
    }
    virtual void defaults() override
    {
        m_page->defaults();
    }
    virtual void reset() override
    {
        m_page->reset();
    }

private:
    KTextEditor::ConfigPage* m_page;
};

}

EditorConfigPage::EditorConfigPage(QWidget* parent)
    : ConfigPage(nullptr, nullptr, parent)
{
    setObjectName("editorconfig");
}

EditorConfigPage::~EditorConfigPage() {};

QString EditorConfigPage::name() const
{
    return i18n("Editor");
}

QIcon EditorConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("accessories-text-editor"));
}

QString EditorConfigPage::fullName() const
{
    return i18n("Configure Text editor");
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
