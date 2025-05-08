/*
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "configdialog.h"

#include "debug.h"

#include <QCloseEvent>
#include <QMetaObject>
#include <QPushButton>
#include <QPointer>

#include <KMessageBox>
#include <KLocalizedString>

#include <iplugin.h>
#include <configpage.h>
#include <icore.h>
#include <iplugincontroller.h>

using namespace KDevelop;

//FIXME: unit test this code!

ConfigDialog::ConfigDialog(QWidget* parent)
    : KPageDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Configure"));
    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    button(QDialogButtonBox::Apply)->setEnabled(false);
    setObjectName(QStringLiteral("configdialog"));

    auto onApplyClicked = [this] {
        auto page = qobject_cast<ConfigPage*>(currentPage()->widget());
        Q_ASSERT(page);
        applyChanges(page);
    };

    connect(button(QDialogButtonBox::Apply), &QPushButton::clicked, onApplyClicked);
    connect(button(QDialogButtonBox::Ok), &QPushButton::clicked, [this, onApplyClicked] {
        if (m_currentPageHasChanges) {
            onApplyClicked();
        }
    });
    connect(button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, [this]() {
        auto page = qobject_cast<ConfigPage*>(currentPage()->widget());
        Q_ASSERT(page);
        page->defaults();
    });

    connect(this, &KPageDialog::currentPageChanged, this, &ConfigDialog::checkForUnsavedChanges);
    // make sure we don't keep any entries for unloaded plugins
    connect(ICore::self()->pluginController(), &IPluginController::unloadingPlugin,
            this, &ConfigDialog::removePagesForPlugin);
}


KPageWidgetItem* ConfigDialog::itemForPage(ConfigPage* page) const
{
    for (auto& item : m_pages) {
        if (item->widget() == page) {
            return item;
        }
    }
    return nullptr;
}

int ConfigDialog::checkForUnsavedChanges(KPageWidgetItem* current, KPageWidgetItem* before)
{
    Q_UNUSED(current);

    if (!m_currentPageHasChanges) {
        return KMessageBox::PrimaryAction;
    }

    // before must be non-null, because if we change from nothing to a new page m_currentPageHasChanges must also be false!
    Q_ASSERT(before);
    auto oldPage = qobject_cast<ConfigPage*>(before->widget());
    Q_ASSERT(oldPage);
    auto dialogResult =
        KMessageBox::warningTwoActionsCancel(this,
                                             i18n("The settings of the current module have changed.\n"
                                                  "Do you want to apply the changes or discard them?"),
                                             i18nc("@title:window", "Apply Settings"), KStandardGuiItem::apply(),
                                             KStandardGuiItem::discard(), KStandardGuiItem::cancel());
    if (dialogResult == KMessageBox::SecondaryAction) {
        m_pageBeingReset = oldPage;
        oldPage->reset();
        // KDevelop::ConfigPage::reset() calls KConfigDialogManager::updateWidgets(), which may call
        // QTimer::singleShot(0, this, &KConfigDialogManager::widgetModified). ConfigDialog::onPageChanged() checks
        // m_pageBeingReset and is connected to the signal ConfigPage::changed(), which in turn is connected to the
        // signal KConfigDialogManager::widgetModified(). Unset m_pageBeingReset asynchronously after the reset()
        // call so that it is still set during the expected invocation(s) of ConfigDialog::onPageChanged().
        QMetaObject::invokeMethod(
            this,
            [this] {
                m_pageBeingReset = nullptr;
            },
            Qt::QueuedConnection);

        m_currentPageHasChanges = false;
        button(QDialogButtonBox::Apply)->setEnabled(false);
    } else if (dialogResult == KMessageBox::PrimaryAction) {
        applyChanges(oldPage);
    } else if (dialogResult == KMessageBox::Cancel) {
        // restore old state
        QSignalBlocker block(this); // prevent recursion
        setCurrentPage(before);
    }
    return dialogResult;
}

void ConfigDialog::closeEvent(QCloseEvent* event)
{
    if (checkForUnsavedChanges(currentPage(), currentPage()) == KMessageBox::Cancel) {
        // if the user clicked cancel he wants to continue editing the current page -> don't close
        event->ignore();
    } else {
        event->accept();
    }
}

void ConfigDialog::removeConfigPage(ConfigPage* page)
{
    auto item = itemForPage(page);
    Q_ASSERT(item);
    removePage(item);
    m_pages.removeAll(QPointer<KPageWidgetItem>(item));
    // also remove all items that were deleted because a parent KPageWidgetItem was removed
    m_pages.removeAll(QPointer<KPageWidgetItem>());
}

void ConfigDialog::removePagesForPlugin(IPlugin* plugin)
{
    Q_ASSERT(plugin);
    const auto oldPages = m_pages;
    for (auto&& item : oldPages) {
        if (!item) {
            continue;
        }
        auto page = qobject_cast<ConfigPage*>(item->widget());
        if (page && page->plugin() == plugin) {
            removePage(item); // this also deletes the config page -> QPointer is set to null
        }
    };
    // also remove all items that were deleted because a parent KPageWidgetItem was removed
    m_pages.removeAll(QPointer<KPageWidgetItem>());
}

void ConfigDialog::appendConfigPage(ConfigPage* page)
{
    addConfigPageInternal(addPage(page, page->name()), page);
}

void ConfigDialog::insertConfigPage(ConfigPage* before, ConfigPage* page)
{
    Q_ASSERT(before);
    auto beforeItem = itemForPage(before);
    Q_ASSERT(beforeItem);
    addConfigPageInternal(insertPage(beforeItem, page, page->name()), page);
}

void ConfigDialog::appendSubConfigPage(ConfigPage* parentPage, ConfigPage* page)
{
    auto item = itemForPage(parentPage);
    Q_ASSERT(item);
    addConfigPageInternal(addSubPage(item, page, page->name()), page);
}

void ConfigDialog::addConfigPageInternal(KPageWidgetItem* item, ConfigPage* page)
{
    item->setHeader(page->fullName());
    item->setIcon(page->icon());
    page->initConfigManager();
    // connect to changed() *after* calling initConfigManager(), which may call reset()
    connect(page, &ConfigPage::changed, this, &ConfigDialog::onPageChanged);
    m_pages.append(item);
    for (int i = 0; i < page->childPages(); ++i) {
        auto child = page->childPage(i);
        appendSubConfigPage(page, child);
    }
}

void ConfigDialog::onPageChanged()
{
    QObject* from = sender();
    if (from && from != currentPage()->widget()) {
        if (from == m_pageBeingReset) {
            qCDebug(SHELL) << "Settings changed in config page" << from << "that is being reset, while"
                           << currentPage()->widget() << "is currently selected.";
            // This is normal and expected. Nothing to do because calling
            // ConfigPage::reset() reverts all changes in the page.
        } else {
            qCWarning(SHELL) << "Settings in config page" << from << "changed, while" << currentPage()->widget()
                             << "is currently selected. This case is not implemented yet.";
        }
        return;
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


void ConfigDialog::applyChanges(ConfigPage* page)
{
    Q_ASSERT(m_currentPageHasChanges);
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

#include "moc_configdialog.cpp"
