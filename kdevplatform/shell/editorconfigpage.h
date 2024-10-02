/*
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KDEVPLATFORM_EDITORCONFIGPAGE_H
#define KDEVPLATFORM_EDITORCONFIGPAGE_H

#include <interfaces/configpage.h>

namespace KDevelop {

/**
 * This class makes all the KTextEditor config pages available for use
 * as a single KDevelop::ConfigPage
 */
class EditorConfigPage : public ConfigPage
{
    Q_OBJECT
public:
    explicit EditorConfigPage(QWidget* parent);
    ~EditorConfigPage() override;

    QString name() const override;
    QIcon icon() const override;
    QString fullName() const override;
    int childPages() const override;
    ConfigPage* childPage(int number) override;
public Q_SLOTS:
    // nothing to edit on this page, only subpages have actual configuration
    void apply() override {};
    void reset() override {};
    void defaults() override {};
};

class KTextEditorConfigPageAdapter : public ConfigPage
{
    Q_OBJECT

public:
    explicit KTextEditorConfigPageAdapter(KTextEditor::ConfigPage* page, IPlugin* plugin, QWidget* parent);

    ~KTextEditorConfigPageAdapter() override = default;

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

#endif // KDEVPLATFORM_EDITORCONFIGPAGE_H
