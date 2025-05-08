/*
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KDEVPLATFORM_CONFIGDIALOG_H
#define KDEVPLATFORM_CONFIGDIALOG_H

#include <KPageDialog>

namespace KDevelop {
class ConfigPage;
class IPlugin;

/**
 * This class is meant to be used to show the global config dialog and the per-project config dialog.
 *
 * This used to be handled by KSettings::Dialog, but since we are no longer using KCMs for config widgets,
 * we use this class instead.
 *
 * TODO: check if we can share this with Kate
 */
class ConfigDialog : public KPageDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget* parent = nullptr);

public Q_SLOTS:
    /**
     * Remove a config page
     */
    void removeConfigPage(ConfigPage* page);

    /**
     * Add a new config page to the end of the list of pages.
     * @param page the new page to add
     */
    void appendConfigPage(ConfigPage* page);

    /**
     * Add a new config page.
     * @param before the page before which the new page will be inserted.
     * @param page the new page to add
     */
    void insertConfigPage(ConfigPage* before, ConfigPage* page);

    /**
     * Add a new sub config page
     * @param parentPage the parent page
     * @param page the page to add
     */
    void appendSubConfigPage(ConfigPage* parentPage, ConfigPage* page);

Q_SIGNALS:
    void configSaved(ConfigPage* page);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    KPageWidgetItem* itemForPage(ConfigPage* page) const;
    int checkForUnsavedChanges(KPageWidgetItem* current, KPageWidgetItem* before);
    void applyChanges(ConfigPage* page);
    void removePagesForPlugin(IPlugin* plugin);
    void addConfigPageInternal(KPageWidgetItem* item, ConfigPage* page);
    void onPageChanged();

private:
    // we have to use QPointer since KPageDialog::removePage() also removes all child pages
    QVector<QPointer<KPageWidgetItem>> m_pages;
    bool m_currentPageHasChanges = false;
    bool m_currentlyApplyingChanges = false;
    const ConfigPage* m_pageBeingReset = nullptr; ///< the config page, on which reset() is being called
};

}

#endif // KDEVPLATFORM_CONFIGDIALOG_H
