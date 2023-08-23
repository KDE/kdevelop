/*
    SPDX-FileCopyrightText: 2012 Ivan Shapovalov <intelfx100@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "shellutils.h"

#include <QDialog>
#include <QGuiApplication>
#include <QTextStream>
#include <QUrl>
#include <QFile>
#include <QList>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include <KLocalizedString>
#include <KMessageBox>
#include <KMessageBox_KDevCompat>
#include <KParts/MainWindow>
#include <KConfigGroup>
#include <KSharedConfig>

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
        out << mainText << Qt::endl;
        QTextStream in(stdin);
        QString input;
        while (true) {
            if (ttyDefaultToYes) {
                out << ttyPrompt << QLatin1String(": [Y/n] ") << Qt::flush;
            } else {
                out << ttyPrompt << QLatin1String(": [y/N] ") << Qt::flush;
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
        int userAnswer = KMessageBox::questionTwoActions(ICore::self()->uiController()->activeMainWindow(),
                                                         mainText + QLatin1String("\n\n") + mboxAdditionalText,
                                                         mboxTitle, okButton, rejectButton);
        return userAnswer == KMessageBox::PrimaryAction;
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
        int answer = KMessageBox::questionTwoActionsCancel(
            ICore::self()->uiController()->activeMainWindow(),
            i18n("You don't have write permissions for the following files; add write permissions for owner before "
                 "saving?")
                + QLatin1String("\n\n") + notWritable.join(QLatin1Char('\n')),
            i18nc("@title:window", "Some Files are Write-Protected"),
            KGuiItem(i18nc("@action:button", "Set Write Permissions"), QStringLiteral("dialog-ok")),
            KGuiItem(i18nc("@action:button", "Ignore"), QStringLiteral("dialog-cancel")), KStandardGuiItem::cancel());
        if (answer == KMessageBox::PrimaryAction) {
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
                                   i18nc("@title:window", "Failed Setting Permissions"));
                return false;
            }
        }
        return answer != KMessageBox::Cancel;
    }
    return true;
}

bool restoreAndAutoSaveGeometry(QDialog& dialog, const QString& configGroupName, const QString& configSubgroupName)
{
    static const QString entryName = QStringLiteral("windowGeometry");

    // Capture strings by value, because this lambda itself is captured by another lambda
    // connected to a signal, which is likely to be emitted after the strings are destroyed.
    const auto useGroup = [configGroupName, configSubgroupName](auto groupUser) {
        KConfigGroup group(KSharedConfig::openConfig(), configGroupName);
        if (configSubgroupName.isEmpty()) {
            return groupUser(group);
        }
        KConfigGroup subgroup(&group, configSubgroupName);
        return groupUser(subgroup);
    };

    QObject::connect(&dialog, &QDialog::finished, [&dialog, useGroup] {
        useGroup([&dialog](KConfigGroup& group) {
            group.writeEntry(entryName, dialog.saveGeometry());
        });
    });

    return useGroup([&dialog](const KConfigGroup& group) {
        return dialog.restoreGeometry(group.readEntry(entryName, QByteArray{}));
    });
}
}
