/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mesonrewriterpage.h"
#include "mesonconfig.h"
#include "mesonmanager.h"
#include "mesonoptionbaseview.h"
#include "mesonrewriterinput.h"
#include "mintro/mesonintrospectjob.h"
#include "mintro/mesonprojectinfo.h"
#include "rewriter/mesondefaultopts.h"
#include "rewriter/mesonkwargsinfo.h"
#include "rewriter/mesonkwargsmodify.h"
#include "rewriter/mesonrewriterjob.h"

#include "ui_mesonrewriterpage.h"

#include <interfaces/iplugin.h>
#include <interfaces/iproject.h>
#include <util/executecompositejob.h>

#include <KColorScheme>
#include <QIcon>
#include <QInputDialog>
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
            i->deleteLater();
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

    m_projectKwargs = constructPojectInputs();

    // Initialize widgets
    for (auto* i : m_projectKwargs) {
        m_ui->c_project->addWidget(i);
        connect(i, &MesonRewriterInputBase::configChanged, this, &MesonRewriterPage::emitChanged);
    }

    recalculateLengths();
    reset();
}

#define STRING_INPUT(name, id) new MesonRewriterInputString(QStringLiteral(name), QStringLiteral(id), this)

QVector<MesonRewriterInputBase*> MesonRewriterPage::constructPojectInputs()
{
    return {
        STRING_INPUT("Version", "version"),
        STRING_INPUT("License", "license"),
        STRING_INPUT("Meson version", "meson_version"),
        STRING_INPUT("Subprojects directory", "subproject_dir"),
    };
}

MesonOptContainerPtr MesonRewriterPage::constructDefaultOpt(const QString& name, const QString& value)
{
    if (!m_opts) {
        return nullptr;
    }

    for (auto& i : m_opts->options()) {
        if (i->name() != name) {
            continue;
        }

        if (!value.isNull()) {
            i->setFromString(value);
        }

        auto optView = MesonOptionBaseView::fromOption(i, this);
        if (!optView) {
            continue;
        }

        auto opt = std::make_shared<MesonRewriterOptionContainer>(optView, this);
        if (!opt) {
            continue;
        }

        connect(opt.get(), &MesonRewriterOptionContainer::configChanged, this, &MesonRewriterPage::emitChanged);
        return opt;
    }

    return nullptr;
}

void MesonRewriterPage::setWidgetsDisabled(bool disabled)
{
    m_ui->c_tabs->setDisabled(disabled);
}

void MesonRewriterPage::recalculateLengths()
{
    // Calculate the maximum name width to align all widgets
    vector<int> lengths;
    int maxWidth = 50;
    lengths.reserve(m_projectKwargs.size() + m_defaultOpts.size());

    auto input_op = [](MesonRewriterInputBase* x) -> int { return x->nameWidth(); };
    auto optCont_op = [](MesonOptContainerPtr x) -> int { return x->view()->nameWidth(); };

    transform(begin(m_projectKwargs), end(m_projectKwargs), back_inserter(lengths), input_op);
    transform(begin(m_defaultOpts), end(m_defaultOpts), back_inserter(lengths), optCont_op);

    maxWidth = accumulate(begin(lengths), end(lengths), maxWidth, [](int a, int b) -> int { return max(a, b); });

    // Set widgets width
    for (auto* i : m_projectKwargs) {
        i->setMinNameWidth(maxWidth);
    }

    for (auto& i : m_defaultOpts) {
        i->view()->setMinNameWidth(maxWidth);
    }

    m_ui->l_dispProject->setMinimumWidth(maxWidth);
}

