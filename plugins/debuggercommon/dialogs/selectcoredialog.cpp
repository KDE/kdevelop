/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "selectcoredialog.h"

#include <KLocalizedString>

using namespace KDevMI;

SelectCoreDialog::SelectCoreDialog(QWidget* parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    setWindowTitle(i18nc("@title:window", "Select Core File"));
}

QUrl SelectCoreDialog::executableFile() const
{
    return m_ui.executableFile->url();
}

QUrl SelectCoreDialog::core() const
{
    return m_ui.coreFile->url();
}
