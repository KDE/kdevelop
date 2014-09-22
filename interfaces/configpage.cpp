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
#include "configpage.h"

#include <QIcon>
#include <QVBoxLayout>

#include <KTextEditor/Editor>

using namespace KDevelop;

ConfigPage::ConfigPage(KCoreConfigSkeleton* config, QWidget* parent)
        : KTextEditor::ConfigPage(parent), m_configSkeleton(config)
{
    if (m_configSkeleton) {
        m_configManager.reset(new KConfigDialogManager(parent, m_configSkeleton));
        connect(m_configManager.data(), &KConfigDialogManager::widgetModified, this, &ConfigPage::changed);
        // m_configManager->addWidget(this) must be called from the config dialog,
        // since the widget tree is not complete yet when calling this constructor
    }
}

ConfigPage::~ConfigPage()
{

}

void ConfigPage::apply()
{
    Q_ASSERT(m_configManager); // if null, this method must be overriden
    m_configManager->updateSettings();
    m_configManager->updateWidgets();
}

void ConfigPage::defaults()
{
    Q_ASSERT(m_configManager); // if null, this method must be overriden
    m_configManager->updateWidgetsDefault();
}

void ConfigPage::reset()
{
    Q_ASSERT(m_configManager); // if null, this method must be overriden
    m_configManager->updateWidgets();
}

void ConfigPage::initConfigManager()
{
    if (m_configManager) {
        m_configManager->addWidget(this);
        m_configManager->updateWidgets();
    }
}


QList<ConfigPage*> ConfigPage::childPages()
{
    return {};
}

KTextEditorConfigPageWrapper::KTextEditorConfigPageWrapper(KTextEditor::ConfigPage* page, QWidget* parent)
        : ConfigPage(nullptr, parent), m_page(page)
{
    page->setParent(this);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(page);
    this->setLayout(layout);
}

KTextEditorConfigPageWrapper::~KTextEditorConfigPageWrapper()
{
}

void KTextEditorConfigPageWrapper::apply()
{
    m_page->apply();
}

void KTextEditorConfigPageWrapper::defaults()
{
    m_page->defaults();
}

void KTextEditorConfigPageWrapper::reset()
{
    m_page->reset();
}

QString KTextEditorConfigPageWrapper::fullName() const
{
    return m_page->fullName();
}

QIcon KTextEditorConfigPageWrapper::icon() const
{
    return m_page->icon();
}

QString KTextEditorConfigPageWrapper::name() const
{
    return m_page->name();
}

EditorConfigPage::EditorConfigPage(QWidget* parent)
        : ConfigPage(nullptr, parent)
{
    qDebug("Editor config page created");
    setObjectName("editorconfig");
}

EditorConfigPage::~EditorConfigPage()
{
    qDebug("Editor config page deleted");
}

QString EditorConfigPage::name() const {
    return i18n("Editor");
}

QIcon EditorConfigPage::icon() const {
    return QIcon::fromTheme(QStringLiteral("accessories-text-editor"));
}

QString EditorConfigPage::fullName() const {
    return i18n("Configure Text editor");
}

QList<ConfigPage*> EditorConfigPage::childPages()
{
    QList<ConfigPage*> ret;
    auto editor = KTextEditor::Editor::instance();
    for (int i = 0; i < editor->configPages(); ++i) {
        ret.append(new KTextEditorConfigPageWrapper(editor->configPage(i, this), this));
    }
    return ret;
};
