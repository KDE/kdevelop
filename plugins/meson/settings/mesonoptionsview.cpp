/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mesonoptionsview.h"

#include "mesonconfig.h"
#include "mesonoptionbaseview.h"
#include "mintro/mesonintrospectjob.h"
#include "ui_mesonoptionsview.h"
#include <debug.h>

#include <interfaces/iproject.h>

#include <algorithm>

using namespace std;

MesonOptionsView::MesonOptionsView(QWidget* parent)
    : QWidget(parent)
{
    m_ui = new Ui::MesonOptionsView;
    m_ui->setupUi(this);
    setDisabled(true);
}

MesonOptionsView::~MesonOptionsView()
{
    m_optViews.clear();
    if (m_ui) {
        delete m_ui;
    }
}

void MesonOptionsView::clear()
{
    setDisabled(true);
    m_optViews.clear();
}

void MesonOptionsView::resetAll()
{
    for (auto& i : m_optViews) {
        i->reset();
    }
}

KJob* MesonOptionsView::repopulateFromBuildDir(KDevelop::IProject* project, const Meson::BuildDir& buildDir)
{
    return repopulate(new MesonIntrospectJob(project, buildDir, { MesonIntrospectJob::BUILDOPTIONS },
                                             MesonIntrospectJob::BUILD_DIR, this));
}

KJob* MesonOptionsView::repopulateFromMesonFile(KDevelop::IProject* project, KDevelop::Path mesonExe)
{
    return repopulate(new MesonIntrospectJob(project, mesonExe, { MesonIntrospectJob::BUILDOPTIONS }, this));
}

KJob* MesonOptionsView::repopulate(MesonIntrospectJob* introJob)
{
    setDisabled(true);

    connect(introJob, &KJob::result, this, [this, introJob]() {
        m_optViews.clear();
        m_options = introJob->buildOptions();
        if (!m_options) {
            qCWarning(KDEV_Meson) << "Failed to get introspection data";
            return;
        }

        for (auto i : m_options->options()) {
            OPT_VIEW_PTR opt = MesonOptionBaseView::fromOption(i, m_ui->tabWidget);

            if (!opt) {
                qCWarning(KDEV_Meson) << "Unhandled option type " << i->type();
                continue;
            }

            QVBoxLayout* layout = nullptr;

            switch (i->section()) {
            case MesonOptionBase::CORE:
                layout = m_ui->l_core;
                break;
            case MesonOptionBase::BACKEND:
                layout = m_ui->l_backend;
                break;
            case MesonOptionBase::BASE:
                layout = m_ui->l_base;
                break;
            case MesonOptionBase::COMPILER:
                layout = m_ui->l_compiler;
                break;
            case MesonOptionBase::DIRECTORY:
                layout = m_ui->l_directory;
                break;
            case MesonOptionBase::USER:
                layout = m_ui->l_user;
                break;
            case MesonOptionBase::TEST:
                layout = m_ui->l_test;
                break;
            }

            if (!layout) {
                qCWarning(KDEV_Meson) << "Unknown section " << i->section();
            }

            connect(opt.get(), &MesonOptionBaseView::configChanged, this, &MesonOptionsView::emitChanged);

            // Insert at count() - 1 to keep the stretch at the bottom
            layout->insertWidget(layout->count() - 1, opt.get());
            m_optViews << opt;
        }

        auto maxEl = max_element(begin(m_optViews), end(m_optViews),
                                 [](auto a, auto b) { return a->nameWidth() < b->nameWidth(); });
        int maxWidth = (**maxEl).nameWidth();
        for_each(begin(m_optViews), end(m_optViews), [maxWidth](auto a) { a->setMinNameWidth(maxWidth); });

        setDisabled(false);
    });

    return introJob;
}

void MesonOptionsView::emitChanged()
{
    emit configChanged();
}

MesonOptsPtr MesonOptionsView::options()
{
    return m_options;
}
