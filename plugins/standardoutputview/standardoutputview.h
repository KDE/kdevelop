/*
    SPDX-FileCopyrightText: 2006-2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_STANDARDOUTPUTVIEW_H
#define KDEVPLATFORM_PLUGIN_STANDARDOUTPUTVIEW_H

#include <outputview/ioutputview.h>
#include <interfaces/iplugin.h>

#include <QHash>
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

    QString standardToolView(StandardToolView view) override;

    void registerToolView(const QString& toolViewId, const QString& title,
                          KDevelop::IOutputView::ViewType type = KDevelop::IOutputView::OneView,
                          const QIcon& icon = QIcon(), KDevelop::IOutputView::Options option = ShowItemsButton,
                          const QList<QAction*>& actionList = QList<QAction*>()) override;

    [[nodiscard]] int registerOutputInToolView(const QString& toolViewId, const QString& title,
                                               Behaviours behaviour = AllowUserClose) override;

    void raiseOutput( int id ) override;

    void setModel( int outputId, QAbstractItemModel* model ) override;
    void setDelegate( int outputId, QAbstractItemDelegate* delegate ) override;

    OutputWidget* outputWidgetForId( int outputId ) const;

    void removeToolView(const QString& toolViewId) override;
    void removeOutput( int outputId ) override;

    void setTitle(int outputId, const QString& title) override;

Q_SIGNALS:
    void activated( const QModelIndex& );

    /**
     * @copydoc KDevelop::IOutputView::outputRemoved()
     *
     * @note This signal hides the base class's "signal" KDevelop::IOutputView::outputRemoved().
     *       The hiding is necessary, because KDevelop::IOutputView does not inherit QObject,
     *       and thus cannot declare an actually working signal.
     */
    void outputRemoved(int outputId);

private:
    [[nodiscard]] ToolViewData* addToolView(const QString& toolViewId, const QString& title, ViewType type,
                                            const QIcon& icon, Options option, const QList<QAction*>& actionList = {});

    QHash<QString, ToolViewData*> m_toolViews;
    int m_lastId = 0;
};

#endif // KDEVPLATFORM_PLUGIN_STANDARDOUTPUTVIEW_H

