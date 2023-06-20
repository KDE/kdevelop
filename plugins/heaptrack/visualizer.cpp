/*
    SPDX-FileCopyrightText: 2017 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "visualizer.h"

#include "debug.h"
#include "globalsettings.h"
#include "utils.h"
// KDevPlatform
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <sublime/message.h>
#include <util/path.h>
// KF
#include <KLocalizedString>
// Qt
#include <QFile>

namespace Heaptrack
{

Visualizer::Visualizer(const QString& resultsFile, QObject* parent)
    : QProcess(parent)
    , m_resultsFile(resultsFile)
{
    connect(this, &QProcess::errorOccurred,
            this, [this](QProcess::ProcessError error) {
        QString errorMessage;
        if (error == QProcess::FailedToStart) {
            errorMessage = i18n("Failed to start Heaptrack visualizer from \"%1\".", program())
                           + QLatin1String("\n\n")
                           + i18n("Check your settings and install the visualizer if necessary.");
        } else {
            errorMessage = i18n("Error during Heaptrack visualizer execution:")
                           + QLatin1String("\n\n")
                           + errorString();
        }
        auto* message = new Sublime::Message(errorMessage, Sublime::Message::Error);
        KDevelop::ICore::self()->uiController()->postMessage(message);
    });

    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this]() {
                deleteLater();
            });

    setProgram(KDevelop::Path(GlobalSettings::heaptrackGuiExecutable()).toLocalFile());
    setArguments({ resultsFile });
}

Visualizer::~Visualizer()
{
    QFile::remove(m_resultsFile);
}

}

#include "moc_visualizer.cpp"
