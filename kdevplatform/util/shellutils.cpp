/*
 * This file is part of KDevelop
 *
 * Copyright 2012 Ivan Shapovalov <intelfx100@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "shellutils.h"
#include <QGuiApplication>
#include <QTextStream>
#include <QUrl>
#include <QFile>
#include <QList>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include <KLocalizedString>
#include <KMessageBox>
#include <KParts/MainWindow>

namespace KDevelop {

bool askUser(const QString& mainText,
             const QString& ttyPrompt,
             const QString& mboxTitle,
             const QString& mboxAdditionalText,
             const QString& confirmText,
             const QString& rejectText,
             bool ttyDefaultToYes)
{
    if (!qobject_cast<QGuiApplication*>(qApp)) {
        // no ui-mode e.g. for duchainify and other tools
        QTextStream out(stdout);
        out << mainText << endl;
        QTextStream in(stdin);
        QString input;
        forever {
            if (ttyDefaultToYes) {
                out << QStringLiteral("%1: [Y/n] ").arg(ttyPrompt) << flush;
            } else {
                out << QStringLiteral("%1: [y/N] ").arg(ttyPrompt) << flush;
            }
            input = in.readLine().trimmed();
            if (input.isEmpty()) {
                return ttyDefaultToYes;
            } else if (input.toLower() == QLatin1String("y")) {
                return true;
            } else if (input.toLower() == QLatin1String("n")) {
                return false;
            }
        }
    } else {
        auto okButton = KStandardGuiItem::ok();
        okButton.setText(confirmText);
        auto rejectButton = KStandardGuiItem::cancel();
        rejectButton.setText(rejectText);
        int userAnswer = KMessageBox::questionYesNo(ICore::self()->uiController()->activeMainWindow(),
                                                    mainText + QLatin1String("\n\n") + mboxAdditionalText,
                                                    mboxTitle,
                                                    okButton,
                                                    rejectButton);
        return userAnswer == KMessageBox::Yes;
    }
}

bool ensureWritable(const QList<QUrl>& urls)
{
    QStringList notWritable;
    for (const QUrl& url : urls) {
        if (url.isLocalFile()) {
            QFile file(url.toLocalFile());
            if (file.exists() && !(file.permissions() & QFileDevice::WriteOwner) &&
                !(file.permissions() & QFileDevice::WriteGroup)) {
                notWritable << url.toLocalFile();
            }
        }
    }

    if (!notWritable.isEmpty()) {
        int answer = KMessageBox::questionYesNoCancel(ICore::self()->uiController()->activeMainWindow(),
                                                      i18n(
                                                          "You don't have write permissions for the following files; add write permissions for owner before saving?") +
                                                      QLatin1String("\n\n") + notWritable.join(QLatin1Char('\n')),
                                                      i18n("Some files are write-protected"),
                                                      KStandardGuiItem::yes(),
                                                      KStandardGuiItem::no(), KStandardGuiItem::cancel());
        if (answer == KMessageBox::Yes) {
            bool success = true;
            for (const QString& filename : qAsConst(notWritable)) {
                QFile file(filename);
                QFileDevice::Permissions permissions = file.permissions();
                permissions |= QFileDevice::WriteOwner;
                success &= file.setPermissions(permissions);
            }

            if (!success) {
                KMessageBox::error(ICore::self()->uiController()->activeMainWindow(),
                                   i18n("Failed adding write permissions for some files."),
                                   i18n("Failed setting permissions"));
                return false;
            }
        }
        return answer != KMessageBox::Cancel;
    }
    return true;
}

}
