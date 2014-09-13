/*
 * Copyright <year> Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KDEVPLATFORM_PLUGIN_BENCH_QUICKOPEN_H
#define KDEVPLATFORM_PLUGIN_BENCH_QUICKOPEN_H

#include "quickopentestbase.h"

class BenchQuickOpen : public QuickOpenTestBase
{
    Q_OBJECT
public:
    explicit BenchQuickOpen(QObject* parent = 0);
private:
    void getData();
private slots:
    void benchProjectFileFilter_addRemoveProject();
    void benchProjectFileFilter_addRemoveProject_data();
    void benchProjectFileFilter_reset();
    void benchProjectFileFilter_reset_data();
    void benchProjectFileFilter_setFilter();
    void benchProjectFileFilter_setFilter_data();
    void benchProjectFileFilter_providerData();
    void benchProjectFileFilter_providerData_data();
    void benchProjectFileFilter_providerDataIcon();
    void benchProjectFileFilter_providerDataIcon_data();
};

#endif // KDEVPLATFORM_PLUGIN_BENCH_QUICKOPEN_H
