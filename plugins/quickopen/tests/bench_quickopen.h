/*
    SPDX-FileCopyrightText: Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_PLUGIN_BENCH_QUICKOPEN_H
#define KDEVPLATFORM_PLUGIN_BENCH_QUICKOPEN_H

#include "quickopentestbase.h"

class BenchQuickOpen
    : public QuickOpenTestBase
{
    Q_OBJECT
public:
    explicit BenchQuickOpen(QObject* parent = nullptr);
private:
    void getData();
    void getAddRemoveData();
private Q_SLOTS:
    void benchProjectFile_swap();
    void benchProjectFileFilter_addRemoveProject();
    void benchProjectFileFilter_addRemoveProject_data();
    void benchProjectFileFilter_addRemoveProjects();
    void benchProjectFileFilter_addRemoveProjects_data();
    void benchProjectFileFilter_reset();
    void benchProjectFileFilter_reset_data();
    void benchProjectFileFilter_setFilter();
    void benchProjectFileFilter_setFilter_data();
    void benchProjectFileFilter_providerData();
    void benchProjectFileFilter_providerData_data();
    void benchProjectFileFilter_providerDataIcon();
    void benchProjectFileFilter_providerDataIcon_data();
    void benchProjectFileFilter_files();
    void benchProjectFileFilter_files_data();
    void benchProjectFileFilter_fileRemovedFromSet_data();
    void benchProjectFileFilter_fileRemovedFromSet();
};

#endif // KDEVPLATFORM_PLUGIN_BENCH_QUICKOPEN_H
