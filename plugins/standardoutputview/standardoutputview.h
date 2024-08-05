/*
    SPDX-FileCopyrightText: 2006-2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_STANDARDOUTPUTVIEW_H
#define KDEVPLATFORM_PLUGIN_STANDARDOUTPUTVIEW_H

#include <outputview/ioutputview.h>
#include <interfaces/iplugin.h>
#include <QVariantList>

template <typename T> class QList;
class QAbstractItemModel;
class QString;
class QModelIndex;
class QAbstractItemDelegate;
class OutputWidget;
class ToolViewData;

/**
@author Andreas Pakulat
*/

namespace Sublime
{
class View;
}

class StandardOutputView : public KDevelop::IPlugin, public KDevelop::IOutputView
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IOutputView )

public:
    explicit StandardOutputView(QObject* parent, const KPluginMetaData& metaData,
                                const QVariantList& args = QVariantList());
    ~StandardOutputView() override;

    int standardToolView( KDevelop::IOutputView::StandardToolView view ) override;
    int registerToolView(const QString& configSubgroupName, const QString& title,
                         KDevelop::IOutputView::ViewType type = KDevelop::IOutputView::OneView,
                         const QIcon& icon = QIcon(), KDevelop::IOutputView::Options option = ShowItemsButton,
                         const QList<QAction*>& actionList = QList<QAction*>()) override;

    int registerOutputInToolView( int toolViewId, const QString& title,
                                  KDevelop::IOutputView::Behaviours behaviour
                                    = KDevelop::IOutputView::AllowUserClose ) override;

    void raiseOutput( int id ) override;

    void setModel( int outputId, QAbstractItemModel* model ) override;
    void setDelegate( int outputId, QAbstractItemDelegate* delegate ) override;

    OutputWidget* outputWidgetForId( int outputId ) const;

    void removeToolView(int toolViewId) override;
    void removeOutput( int outputId ) override;

    void scrollOutputTo( int outputId, const QModelIndex& idx ) override;
    void setTitle(int outputId, const QString& title) override;

public Q_SLOTS:
    void removeSublimeView( Sublime::View* );

Q_SIGNALS:
    void activated( const QModelIndex& );
    void outputRemoved(int toolViewId, int outputId);
    void toolViewRemoved(int toolViewId);

private:
    QMap<int, ToolViewData*> m_toolViews;
    QList<int> m_ids;
    QMap<KDevelop::IOutputView::StandardToolView, int> m_standardViews;
};

#endif // KDEVPLATFORM_PLUGIN_STANDARDOUTPUTVIEW_H

