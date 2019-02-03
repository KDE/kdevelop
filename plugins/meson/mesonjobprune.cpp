/* This file is part of KDevelop
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "mesonjobprune.h"

#include "mesonbuilder.h"
#include "mesonconfig.h"

#include <outputview/outputmodel.h>

#include <KIO/DeleteJob>
#include <KLocalizedString>

#include <QDir>

using namespace KDevelop;

MesonJobPrune::MesonJobPrune(Meson::BuildDir const& buildDir, QObject* parent)
    : OutputJob(parent, Verbose)
    , m_buildDir(buildDir.buildDir)
    , m_backend(buildDir.mesonBackend)
{
    setCapabilities(Killable);
    setToolTitle(i18n("Meson"));
    setStandardToolView(KDevelop::IOutputView::BuildView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);
}

void MesonJobPrune::start()
{
    auto* output = new OutputModel(this);
    setModel(output);
    startOutput();

    auto status = MesonBuilder::evaluateBuildDirectory(m_buildDir, m_backend);

    switch (status) {
    case MesonBuilder::DOES_NOT_EXIST:
    case MesonBuilder::CLEAN:
        output->appendLine(i18n("The directory '%1' is already pruned", m_buildDir.toLocalFile()));
        emitResult();
        return;
    case MesonBuilder::DIR_NOT_EMPTY:
    case MesonBuilder::INVALID_BUILD_DIR:
        output->appendLine(
            i18n("The directory '%1' does not appear to be a meson build directory", m_buildDir.toLocalFile()));
        output->appendLine(i18n("Aborting prune operation"));
        emitResult();
        return;
    case MesonBuilder::EMPTY_STRING:
        output->appendLine(
            i18n("The current build configuration is broken, because the build directory is not specified"));
        emitResult();
        return;
    default:
        break;
    }

    QDir d(m_buildDir.toLocalFile());
    QList<QUrl> urls;
    const auto entries = d.entryList(QDir::NoDotAndDotDot | QDir::Hidden | QDir::AllEntries);
    urls.reserve(entries.size());
    for (const auto& entry : entries) {
        urls << Path(m_buildDir, entry).toUrl();
    }

    output->appendLine(i18n("Deleting contents of '%1'", m_buildDir.toLocalFile()));
    m_job = KIO::del(urls);
    m_job->start();

    connect(m_job, &KJob::finished, this, [this, output](KJob* job) {
        if (job->error() == 0) {
            output->appendLine(i18n("** Prune successful **"));
        } else {
            output->appendLine(i18n("** Prune failed: %1 **", job->errorString()));
        }
        emitResult();
        m_job = nullptr;
    });
}

bool MesonJobPrune::doKill()
{
    return m_job->kill();
}
