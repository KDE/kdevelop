/* This file is part of KDevelop
   Copyright 2017 Anton Anikin <anton.anikin@htower.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "visualizer.h"

#include "debug.h"
#include "globalsettings.h"
#include "utils.h"

#include <util/path.h>

#include <KLocalizedString>
#include <KMessageBox>

#include <QFile>

namespace Heaptrack
{

Visualizer::Visualizer(const QString& resultsFile, QObject* parent)
    : QProcess(parent)
    , m_resultsFile(resultsFile)
{
#if QT_VERSION < 0x050600
    connect(this, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
#else
    connect(this, &QProcess::errorOccurred,
#endif
            this, [this](QProcess::ProcessError error) {
        QString errorMessage;
        if (error == QProcess::FailedToStart) {
            errorMessage = i18n("Failed to start visualizer from \"%1\".", program())
                           + QStringLiteral("\n\n")
                           + i18n("Check your settings and install the visualizer if necessary.");
        } else {
            errorMessage = i18n("Error during visualizer execution:")
                           + QStringLiteral("\n\n")
                           + errorString();
        }
        KMessageBox::error(activeMainWindow(), errorMessage, i18n("Heaptrack Error"));
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
