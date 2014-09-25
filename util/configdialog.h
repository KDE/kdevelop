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
#ifndef KDEVPLATFORM_CONFIGDIALOG_H
#define KDEVPLATFORM_CONFIGDIALOG_H

#include <KPageDialog>

#include "utilexport.h"

namespace KDevelop {
class ConfigPage;
class IPlugin;

/**
 * This class is meant to be used to show the global config dialog and the per-project config dialog
 */
class KDEVPLATFORMUTIL_EXPORT ConfigDialog : public KPageDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QList<ConfigPage*> pages, QWidget* parent = 0, Qt::WindowFlags flags = 0);

public Q_SLOTS:
    /**
     * Remove a config page
     */
    void removeConfigPage(ConfigPage* page);
    /**
     * Add a new config page.
     * @param page the new page to add
     * @param next if this parameter is passed the new page will be before it, otherwise
     * it will be inserted as the last config page.
     */
    void addConfigPage(ConfigPage* page, ConfigPage* next = nullptr);

Q_SIGNALS:
    void configSaved(ConfigPage* page);

protected:
    virtual void closeEvent(QCloseEvent* event);

private:
    KPageWidgetItem* itemForPage(ConfigPage* page) const;
    int checkForUnsavedChanges(KPageWidgetItem* current, KPageWidgetItem* before);
    void applyChanges(ConfigPage* page);
    void removePagesForPlugin(IPlugin* plugin);
    void addConfigPageInternal(KPageWidgetItem* item, ConfigPage* page);

private:
    // we have to use QPointer since KPageDialog::removePage() also removes all child pages
    QVector<QPointer<KPageWidgetItem>> m_pages;
    bool m_currentPageHasChanges;
};

}

#endif // KDEVPLATFORM_CONFIGDIALOG_H
