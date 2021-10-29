/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PROBLEMTREEVIEW_H
#define KDEVPLATFORM_PLUGIN_PROBLEMTREEVIEW_H

#include <QTreeView>

namespace KDevelop
{
class TopDUContext;
class IDocument;
class ProblemModel;
}

class ProblemReporterPlugin;
class QSortFilterProxyModel;

class ProblemTreeView : public QTreeView
{
    Q_OBJECT

public:
    ProblemTreeView(QWidget* parent, QAbstractItemModel* itemModel);
    ~ProblemTreeView() override;

    KDevelop::ProblemModel* model() const;
    void setModel(QAbstractItemModel* model) override;

    void contextMenuEvent(QContextMenuEvent*) override;
    void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                             const QVector<int>& roles = QVector<int>()) override;
    void reset() override;

    int setFilter(const QString& filterText);

public Q_SLOTS:
    void openDocumentForCurrentProblem();

Q_SIGNALS:
    // Emitted when the model's rows change (added/removed/reset)
    void changed();

protected:
    void showEvent(QShowEvent* event) override;

private Q_SLOTS:
    void itemActivated(const QModelIndex& index);

private:
    void resizeColumns();
    ProblemReporterPlugin* m_plugin;
    QSortFilterProxyModel* m_proxy;
};

#endif
