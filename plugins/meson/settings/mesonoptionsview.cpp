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

#include "mesonoptionsview.h"

#include "mesonconfig.h"
#include "mesonintrospectjob.h"
#include "mesonoptionbaseview.h"
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

KJob* MesonOptionsView::repopulateFromBuildDir(KDevelop::IProject* project, Meson::BuildDir const& buildDir)
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
            OPT_VIEW_PTR opt = nullptr;
            switch (i->type()) {
            case MesonOptionBase::ARRAY:
                opt = make_shared<MesonOptionArrayView>(i, m_ui->tabWidget);
                break;
            case MesonOptionBase::BOOLEAN:
                opt = make_shared<MesonOptionBoolView>(i, m_ui->tabWidget);
                break;
            case MesonOptionBase::COMBO:
                opt = make_shared<MesonOptionComboView>(i, m_ui->tabWidget);
                break;
            case MesonOptionBase::INTEGER:
                opt = make_shared<MesonOptionIntegerView>(i, m_ui->tabWidget);
                break;
            case MesonOptionBase::STRING:
                opt = make_shared<MesonOptionStringView>(i, m_ui->tabWidget);
                break;
            }

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
