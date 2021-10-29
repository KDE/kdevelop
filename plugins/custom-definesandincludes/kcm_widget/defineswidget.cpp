/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#include "defineswidget.h"

#include <KLocalizedString>
#include <QAction>

#include "../ui_defineswidget.h"
#include "definesmodel.h"
#include <debug.h>

using namespace KDevelop;

DefinesWidget::DefinesWidget( QWidget* parent )
    : QWidget ( parent ), ui( new Ui::DefinesWidget )
    , definesModel( new DefinesModel( this ) )
{
    ui->setupUi( this );
    ui->defines->setModel( definesModel );
    ui->defines->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
    connect(definesModel, &DefinesModel::dataChanged, this, QOverload<>::of(&DefinesWidget::definesChanged));
    connect(definesModel, &DefinesModel::rowsInserted, this, QOverload<>::of(&DefinesWidget::definesChanged));
    connect(definesModel, &DefinesModel::rowsRemoved, this, QOverload<>::of(&DefinesWidget::definesChanged));

    auto* delDefAction = new QAction(i18nc("@action", "Delete Define"), this);
    delDefAction->setShortcut( QKeySequence(Qt::Key_Delete) );
    delDefAction->setShortcutContext( Qt::WidgetWithChildrenShortcut );
    delDefAction->setIcon( QIcon::fromTheme(QStringLiteral("edit-delete")) );
    ui->defines->addAction( delDefAction );
    ui->defines->setContextMenuPolicy( Qt::ActionsContextMenu );
    connect( delDefAction, &QAction::triggered, this, &DefinesWidget::deleteDefine );
}

DefinesWidget::~DefinesWidget()
{
}

void DefinesWidget::setDefines( const Defines& defines )
{
    bool b = blockSignals( true );
    clear();
    definesModel->setDefines( defines );
    blockSignals( b );
}

void DefinesWidget::definesChanged()
{
    qCDebug(DEFINESANDINCLUDES) << "defines changed";
    emit definesChanged( definesModel->defines() );
}

void DefinesWidget::clear()
{
    definesModel->setDefines( {} );
}

void DefinesWidget::deleteDefine()
{
    qCDebug(DEFINESANDINCLUDES) << "Deleting defines";
    const QModelIndexList selection = ui->defines->selectionModel()->selectedRows();
    for (const QModelIndex& row : selection) {
        definesModel->removeRow( row.row() );
    }
}

