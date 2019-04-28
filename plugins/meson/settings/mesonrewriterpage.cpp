/* This file is part of KDevelop
    Copyright 2019 Daniel Mensinger <daniel@mensinger-ka.de>

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

#include "mesonrewriterpage.h"
#include "mesonconfig.h"
#include "mesonmanager.h"
#include "mintro/mesonintrospectjob.h"
#include "mintro/mesonprojectinfo.h"
#include "rewriter/mesonkwargsinfo.h"
#include "rewriter/mesonrewriterjob.h"

#include "ui_mesonrewriterpage.h"

#include <interfaces/iplugin.h>
#include <interfaces/iproject.h>
#include <util/executecompositejob.h>

#include <KColorScheme>
#include <QIcon>
#include <algorithm>
#include <debug.h>

using namespace KDevelop;
using namespace std;

class JobDeleter
{
public:
    explicit JobDeleter(QList<KJob*> jobs)
        : m_jobs(jobs)
    {
    }
    ~JobDeleter()
    {
        for (KJob* i : m_jobs) {
            delete i;
        }
    }

private:
    QList<KJob*> m_jobs;
};

MesonRewriterPage::MesonRewriterPage(IPlugin* plugin, IProject* project, QWidget* parent)
    : ConfigPage(plugin, nullptr, parent)
    , m_project(project)
{
    Q_ASSERT(m_project);

    m_ui = new Ui::MesonRewriterPage;
    m_ui->setupUi(this);

    int maxWidth = 50;
    m_projectKwargs = { m_ui->w_version, m_ui->w_license, m_ui->w_mesonVersion };

    for (auto* i : m_projectKwargs) {
        maxWidth = std::max(maxWidth, i->nameWidth());
    }

    for (auto* i : m_projectKwargs) {
        i->setMinNameWidth(maxWidth);
        connect(i, &MesonRewriterInputBase::configChanged, this, &MesonRewriterPage::emitChanged);
    }
    m_ui->l_dispProject->setMinimumWidth(maxWidth);

    reset();
}

void MesonRewriterPage::setWidgetsDisabled(bool disabled)
{
    m_ui->c_tabs->setDisabled(disabled);
}

void MesonRewriterPage::checkStatus()
{
    // Get the config build dir status
    auto setStatus = [this](QString const& msg, int color) -> void {
        KColorScheme scheme(QPalette::Normal);
        KColorScheme::ForegroundRole role;
        switch (color) {
        case 0:
            role = KColorScheme::PositiveText;
            setDisabled(false);
            break;
        case 1:
            role = KColorScheme::NeutralText;
            setDisabled(true);
            break;
        case 2:
        default:
            role = KColorScheme::NegativeText;
            setDisabled(true);
            break;
        }

        QPalette pal = m_ui->l_status->palette();
        pal.setColor(QPalette::Foreground, scheme.foreground(role).color());
        m_ui->l_status->setPalette(pal);
        m_ui->l_status->setText(i18n("Status: %1", msg));
    };

    switch (m_state) {
    case START:
        setStatus(i18n("Initializing GUI"), 1);
        break;
    case LOADING:
        setStatus(i18n("Loading project data..."), 1);
        break;
    case WRITING:
        setStatus(i18n("Writing project data..."), 1);
        break;
    case READY:
        setStatus(i18n("Initializing GUI"), 0);
        break;
    case ERROR:
        setStatus(i18n("Loading meson rewriter data failed"), 2);
        break;
    }

    KColorScheme scheme(QPalette::Normal);
    KColorScheme::ForegroundRole role;
    int numChanged = 0;

    numChanged += count_if(begin(m_projectKwargs), end(m_projectKwargs), [](auto* x) { return x->hasChanged(); });

    if (numChanged == 0) {
        role = KColorScheme::NormalText;
        m_ui->l_changed->setText(i18n("No changes"));
    } else {
        role = KColorScheme::NeutralText;
        m_ui->l_changed->setText(i18n("%1 options changed", numChanged));
    }

    QPalette pal = m_ui->l_changed->palette();
    pal.setColor(QPalette::Foreground, scheme.foreground(role).color());
    m_ui->l_changed->setPalette(pal);
}

void MesonRewriterPage::setStatus(MesonRewriterPage::State s)
{
    m_state = s;
    checkStatus();
}

void MesonRewriterPage::apply()
{
    qCDebug(KDEV_Meson) << "REWRITER GUI: APPLY";
}

void MesonRewriterPage::reset()
{
    qCDebug(KDEV_Meson) << "REWRITER GUI: RESET";

    Meson::BuildDir buildDir = Meson::currentBuildDir(m_project);
    if (!buildDir.isValid()) {
        setStatus(ERROR);
        return;
    }

    auto projectInfo = std::make_shared<MesonKWARGSProjectInfo>();

    QVector<MesonRewriterActionPtr> actions = { projectInfo };

    QVector<MesonIntrospectJob::Type> types = { MesonIntrospectJob::PROJECTINFO };
    MesonIntrospectJob::Mode mode = MesonIntrospectJob::MESON_FILE;

    auto introspectJob = new MesonIntrospectJob(m_project, buildDir, types, mode, this);
    auto rewriterJob = new MesonRewriterJob(m_project, actions, this);

    QList<KJob*> jobs = { introspectJob, rewriterJob };

    // Don't automatically delete jobs beause they are used in the lambda below
    for (KJob* i : jobs) {
        i->setAutoDelete(false);
    }

    KJob* job = new ExecuteCompositeJob(this, jobs);

    connect(job, &KJob::result, this, [=]() -> void {
        JobDeleter deleter(jobs); // Make sure to free all jobs with RAII

        auto prInfo = introspectJob->projectInfo();
        if (!prInfo) {
            setStatus(ERROR);
            return;
        }

        m_ui->l_project->setText(QStringLiteral("<html><head/><body><h3>") + prInfo->name()
                                 + QStringLiteral("</h3></body></html>"));
        m_ui->w_version->resetWidget(projectInfo->version());
        m_ui->w_license->resetWidget(projectInfo->license());
        m_ui->w_mesonVersion->resetWidget(projectInfo->mesonVersion());

        setStatus(READY);
        return;
    });

    setStatus(LOADING);
    job->start();
}

void MesonRewriterPage::defaults()
{
    reset();
}

void MesonRewriterPage::emitChanged()
{
    m_configChanged = true;
    checkStatus();
    emit changed();
}

QString MesonRewriterPage::name() const
{
    return i18n("Project");
}

QString MesonRewriterPage::fullName() const
{
    return i18n("Meson project settings");
}

QIcon MesonRewriterPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("meson"));
}
