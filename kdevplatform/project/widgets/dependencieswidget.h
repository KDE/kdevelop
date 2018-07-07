/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>
    Copyright 2009 Niko Sams <niko.sams@gmail.com>

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

#ifndef DEPENDENCIESWIDGET_H
#define DEPENDENCIESWIDGET_H

#include <QWidget>
#include <QItemSelection>
#include <project/projectexport.h>

namespace Ui { class DependenciesWidget; }
namespace KDevelop {
class IProject;

class KDEVPLATFORMPROJECT_EXPORT DependenciesWidget : public QWidget
{
    Q_OBJECT
    public:
        explicit DependenciesWidget(QWidget* parent);
        ~DependenciesWidget() override;

        void setSuggestion(KDevelop::IProject* project);

        void setDependencies(const QVariantList &deps);
        QVariantList dependencies() const;

    Q_SIGNALS:
        void changed();

    private:
        void depEdited( const QString& str );
        void checkActions( const QItemSelection& selected, const QItemSelection& unselected );
        void moveDependencyDown();
        void moveDependencyUp();
        void addDep();
        void removeDep();
        void selectItemDialog();

        Ui::DependenciesWidget* m_ui;
};

}

#endif
