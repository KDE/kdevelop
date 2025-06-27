/*
    SPDX-FileCopyrightText: 2012 Ivan Shapovalov <intelfx100@gmail.com>
    SPDX-FileCopyrightText: 2024, 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "shellutils.h"

#include <debug.h>

#include <interfaces/icore.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iuicontroller.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KParts/MainWindow>
#include <KSharedConfig>
#include <KShell>

#include <QEvent>
#include <QFile>
#include <QGuiApplication>
#include <QList>
#include <QMenu>
#include <QTextStream>
#include <QUrl>
#include <QWidget>

namespace {

class WidgetGeometrySaver : public QObject
{
    Q_OBJECT
public:
    explicit WidgetGeometrySaver(QWidget& widget, const QString& configGroupName, const QString& configSubgroupName)
        : QObject(&widget)
        , m_configGroupName{configGroupName}
        , m_configSubgroupName{configSubgroupName}
    {
        widget.installEventFilter(this);
    }

    bool restoreWidgetGeometry() const
    {
        return widget().restoreGeometry(configGroup().readEntry(entryName(), QByteArray{}));
    }

    bool eventFilter(QObject* watched, QEvent* event) override
    {
        // Saving geometry on a nonspontaneous hide event is the only choice that is both reliable and safe:
        // * the close event is received only when a dialog is dismissed via
        //   its standard window close button and not when it is accepted or rejected;
        // * the QDialog::finished() signal is unavailable for non-dialog widgets (less general) and is not emitted
        //   if the dialog is explicitly hidden or destroyed while visible rather than finished normally;
        // * Qt documentation does not directly promise to emit the QObject::destroyed() signal from ~QWidget()
        //   as opposed to the too-late ~QObject(). Also Qt developers do not guarantee that the state
        //   of the widget being destroyed can be safely accessed in a slot connected to this signal.
        //   Some parts of the widget, e.g. its layout, are destroyed before the destroyed() signal is emitted.
        // A downside of handling the hide event is that when a widget is hidden, then shown again, its geometry
        // is saved multiple times redundantly. Fortunately, a dialog is unlikely to be shown again
        // after being hidden. If the redundant saving becomes a bottleneck for some widget, the visibility
        // of which changes often, a custom saving of geometry can be implemented in that widget's destructor.
        // See also the discussion of when to save geometry at https://codereview.qt-project.org/c/qt/qtbase/+/498797
        if (event->type() == QEvent::Hide && !event->spontaneous()) {
            const auto& widget = this->widget();
            Q_ASSERT(watched == &widget);
            configGroup().writeEntry(entryName(), widget.saveGeometry());
        }
        return false; // do not filter any events out
    }

private:
    static QString entryName()
    {
        return QStringLiteral("windowGeometry");
    }

    QWidget& widget() const
    {
        Q_ASSERT(parent());
        Q_ASSERT(parent()->isWidgetType());
        return *static_cast<QWidget*>(parent());
    }

    KConfigGroup configGroup() const
    {
        KConfigGroup group(KSharedConfig::openConfig(), m_configGroupName);
        if (m_configSubgroupName.isEmpty()) {
            return group;
        }
        KConfigGroup subgroup(&group, m_configSubgroupName);
        return subgroup;
    }

    const QString m_configGroupName;
    const QString m_configSubgroupName;
};

} // unnamed namespace

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

        auto* parent = ICore::self()->uiController()->activeMainWindow();
        if (parent && !parent->isVisible()) {
            // On Wayland, a dialog with an invisible or null parent appears
            // at the top-left corner of the primary screen.
            // On X11, a dialog with an invisible parent appears close to the top-left corner of the
            // left screen, even if the left screen is secondary. A dialog with a null parent appears
            // at the center of the primary screen. The center of the primary screen is a much more
            // convenient location, especially if the secondary screen is off.
            // KMessageBox::questionTwoActions() destroys its dialog before returning in any case.
            qCDebug(UTIL) << "the main window is invisible, so not using it as a dialog parent";
            parent = nullptr;
        }

        const auto userAnswer = KMessageBox::questionTwoActions(
            parent, mainText + QLatin1String("\n\n") + mboxAdditionalText, mboxTitle, okButton, rejectButton);
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
            for (const QString& filename : std::as_const(notWritable)) {
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

bool restoreAndAutoSaveGeometry(QWidget& widget, const QString& configGroupName, const QString& configSubgroupName)
{
    const auto* const saver = new WidgetGeometrySaver(widget, configGroupName, configSubgroupName);
    return saver->restoreWidgetGeometry();
}

void prepareStandardContextMenuToAddingCustomActions(QMenu*& standardContextMenu, QWidget* menuParent)
{
    if (!standardContextMenu) {
        standardContextMenu = new QMenu(menuParent);
    } else if (!standardContextMenu->isEmpty()) {
        standardContextMenu->addSeparator();
    }
}

/// This macro is used to deduplicate while avoiding evaluating arguments if the warning debug output is disabled.
#define ENTRY_NAME_FOR_LAUNCH_CONFIGURATION                                                                            \
    entryName.untranslatable << "for the launch configuration" << launchConfiguration.name()

QStringList splitLaunchConfigurationEntry(const ILaunchConfiguration& launchConfiguration, const QString& entryText,
                                          const LaunchConfigurationEntryName& entryName, QString& errorMessage)
{
    KShell::Errors errorCode;
    auto ret = KShell::splitArgs(entryText, KShell::TildeExpand | KShell::AbortOnMeta, &errorCode);

    switch (errorCode) {
    case KShell::NoError:
        return ret;
    case KShell::BadQuoting:
        errorMessage = i18nc("%1 - entry name, e.g. 'arguments' or 'executable path'; %2 - launch configuration name",
                             "There is a quoting error in the %1 for the launch configuration '%2'. Aborting start.",
                             entryName.translatable, launchConfiguration.name());
        qCWarning(UTIL) << "quoting error in the" << ENTRY_NAME_FOR_LAUNCH_CONFIGURATION;
        return {};
    case KShell::FoundMeta:
        errorMessage = i18nc("%1 - entry name, e.g. 'arguments' or 'executable path'; %2 - launch configuration name",
                             "A shell meta character was included in the %1 for the launch configuration '%2', "
                             "this is not supported currently. Aborting start.",
                             entryName.translatable, launchConfiguration.name());
        qCWarning(UTIL) << "shell meta character in the" << ENTRY_NAME_FOR_LAUNCH_CONFIGURATION;
        return {};
    }

    errorMessage =
        i18nc("%1 - entry name, e.g. 'arguments' or 'executable path'; %2 - error code; %3 - launch configuration name",
              "KShell::splitArgs(<the %1>) reported an unknown error code %2 "
              "for the launch configuration '%3'. Aborting start.",
              entryName.translatable, errorCode, launchConfiguration.name());
    qCWarning(UTIL) << "unknown error code" << errorCode << "reported for the" << ENTRY_NAME_FOR_LAUNCH_CONFIGURATION;
    return {};
}

static QStringList splitLaunchConfigurationEntryToNonemptyList(const ILaunchConfiguration& launchConfiguration,
                                                               const QString& entryText,
                                                               const LaunchConfigurationEntryName& entryName,
                                                               QString& errorMessage)
{
    // Do not pass errorMessage to splitLaunchConfigurationEntry(), because it may be nonempty from the beginning.
    QString initiallyEmptyErrorMessage;
    auto ret = splitLaunchConfigurationEntry(launchConfiguration, entryText, entryName, initiallyEmptyErrorMessage);

    if (!initiallyEmptyErrorMessage.isEmpty()) {
        // Assign to errorMessage only a nonempty value so as to avoid touching
        // it in the absence of an error, and thus conform to the documentation.
        errorMessage = std::move(initiallyEmptyErrorMessage);
        return ret;
    }

    if (ret.empty()) {
        // No prior error but the result of the splitting is empty. Report this as an error.
        errorMessage = i18nc(
            "%1 - entry name, e.g. 'executable path' or 'external terminal command'; %2 - launch configuration name",
            "Splitting the %1 for the launch configuration '%2' yields an empty list. Aborting start.",
            entryName.translatable, launchConfiguration.name());
        qCWarning(UTIL) << "empty result of splitting the" << ENTRY_NAME_FOR_LAUNCH_CONFIGURATION;
        return {};
    }

    return ret;
}

QStringList splitNonemptyLaunchConfigurationEntry(const ILaunchConfiguration& launchConfiguration,
                                                  const QString& entryText,
                                                  const LaunchConfigurationEntryName& entryName, QString& errorMessage)
{
    if (entryText.isEmpty()) {
        errorMessage = i18nc(
            "%1 - entry name, e.g. 'external terminal command' or 'interpreter command'; "
            "%2 - launch configuration name",
            "Empty %1 specified for the launch configuration '%2'. Aborting start.", entryName.translatable,
            launchConfiguration.name());
        qCWarning(UTIL) << "empty" << ENTRY_NAME_FOR_LAUNCH_CONFIGURATION;
        return {};
    }

    return splitLaunchConfigurationEntryToNonemptyList(launchConfiguration, entryText, entryName, errorMessage);
}

QStringList splitLocalFileLaunchConfigurationEntry(const ILaunchConfiguration& launchConfiguration,
                                                   const QUrl& entryUrl, const LaunchConfigurationEntryName& entryName,
                                                   QString& errorMessage)
{
    if (entryUrl.isEmpty() || !entryUrl.isLocalFile()) {
        errorMessage = i18nc("%1 - entry name, e.g. 'executable path' or 'script path'; %2 - launch configuration name",
                             "No valid %1 specified for the launch configuration '%2'. Aborting start.",
                             entryName.translatable, launchConfiguration.name());
        qCWarning(UTIL) << "invalid or empty" << ENTRY_NAME_FOR_LAUNCH_CONFIGURATION;
        return {};
    }

    return splitLaunchConfigurationEntryToNonemptyList(launchConfiguration, entryUrl.toLocalFile(), entryName,
                                                       errorMessage);
}

#undef ENTRY_NAME_FOR_LAUNCH_CONFIGURATION

} // namespace KDevelop

#include "shellutils.moc"
