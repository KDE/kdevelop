/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