void MesonRewriterPage::checkStatus()
{
    // Get the config build dir status
    auto setStatus = [this](const QString& msg, int color) -> void {
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
        pal.setColor(QPalette::WindowText, scheme.foreground(role).color());
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

    // Remove old default options
    m_defaultOpts.erase(remove_if(begin(m_defaultOpts), end(m_defaultOpts), [](auto x) { return x->shouldDelete(); }),
                        end(m_defaultOpts));

    KColorScheme scheme(QPalette::Normal);
    KColorScheme::ForegroundRole role;
    int numChanged = 0;

    numChanged += count_if(begin(m_projectKwargs), end(m_projectKwargs), [](auto* x) { return x->hasChanged(); });
    numChanged += count_if(begin(m_defaultOpts), end(m_defaultOpts), [](auto x) { return x->hasChanged(); });

    if (numChanged == 0) {
        role = KColorScheme::NormalText;
        m_ui->l_changed->setText(i18n("No changes"));
    } else {
        role = KColorScheme::NeutralText;
        m_ui->l_changed->setText(i18np("%1 option changed", "%1 options changed", numChanged));
    }

    QPalette pal = m_ui->l_changed->palette();
    pal.setColor(QPalette::WindowText, scheme.foreground(role).color());
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

    auto projectSet = make_shared<MesonKWARGSProjectModify>(MesonKWARGSProjectModify::SET);
    auto projectDel = make_shared<MesonKWARGSProjectModify>(MesonKWARGSProjectModify::DELETE);
    auto defOptsSet = make_shared<MesonRewriterDefaultOpts>(MesonRewriterDefaultOpts::SET);
    auto defOptsDel = make_shared<MesonRewriterDefaultOpts>(MesonRewriterDefaultOpts::DELETE);

    auto writer = [](MesonRewriterInputBase* widget, MesonKWARGSModifyPtr set, MesonKWARGSModifyPtr del) {
        if (!widget->hasChanged()) {
            return;
        }

        if (widget->isEnabled()) {
            widget->writeToAction(set.get());
        } else {
            widget->writeToAction(del.get());
        }
    };

    for_each(begin(m_projectKwargs), end(m_projectKwargs), [&](auto* w) { writer(w, projectSet, projectDel); });

    QStringList deletedOptions = m_initialDefaultOpts;

    for (auto& i : m_defaultOpts) {
        auto opt = i->view()->option();

        // Detect deleted options by removing all current present options from the initial option list
        deletedOptions.removeAll(opt->name());

        if (opt->isUpdated() || !m_initialDefaultOpts.contains(opt->name())) {
            defOptsSet->set(opt->name(), opt->value());
        }
    }

    for (auto i : deletedOptions) {
        defOptsDel->set(i, QString());
    }

    QVector<MesonRewriterActionPtr> actions = { projectSet, projectDel, defOptsSet, defOptsDel };

    KJob* rewriterJob = new MesonRewriterJob(m_project, actions, this);

    // Reload the GUI once the data has been written
    connect(rewriterJob, &KJob::result, this, &MesonRewriterPage::reset);

    setStatus(WRITING);
    rewriterJob->start();
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

    QVector<MesonIntrospectJob::Type> types = { MesonIntrospectJob::PROJECTINFO, MesonIntrospectJob::BUILDOPTIONS };
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
        m_opts = introspectJob->buildOptions();
        if (!prInfo || !m_opts) {
            setStatus(ERROR);
            return;
        }

        m_ui->l_project->setText(QStringLiteral("<html><head/><body><h3>") + prInfo->name()
                                 + QStringLiteral("</h3></body></html>"));

        auto setter = [](MesonRewriterInputBase* w, MesonKWARGSInfoPtr i) { w->resetFromAction(i.get()); };

        for_each(begin(m_projectKwargs), end(m_projectKwargs), [=](auto* x) { setter(x, projectInfo); });

        // Updated the default options
        m_defaultOpts.clear();
        m_initialDefaultOpts.clear();
        if (projectInfo->hasKWARG(QStringLiteral("default_options"))) {
            auto rawValues = projectInfo->getArray(QStringLiteral("default_options"));
            auto options = m_opts->options();

            for (auto i : rawValues) {
                int idx = i.indexOf(QLatin1Char('='));
                if (idx < 0) {
                    continue;
                }

                QString name = i.left(idx);
                QString val = i.mid(idx + 1);

                auto opt = constructDefaultOpt(name, val);
                if (!opt) {
                    continue;
                }

                m_defaultOpts += opt;
                m_initialDefaultOpts += name;
                m_ui->c_defOpts->addWidget(opt.get());
            }
        }

        recalculateLengths();
        setStatus(READY);
        return;
    });

    setStatus(LOADING);
    job->start();
}

void MesonRewriterPage::newOption()
{
    // Sort by section
    QStringList core;
    QStringList backend;
    QStringList base;
    QStringList compiler;
    QStringList directory;
    QStringList user;
    QStringList test;

    for (auto& i : m_opts->options()) {
        switch (i->section()) {
        case MesonOptionBase::CORE:
            core += i->name();
            break;
        case MesonOptionBase::BACKEND:
            backend += i->name();
            break;
        case MesonOptionBase::BASE:
            base += i->name();
            break;
        case MesonOptionBase::COMPILER:
            compiler += i->name();
            break;
        case MesonOptionBase::DIRECTORY:
            directory += i->name();
            break;
        case MesonOptionBase::USER:
            user += i->name();
            break;
        case MesonOptionBase::TEST:
            test += i->name();
            break;
        }
    }

    QStringList total = core + backend + base + compiler + directory + user + test;

    // Remove already existing options
    for (auto& i : m_defaultOpts) {
        total.removeAll(i->view()->option()->name());
    }

    QInputDialog dialog(this);

    dialog.setOption(QInputDialog::UseListViewForComboBoxItems, true);
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setWindowTitle(i18nc("@title:window", "Select Additional Meson Option"));
    dialog.setLabelText(i18nc("@label:listbox", "Meson option to add:"));
    dialog.setComboBoxItems(total);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    auto opt = constructDefaultOpt(dialog.textValue(), QString());
    if (!opt) {
        return;
    }

    m_defaultOpts += opt;
    m_ui->c_defOpts->addWidget(opt.get());
    recalculateLengths();
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
    return i18nc("@title:tab", "Project");
}

QString MesonRewriterPage::fullName() const
{
    return i18nc("@title:tab", "Meson Project Settings");
}

QIcon MesonRewriterPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("run-build"));
}

#include "moc_mesonrewriterpage.cpp"
