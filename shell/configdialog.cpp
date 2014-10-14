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
#include "configdialog.h"

#include <QPushButton>
#include <QCloseEvent>

#include <KMessageBox>
#include <KLocalizedString>
#include <KTextEditor/ConfigPage>


#include <iplugin.h>
#include <configpage.h>
#include <icore.h>
#include <iplugincontroller.h>

KDevelop::ConfigDialog::ConfigDialog(QList<ConfigPage*> pages, QWidget* parent, Qt::WindowFlags flags)
        : KPageDialog(parent, flags), m_currentPageHasChanges(false)
{
    setWindowTitle(i18n("Configure"));
    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    button(QDialogButtonBox::Apply)->setEnabled(false);
    setObjectName(QStringLiteral("configdialog"));

    for (auto page : pages) {
        addConfigPage(page);
    }

    auto onApplyClicked = [this]() {
        auto page = qobject_cast<ConfigPage*>(currentPage()->widget());
        Q_ASSERT(page);
        applyChanges(page);
    };

    connect(button(QDialogButtonBox::Apply), &QPushButton::clicked, onApplyClicked);
    connect(button(QDialogButtonBox::Ok), &QPushButton::clicked, onApplyClicked);
    connect(button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, [this]() {
        auto page = qobject_cast<ConfigPage*>(currentPage()->widget());
        Q_ASSERT(page);
        page->defaults();
    });

    connect(this, &KPageDialog::currentPageChanged, this, &ConfigDialog::checkForUnsavedChanges);
    // make sure we don't keep any entries for unloaded plugins
    connect(ICore::self()->pluginController(), &IPluginController::unloadingPlugin,
            this, &ConfigDialog::removePagesForPlugin);
}


KPageWidgetItem* KDevelop::ConfigDialog::itemForPage(ConfigPage* page) const
{
    for (auto item : m_pages) {
        if (item->widget() == page) {
            return item;
        }
    }
    return nullptr;
}

int KDevelop::ConfigDialog::checkForUnsavedChanges(KPageWidgetItem* current, KPageWidgetItem* before)
{
    Q_UNUSED(current);
    if (m_currentPageHasChanges) {
        // before must be non-null, because if we change from nothing to a new page m_currentPageHasChanges must also be false!
        auto oldPage = qobject_cast<ConfigPage*>(before->widget());
        Q_ASSERT(oldPage);
        auto dialogResult = KMessageBox::warningYesNoCancel(this, i18n("The settings of the current module have changed.\n"
                "Do you want to apply the changes or discard them?"), i18n("Apply Settings"), KStandardGuiItem::apply(),
                KStandardGuiItem::discard(), KStandardGuiItem::cancel());
        if (dialogResult == KMessageBox::No) {
            oldPage->reset();
            m_currentPageHasChanges = false;
            button(QDialogButtonBox::Apply)->setEnabled(false);
        } else if (dialogResult == KMessageBox::Yes) {
            applyChanges(oldPage);
        } else if (dialogResult == KMessageBox::Cancel) {
            // restore old state
            QSignalBlocker block(this); // prevent recursion
            setCurrentPage(before);
        }
        return dialogResult;
    }
    return KMessageBox::Yes;
}

void KDevelop::ConfigDialog::closeEvent(QCloseEvent* event)
{
    if (checkForUnsavedChanges(currentPage(), currentPage()) == KMessageBox::Cancel) {
        // if the user clicked cancel he wants to continue editing the current page -> don't close
        event->ignore();
    } else {
        event->accept();
    }
}

void KDevelop::ConfigDialog::removeConfigPage(ConfigPage* page)
{
    auto item = itemForPage(page);
    Q_ASSERT(item);
    removePage(item);
    m_pages.removeAll(QPointer<KPageWidgetItem>(item));
    // also remove all items that were deleted because a parent KPageWidgetItem was removed
    m_pages.removeAll(QPointer<KPageWidgetItem>());
}

void KDevelop::ConfigDialog::removePagesForPlugin(KDevelop::IPlugin* plugin)
{
    Q_ASSERT(plugin);
    for (auto it = m_pages.begin(); it != m_pages.end(); ++it) {
        auto item = *it;
        if (!item) {
            continue;
        }
        auto page = qobject_cast<ConfigPage*>(item->widget());
        if (page && page->plugin() == plugin) {
            // qDebug("Removing page %s (%s) (%p) because associated plugin (%p) is unloading.",
            //      qPrintable(page->name()), qPrintable(page->fullName()), item.data(), page->plugin());
            removePage(item); // this also deletes the config page -> QPointer is set to null
        }
    };
    // also remove all items that were deleted because a parent KPageWidgetItem was removed
    m_pages.removeAll(QPointer<KPageWidgetItem>());
}

void KDevelop::ConfigDialog::addConfigPage(ConfigPage* page, ConfigPage* previous)
{
    if (previous) {
        auto previousItem = itemForPage(previous);
        Q_ASSERT(previousItem);
        addConfigPageInternal(insertPage(previousItem, page, page->name()), page);
    } else {
        addConfigPageInternal(addPage(page, page->name()), page);
    }
}

void KDevelop::ConfigDialog::addConfigPageInternal(KPageWidgetItem* item, ConfigPage* page)
{
    item->setHeader(page->fullName());
    item->setIcon(page->icon());
    connect(page, &ConfigPage::changed, this, &ConfigDialog::onPageChanged);
    page->initConfigManager();
    m_pages.append(item);
    for (int i = 0; i < page->childPages(); ++i) {
        auto child = page->childPage(i);
        addConfigPageInternal(addSubPage(item, child, child->name()), child);
    }
}

void KDevelop::ConfigDialog::onPageChanged()
{
    QObject* from = sender();
    if (from && from != currentPage()->widget()) {
        qFatal("Settings in a different page changed, this case is not handled yet");
        // TODO: add a QHash<ConfigPage*, bool> as a member to make sure the apply button is always correct

        // TODO: when pressing okay show confirm dialog if other pages have changed or just silently apply every page? "Items on other pages have changed, do you wish to review those changes? + list with changed pages."
    }
    if (!m_currentlyApplyingChanges) {
        // e.g. PluginPreferences emits changed() from its apply method, better fix this here than having to
        // ensure that no plugin emits changed() from apply()
        // together with KPageDialog emitting currentPageChanged("Plugins", nullptr) this could cause a crash
        // when we dereference before
        m_currentPageHasChanges = true;
        button(QDialogButtonBox::Apply)->setEnabled(true);
    }
}


void KDevelop::ConfigDialog::applyChanges(ConfigPage* page)
{
    // must set this to false before calling apply, otherwise we get the confirmation dialog
    // whenever we enable/disable plugins.
    // This is because KPageWidget then emits currentPageChanged("Plugins", nullptr), which seems like a bug to me,
    // it should rather emit currentPageChanged("Plugins", "Plugins") or even better nothing at all, since the current
    // page did not actually change!
    // TODO: fix KPageWidget
    m_currentPageHasChanges = false;
    m_currentlyApplyingChanges = true;
    page->apply();
    m_currentlyApplyingChanges = false;
    Q_ASSERT(!m_currentPageHasChanges);
    button(QDialogButtonBox::Apply)->setEnabled(false);
    emit configSaved(page);
}
