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
#ifndef KDEVELOP_CONFIGPAGE_H
#define KDEVELOP_CONFIGPAGE_H

#include <KTextEditor/ConfigPage>
#include <KLocalizedString>

#include "interfacesexport.h"

class KCoreConfigSkeleton;

namespace KDevelop {
class IPlugin;
class ConfigPagePrivate;

class KDEVPLATFORMINTERFACES_EXPORT ConfigPage : public KTextEditor::ConfigPage
{
    Q_OBJECT

public:
    /**
     * Create a new config page
     * @param plugin the plugin that created this config page
     * @param config the config skeleton that is used to store the preferences. If you don't use
     * a K(Core)ConfigSkeleton to save the settings you can also pass null here.
     * However this means that you will have to manually implement the apply(), defaults() and reset() slots
     * @param parent parent widget
     */
    explicit ConfigPage(IPlugin* plugin, KCoreConfigSkeleton* config = nullptr, QWidget* parent = nullptr);
    ~ConfigPage() override;

    /**
     * Get the number of subpages of this page
     * @return The number of child pages or an integer < 1 if there are none.
     * The default implementation returns zero.
     */
    virtual int childPages() const;

    /**
     * @return the child config page for index @p number or @c nullptr if there is none.
     * The default implementation returns @c nullptr.
     */
    virtual ConfigPage* childPage(int number);

    enum ConfigPageType
    {
        DefaultConfigPage,
        LanguageConfigPage, ///< A config page that contains language specific settings. This page is appended as a child page to the "Language support" config page.
        AnalyzerConfigPage, ///< A config page that contains settings for some analyzer. This page is appended as a child page to the "Analyzers" config page.
        DocumentationConfigPage, ///< A config page that contains settings for some documentation plugin. This page is appended as a child page to the "Documentation" config page.
        RuntimeConfigPage ///< A config page that contains settings for some runtime plugin. This page is appended as a child page to the "Runtimes" config page.
    };

    /**
     * @return The type of this config page. Default implementaion returns DefaultConfigPageType
     */
    virtual ConfigPageType configPageType() const;

    /**
     * @return the plugin that this config page was created by or nullptr if it was not created by a plugin.
     */
    IPlugin* plugin() const;

    /**
     * Initializes the KConfigDialogManager.
     * Must be called explicitly since not all child widgets are available at the end of the constructor.
     * This is handled automatically by KDevelop::ConfigDialog, subclasses don't need to call this.
     */
    void initConfigManager();

    /**
     * @return the KCoreConfigSkeleton used to store the settings for this page or @c nullptr
     * if settings are managed differently
     */
    KCoreConfigSkeleton* configSkeleton() const;

    /**
     * Sets the config skeleton to @p skel and will create a KConfigDialogManager if needed.
     * This can be used if the KCoreConfigSkeleton* doesn't exist yet when calling the base class constructor.
     */
    void setConfigSkeleton(KCoreConfigSkeleton* skel);

public Q_SLOTS:
    void apply() override;
    void defaults() override;
    void reset() override;

private:
    const QScopedPointer<class ConfigPagePrivate> d_ptr;
    Q_DECLARE_PRIVATE(ConfigPage)
};

}

#endif // KDEVELOP_CONFIGPAGE_H
