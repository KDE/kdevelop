/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "selectcoredialog.h"

#include <KLocalizedString>

#include <QFileInfo>

using namespace KDevMI;

SelectCoreDialog::SelectCoreDialog(QWidget* parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    setWindowTitle(i18nc("@title:window", "Select Core File"));

    for (const auto* const urlRequester : {m_ui.executableFile, m_ui.coreFile}) {
        connect(urlRequester, &KUrlRequester::textChanged, this, &SelectCoreDialog::validateSelection);
    }
    validateSelection();
}

QUrl SelectCoreDialog::executableFile() const
{
    return m_ui.executableFile->url();
}

QUrl SelectCoreDialog::core() const
{
    return m_ui.coreFile->url();
}

void SelectCoreDialog::validateSelection()
{
    const auto reportError = [this](const QString& errorMessage) {
        m_ui.selectionValidWidget->setText(errorMessage);
        m_ui.selectionValidWidget->animatedShow();
        m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    };

    // an empty executable is acceptable
    if (const auto executableUrl = executableFile(); !executableUrl.isEmpty()) {
        if (!executableUrl.isValid()) {
            reportError(i18n("Invalid executable file name"));
            return;
        }

        const QFileInfo executableInfo(executableUrl.toLocalFile());
        if (!executableInfo.exists()) {
            reportError(i18n("Executable file does not exist"));
            return;
        }
        if (!executableInfo.isFile()) {
            reportError(i18n("Executable is not a file"));
            return;
        }
        if (!executableInfo.isReadable()) {
            reportError(i18n("Executable file is not readable"));
            return;
        }
    }

    const auto coreUrl = core();
    if (coreUrl.isEmpty()) {
        reportError(i18n("Empty core file name"));
        return;
    }
    if (!coreUrl.isValid()) {
        reportError(i18n("Invalid core file name"));
        return;
    }

    const QFileInfo coreInfo(coreUrl.toLocalFile());
    if (!coreInfo.exists()) {
        reportError(i18n("Core file does not exist"));
        return;
    }
    if (!coreInfo.isFile()) {
        reportError(i18n("Core is not a file"));
        return;
    }
    if (!coreInfo.isReadable()) {
        reportError(i18n("Core file is not readable"));
        return;
    }

    m_ui.selectionValidWidget->animatedHide();
    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

#include "moc_selectcoredialog.cpp"
