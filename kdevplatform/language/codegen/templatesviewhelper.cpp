/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "templatesviewhelper.h"

#include "templatesmodel.h"

#include <debug.h>

#include <KNSCore/Entry>

#include <QStringList>

using namespace KDevelop;

bool TemplatesViewHelper::handleNewStuffDialogFinished(const QList<KNSCore::Entry>& changedEntries)
{
    if (changedEntries.empty()) {
        return true; // nothing has changed, so nothing to do
    }

    m_model.refresh();

    // Try to select one of the newly installed templates in the UI.
    for (const auto& entry : changedEntries) {
        const auto installedFiles = entry.installedFiles();
        if (installedFiles.empty()) {
            continue; // this template was uninstalled, keep looking for an installed one
        }
        if (installedFiles.size() > 1) {
            qCWarning(LANGUAGE) << "template archives are never uncompressed, so why more than one,"
                                << installedFiles.size() << "files are installed?";
        }
        if (setCurrentTemplate(installedFiles.constFirst())) {
            return true; // selected one, nothing more to do
        }
    }

    return false; // nothing is selected after the refreshing
}

bool TemplatesViewHelper::setCurrentTemplate(const QString& fileName)
{
    return setCurrentTemplate(m_model.templateIndexes(fileName));
}
