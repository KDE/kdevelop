/*************************************************************************************
 *  Copyright (C) 2016 by Carlos Nihelton <carlosnsoliveira@gmail.com>               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#ifndef CLANGTIDYGENERICCONFIGPAGE_H_
#define CLANGTIDYGENERICCONFIGPAGE_H_

#include <QItemSelectionModel>
#include <QObject>
#include <QStringListModel>
#include <interfaces/configpage.h>

#include "configgroup.h"

class QIcon;
class QStringListModel;

namespace KDevelop
{
class IProject;
}

namespace ClangTidy
{
class Plugin;

namespace Ui
{
    class GenericConfig;
}

class GenericConfigPage : public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    GenericConfigPage(KDevelop::IProject* project, QWidget* parent);
    ~GenericConfigPage() override;

    QString name() const override;
    void setList(QStringList list);
    void setActiveChecksReceptorList(QStringList* list);

signals:

public slots:
    void apply() override;
    void defaults() override;
    void reset() override;

private:
    KDevelop::IProject* m_project;
    Ui::GenericConfig* ui;
    QStringList* m_activeChecksReceptor;
    QStringList m_underlineAvailChecks;
    QStringListModel* m_availableChecksModel;
    QItemSelectionModel* m_selectedItemModel;
};
}

#endif /* CLANGTIDYGENERICCONFIGPAGE_H_ */
