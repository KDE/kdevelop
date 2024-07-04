// SPDX-FileCopyrightText: 2022 Gleb Popov <arrowd@FreeBSD.org>
// SPDX-License-Identifier: BSD-3-Clause

#include "craftplugin.h"
#include "craftruntime.h"
#include "debug_craft.h"

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruntimecontroller.h>
#include <interfaces/iuicontroller.h>

#include <KParts/MainWindow>
#include <KPluginFactory>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KMessageBox>
#include <KMessageBox_KDevCompat>

K_PLUGIN_FACTORY_WITH_JSON(KDevCraftFactory, "kdevcraft.json", registerPlugin<CraftPlugin>();)

using namespace KDevelop;

namespace {
bool wantAutoEnable(KDevelop::IProject* project, const QString& craftRoot)
{
    auto projectConfigGroup = project->projectConfiguration()->group(QStringLiteral("Project"));
    const bool haveConfigEntry = projectConfigGroup.entryMap().contains(QLatin1String("AutoEnableCraftRuntime"));

    if (!haveConfigEntry) {
        const QString msgboxText = i18n(
            "The project being loaded (%1) is detected to reside\n"
            "under a Craft root [%2] .\nDo you want to automatically switch to the Craft runtime?",
            project->name(), craftRoot);

        auto answer = KMessageBox::questionTwoActions(
            ICore::self()->uiController()->activeMainWindow(), msgboxText, QString(),
            KGuiItem(i18nc("@action:button", "Switch to Craft Runtime"), QStringLiteral("dialog-ok")),
            KGuiItem(i18nc("@action:button", "Do not switch automatically"), QStringLiteral("dialog-cancel")));
        projectConfigGroup.writeEntry("AutoEnableCraftRuntime", answer == KMessageBox::PrimaryAction);
        return answer == KMessageBox::PrimaryAction;
    } else {
        return projectConfigGroup.readEntry("AutoEnableCraftRuntime", false);
    }
}
}

CraftPlugin::CraftPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& /*args*/)
    : IPlugin(QStringLiteral("kdevcraft"), parent, metaData)
{
    const QString pythonExecutable = CraftRuntime::findPython();
    if (pythonExecutable.isEmpty())
        return;

    // If KDevelop itself runs under Craft env, this plugin has nothing to do
    if (qEnvironmentVariableIsSet("KDEROOT"))
        return;

    connect(ICore::self()->projectController(), &IProjectController::projectAboutToBeOpened, this,
            [pythonExecutable](KDevelop::IProject* project) {
                const QString craftRoot = CraftRuntime::findCraftRoot(project->path());
                auto* currentCraftRuntime =
                    qobject_cast<CraftRuntime*>(ICore::self()->runtimeController()->currentRuntime());

                if (craftRoot.isEmpty()) {
                    if (currentCraftRuntime)
                        qCDebug(CRAFT) << "Loading a non-Craft project while Craft runtime is enabled. This will cause "
                                          "the project to build and run under a Craft env!";
                    return;
                }

                qCDebug(CRAFT) << "Found Craft root at" << craftRoot;
                if (currentCraftRuntime) {
                    qCDebug(CRAFT) << "A Craft runtime rooted at" << currentCraftRuntime->craftRoot()
                                   << "is already active. Will not create another one";
                    return;
                }

                auto* runtime = new CraftRuntime(craftRoot, pythonExecutable);
                ICore::self()->runtimeController()->addRuntimes(runtime);
                if (wantAutoEnable(project, craftRoot))
                    ICore::self()->runtimeController()->setCurrentRuntime(runtime);
            });
}

#include "craftplugin.moc"
#include "moc_craftplugin.cpp"
