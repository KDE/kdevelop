/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "templatesviewhelper.h"

#include "templatesmodel.h"

#include <debug.h>

#include <util/scopeddialog.h>

#include <KLocalizedString>
#include <KNSCore/Entry>

#include <QFileDialog>
#include <QStringList>

using namespace KDevelop;

bool TemplatesViewHelper::handleNewStuffDialogFinished(const QList<KNSCore::Entry>& changedEntries)
{
    if (changedEntries.empty()) {
        return true; // nothing has changed, so nothing to do
    }

    const auto selectNewlyInstalledTemplateInUi = [&changedEntries, this] {
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
        return false;
    };
    return refreshModelAndSelectTemplate(selectNewlyInstalledTemplateInUi);
}

bool TemplatesViewHelper::loadTemplatesFromFiles(QWidget* dialogParent)
{
    static const QStringList supportedMimeTypes{QStringLiteral("application/x-desktop"),
                                                QStringLiteral("application/x-bzip-compressed-tar"),
                                                QStringLiteral("application/zip")};
    ScopedDialog<QFileDialog> fileDialog(dialogParent, i18nc("@title:window", "Load Template from File"));
    fileDialog->setMimeTypeFilters(supportedMimeTypes);
    fileDialog->setFileMode(QFileDialog::ExistingFiles);

    if (!fileDialog->exec()) {
        return true; // canceled by the user, so nothing to do
    }

    // Load templates from the files selected by the user.
    const auto selectedFiles = fileDialog->selectedFiles();
    QStringList templateFileNames;
    templateFileNames.reserve(selectedFiles.size());
    for (const auto& selectedFile : selectedFiles) {
        templateFileNames.push_back(m_model.loadTemplateFile(selectedFile));
    }

    const auto selectNewlyLoadedTemplateInUi = [&templateFileNames, this] {
        for (const auto& templateFileName : std::as_const(templateFileNames)) {
            if (setCurrentTemplate(templateFileName)) {
                return true; // selected one, nothing more to do
            }
        }
        return false;
    };
    return refreshModelAndSelectTemplate(selectNewlyLoadedTemplateInUi);
}

void TemplatesViewHelper::refreshModel()
{
    m_model.refresh();
}

bool TemplatesViewHelper::setCurrentTemplate(const QString& fileName)
{
    return setCurrentTemplate(m_model.templateIndexes(fileName));
}

template<typename SelectNewTemplateInUi>
bool TemplatesViewHelper::refreshModelAndSelectTemplate(SelectNewTemplateInUi selectNewTemplateInUi)
{
    // Retrieve the currently selected template before refreshing the model and invalidating the selection.
    const auto lastSelectedFileName = currentTemplateFileName();
    refreshModel();

    if (selectNewTemplateInUi()) {
        return true;
    }

    // Reselect the previously selected template as a fallback.
    if (!lastSelectedFileName.isEmpty() && setCurrentTemplate(lastSelectedFileName)) {
        return true;
    }
    return false;
}
