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

    auto applyChanges = [this]() {
        auto page = qobject_cast<ConfigPage*>(currentPage()->widget());
        Q_ASSERT(page);
        page->apply();
        m_currentPageHasChanges = false;
        button(QDialogButtonBox::Apply)->setEnabled(false);
        qDebug("Applied changes");
    };
    connect(button(QDialogButtonBox::Apply), &QPushButton::clicked, applyChanges);
    connect(button(QDialogButtonBox::Ok), &QPushButton::clicked, applyChanges);
    connect(button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, [this]() {
        auto page = qobject_cast<ConfigPage*>(currentPage()->widget());
        Q_ASSERT(page);
        page->defaults();
        qDebug("Restored defaults");
    });

    connect(this, &KPageDialog::currentPageChanged, this, &ConfigDialog::checkForUnsavedChanges);
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
            oldPage->apply();
            m_currentPageHasChanges = false;
            button(QDialogButtonBox::Apply)->setEnabled(false);
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
    // only close dialog if the user doesn't click cancel
    if (checkForUnsavedChanges(currentPage(), currentPage()) == KMessageBox::Cancel) {
        qDebug("Not closing");
        event->ignore();
    } else {
        qDebug("closing");
        event->accept();
    }
}

void KDevelop::ConfigDialog::removeConfigPage(ConfigPage* page)
{
    auto item = itemForPage(page);
    Q_ASSERT(item);
    removePage(item);

}

void KDevelop::ConfigDialog::addConfigPage(ConfigPage* page, ConfigPage* previous)
{
    KPageWidgetItem* item;
    if (previous) {
        auto previousItem = itemForPage(previous);
        Q_ASSERT(previousItem);
        item = insertPage(previousItem, page, page->name());
    } else {
         item = addPage(page, page->name());
    }
    m_pages.append(item);
    item->setHeader(page->fullName());
    item->setIcon(page->icon());
    auto onChanged = [this]() {
        m_currentPageHasChanges = true;
        button(QDialogButtonBox::Apply)->setEnabled(true);
    };
    connect(page, &ConfigPage::changed, onChanged);
    page->initConfigManager();
    for (auto child : page->childPages()) {
        KPageWidgetItem* childItem = addSubPage(item, child, child->name());
        childItem->setHeader(child->fullName());
        childItem->setIcon(child->icon());
        connect(child, &ConfigPage::changed, onChanged);
        child->initConfigManager();
    }
}
