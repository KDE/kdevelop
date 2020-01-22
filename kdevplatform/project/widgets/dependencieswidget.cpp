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

#include "dependencieswidget.h"
#include "ui_dependencieswidget.h"
#include <project/projectmodel.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <util/kdevstringhandler.h>

using namespace KDevelop;

static KDevelop::ProjectBaseItem* itemForPath(const QStringList& path, KDevelop::ProjectModel* model)
{
    return model->itemFromIndex(model->pathToIndex(path));
}

DependenciesWidget::DependenciesWidget(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::DependenciesWidget)
{
    m_ui->setupUi(this);

    connect( m_ui->addDependency, &QPushButton::clicked, this, &DependenciesWidget::addDep );
    connect( m_ui->addDependency, &QPushButton::clicked, this, &DependenciesWidget::changed );
    connect( m_ui->removeDependency, &QPushButton::clicked, this, &DependenciesWidget::changed );
    connect( m_ui->removeDependency, &QPushButton::clicked, this, &DependenciesWidget::removeDep );
    connect( m_ui->moveDepDown, &QPushButton::clicked, this, &DependenciesWidget::changed );
    connect( m_ui->moveDepUp, &QPushButton::clicked, this, &DependenciesWidget::changed );
    connect( m_ui->moveDepDown, &QPushButton::clicked, this, &DependenciesWidget::moveDependencyDown );
    connect( m_ui->moveDepUp, &QPushButton::clicked, this, &DependenciesWidget::moveDependencyUp );
    connect( m_ui->dependencies->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DependenciesWidget::checkActions );
    connect( m_ui->targetDependency, &ProjectItemLineEdit::textChanged, this, &DependenciesWidget::depEdited);
    connect( m_ui->browseProject, &QPushButton::clicked, this, &DependenciesWidget::selectItemDialog);
}

DependenciesWidget::~DependenciesWidget()
{
    delete m_ui;
}

void DependenciesWidget::setSuggestion(KDevelop::IProject* project)
{
    m_ui->targetDependency->setSuggestion(project);
}

void DependenciesWidget::depEdited( const QString& str )
{
    int pos;
    QString tmp = str;
    m_ui->addDependency->setEnabled( !str.isEmpty()
                               && ( !m_ui->targetDependency->validator()
                               || m_ui->targetDependency->validator()->validate( tmp, pos ) == QValidator::Acceptable ) );
}

void DependenciesWidget::checkActions( const QItemSelection& selected, const QItemSelection& unselected )
{
    Q_UNUSED( unselected );
    if( !selected.indexes().isEmpty() )
    {
        Q_ASSERT( selected.indexes().count() == 1 );
        QModelIndex idx = selected.indexes().at( 0 );
        m_ui->moveDepUp->setEnabled( idx.row() > 0 );
        m_ui->moveDepDown->setEnabled( idx.row() < m_ui->dependencies->count() - 1 );
        m_ui->removeDependency->setEnabled( true );
    } else
    {
        m_ui->removeDependency->setEnabled( false );
        m_ui->moveDepUp->setEnabled( false );
        m_ui->moveDepDown->setEnabled( false );
    }
}

void DependenciesWidget::moveDependencyDown()
{
    QList<QListWidgetItem*> list = m_ui->dependencies->selectedItems();
    if( !list.isEmpty() )
    {
        Q_ASSERT( list.count() == 1 );
        QListWidgetItem* item = list.at( 0 );
        int row = m_ui->dependencies->row( item );
        m_ui->dependencies->takeItem( row );
        m_ui->dependencies->insertItem( row+1, item );
        m_ui->dependencies->selectionModel()->select( m_ui->dependencies->model()->index( row+1, 0, QModelIndex() ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::SelectCurrent );
    }
}

void DependenciesWidget::moveDependencyUp()
{

    QList<QListWidgetItem*> list = m_ui->dependencies->selectedItems();
    if( !list.isEmpty() )
    {
        Q_ASSERT( list.count() == 1 );
        QListWidgetItem* item = list.at( 0 );
        int row = m_ui->dependencies->row( item );
        m_ui->dependencies->takeItem( row );
        m_ui->dependencies->insertItem( row-1, item );
        m_ui->dependencies->selectionModel()->select( m_ui->dependencies->model()->index( row-1, 0, QModelIndex() ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::SelectCurrent );
    }
}

void DependenciesWidget::addDep()
{
    QIcon icon;
    KDevelop::ProjectBaseItem* pitem = m_ui->targetDependency->currentItem();
    if(pitem)
        icon = QIcon::fromTheme(pitem->iconName());

    auto* item = new QListWidgetItem(icon, m_ui->targetDependency->text(), m_ui->dependencies);
    item->setData( Qt::UserRole, m_ui->targetDependency->itemPath() );
    m_ui->targetDependency->clear();
    m_ui->addDependency->setEnabled( false );
    m_ui->dependencies->selectionModel()->clearSelection();
    item->setSelected(true);
//     dependencies->selectionModel()->select( dependencies->model()->index( dependencies->model()->rowCount() - 1, 0, QModelIndex() ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::SelectCurrent );
}

void DependenciesWidget::selectItemDialog()
{
    if(m_ui->targetDependency->selectItemDialog()) {
        addDep();
    }
}

void DependenciesWidget::removeDep()
{
    QList<QListWidgetItem*> list = m_ui->dependencies->selectedItems();
    if( !list.isEmpty() )
    {
        Q_ASSERT( list.count() == 1 );
        int row = m_ui->dependencies->row( list.at(0) );
        delete m_ui->dependencies->takeItem( row );

        m_ui->dependencies->selectionModel()->select( m_ui->dependencies->model()->index( row - 1, 0, QModelIndex() ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::SelectCurrent );
    }
}

void DependenciesWidget::setDependencies(const QVariantList &deps)
{
    for (const QVariant& dep : deps) {
        QStringList deplist = dep.toStringList();
        KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
        KDevelop::ProjectBaseItem* pitem=itemForPath(deplist, model);
        QIcon icon;
        if(pitem)
            icon=QIcon::fromTheme(pitem->iconName());

        auto* item = new QListWidgetItem(icon, KDevelop::joinWithEscaping(deplist, QLatin1Char('/'), QLatin1Char('\\')), m_ui->dependencies);
        item->setData( Qt::UserRole, dep );
    }
}

QVariantList DependenciesWidget::dependencies() const
{
    const int dependenciesCount = m_ui->dependencies->count();
    QVariantList deps;
    deps.reserve(dependenciesCount);
    for (int i = 0; i < dependenciesCount; ++i) {
        deps << m_ui->dependencies->item( i )->data( Qt::UserRole );
    }
    return deps;
}
