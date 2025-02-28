/*
    SPDX-FileCopyrightText: 2006-2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "standardoutputview.h"
#include "outputwidget.h"
#include "toolviewdata.h"
#include "debug.h"

#include <QAbstractItemDelegate>
#include <QAbstractItemModel>
#include <QAction>
#include <QList>

#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/controller.h>
#include <sublime/document.h>

class OutputViewFactory : public KDevelop::IToolViewFactory{
public:
    explicit OutputViewFactory(const ToolViewData* data)
        : m_data(data)
        , m_id{QLatin1String("org.kdevelop.OutputView.") + m_data->id}
    {
    }
    QWidget* create(QWidget *parent = nullptr) override
    {
        return new OutputWidget( parent, m_data );
    }
    Qt::DockWidgetArea defaultPosition() const override
    {
        return Qt::BottomDockWidgetArea;
    }
    void viewCreated( Sublime::View* view ) override
    {
        m_data->views << view;
    }
    QString id() const override
    {
        return m_id;
    }
private:
    const ToolViewData *m_data;
    const QString m_id; ///< cached factory ID
};

StandardOutputView::StandardOutputView(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevstandardoutputview"), parent, metaData)
{
}

StandardOutputView::~StandardOutputView()
{
}

QString StandardOutputView::standardToolView(StandardToolView view)
{
    /// A convenience wrapper to call QIcon::fromTheme() once per standard tool view and to return the tool view ID.
    const auto ensureRegistered = [this](QString toolViewId, const QString& title, ViewType type,
                                         const QString& iconName, Options option = AddFilterAction) {
        auto& toolView = m_toolViews[toolViewId];
        if (!toolView) {
            toolView = addToolView(toolViewId, title, type, QIcon::fromTheme(iconName), option);
        }
        return toolViewId;
    };

    switch( view )
    {
    case KDevelop::IOutputView::BuildView:
        return ensureRegistered(QStringLiteral("Build"), i18nc("@title:window", "Build"), HistoryView,
                                QStringLiteral("run-build"));
    case KDevelop::IOutputView::RunView:
        return ensureRegistered(QStringLiteral("Run"), i18nc("@title:window", "Run"), MultipleView,
                                QStringLiteral("system-run"));
    case KDevelop::IOutputView::DebugView:
        return ensureRegistered(QStringLiteral("Debug"), i18nc("@title:window", "Debug"), MultipleView,
                                QStringLiteral("debug-step-into"));
    case KDevelop::IOutputView::AnalyzeView:
        return ensureRegistered(QStringLiteral("Analyze"), i18nc("@title:window", "Analyze"), HistoryView,
                                QStringLiteral("dialog-ok"));
    case KDevelop::IOutputView::VcsView:
        return ensureRegistered(QStringLiteral("VersionControl"), i18nc("@title:window", "Version Control"),
                                HistoryView, QStringLiteral("system-run"));
    }
    Q_UNREACHABLE();
}

void StandardOutputView::registerToolView(const QString& toolViewId, const QString& title,
                                          KDevelop::IOutputView::ViewType type, const QIcon& icon, Options option,
                                          const QList<QAction*>& actionList)
{
    if (toolViewId.isEmpty()) {
        qCWarning(PLUGIN_STANDARDOUTPUTVIEW).nospace()
            << "registering a view with an empty ID, title " << title << " and type " << type
            << "; an empty ID is not unique, so expect bugs!";
    }

    auto& toolView = m_toolViews[toolViewId];
    if (toolView) {
        qCDebug(PLUGIN_STANDARDOUTPUTVIEW)
            << "reusing view" << toolViewId << "for title" << title << "and type" << type;
        return;
    }
    toolView = addToolView(toolViewId, title, type, icon, option, actionList);
}

ToolViewData* StandardOutputView::addToolView(const QString& toolViewId, const QString& title, ViewType type,
                                              const QIcon& icon, Options option, const QList<QAction*>& actionList)
{
    qCDebug(PLUGIN_STANDARDOUTPUTVIEW) << "adding view" << toolViewId << "with title" << title << "and type" << type;

    auto* tvdata = new ToolViewData( this );
    tvdata->id = toolViewId;
    tvdata->type = type;
    tvdata->title = title;
    tvdata->icon = icon;
    tvdata->plugin = this;
    tvdata->option = option;
    tvdata->actionList = actionList;

    core()->uiController()->addToolView( title, new OutputViewFactory( tvdata ) );
    return tvdata;
}

int StandardOutputView::registerOutputInToolView(const QString& toolViewId, const QString& title,
                                                 KDevelop::IOutputView::Behaviours behaviour)
{
    const auto toolViewIt = m_toolViews.constFind(toolViewId);
    if (toolViewIt == m_toolViews.constEnd())
        return -1;
    const auto newid = ++m_lastId;
    (*toolViewIt)->addOutput(newid, title, behaviour);
    return newid;
}

void StandardOutputView::raiseOutput(int outputId)
{
    for (const auto* toolViewData : std::as_const(m_toolViews)) {
        if (toolViewData->outputdata.contains(outputId)) {
            for (Sublime::View* v : std::as_const(toolViewData->views)) {
                if (auto* const widget = v->widget()) {
                    auto* const w = qobject_cast<OutputWidget*>(widget);
                    w->raiseOutput( outputId );
                    v->requestRaise();
                }
            }
        }
        // TODO: not break here?
    }
}

void StandardOutputView::setModel( int outputId, QAbstractItemModel* model )
{
    OutputData* outputData = nullptr;
    for (const auto* toolViewData : std::as_const(m_toolViews)) {
        const auto& outputDataMap = toolViewData->outputdata;
        auto outputDataIt = outputDataMap.find(outputId);
        if (outputDataIt != outputDataMap.end()) {
            outputData = outputDataIt.value();
            break;
        }
    }
    if (!outputData) {
        qCDebug(PLUGIN_STANDARDOUTPUTVIEW) << "Trying to set model on unknown view-id:" << outputId;
    } else {
        outputData->setModel(model);
    }
}

void StandardOutputView::setDelegate( int outputId, QAbstractItemDelegate* delegate )
{
    OutputData* outputData = nullptr;
    for (const auto* toolViewData : std::as_const(m_toolViews)) {
        const auto& outputDataMap = toolViewData->outputdata;
        auto outputDataIt = outputDataMap.find(outputId);
        if (outputDataIt != outputDataMap.end()) {
            outputData = outputDataIt.value();
            break;
        }
    }
    if (!outputData) {
        qCDebug(PLUGIN_STANDARDOUTPUTVIEW) << "Trying to set model on unknown view-id:" << outputId;
    } else {
        outputData->setDelegate(delegate);
    }
}

// TODO: only StandardOutputViewTest calls this function, so output tool views are never removed in KDevelop.
//       Calling Sublime::Area::removeToolView() in a loop does not completely remove
//       a tool view, so consider calling IUiController::removeToolView() instead.
//       StandardOutputView::addToolView() calls IUiController::addToolView() but
//       StandardOutputView::unload() does not call IUiController::removeToolView() as all
//       other plugins do. This does not cause a crash when the user attempts to unload
//       the Output View plugin on the Plugins tab of the Configure KDevelop dialog, because
//       the unloading is prevented by the absence of the "X-KDevelop-Category": "Global"
//       entry in kdevstandardoutputview.json (see df99304843fbdb593398e3d7d912e15cbd7c56b9).
void StandardOutputView::removeToolView(const QString& toolViewId)
{
    const auto toolViewIt = m_toolViews.find(toolViewId);
    if (toolViewIt != m_toolViews.end()) {
        ToolViewData* td = *toolViewIt;
        const auto views = td->views;
        for (Sublime::View* view : views) {
            if (auto* const widget = view->widget()) {
                auto* const outputWidget = qobject_cast<OutputWidget*>(widget);
                for (auto it = td->outputdata.keyBegin(), end = td->outputdata.keyEnd(); it != end; ++it) {
                    outputWidget->removeOutput(*it);
                }
            }
            for (Sublime::Area* area : KDevelop::ICore::self()->uiController()->controller()->allAreas()) {
                area->removeToolView( view );
            }
        }
        delete td;
        m_toolViews.erase(toolViewIt);
    }
}

OutputWidget* StandardOutputView::outputWidgetForId( int outputId ) const
{
    for (ToolViewData* td : m_toolViews) {
        if( td->outputdata.contains( outputId ) )
        {
            for (const auto* const view : std::as_const(td->views)) {
                if (auto* const widget = view->widget()) {
                    return qobject_cast<OutputWidget*>(widget);
                }
            }
        }
    }
    return nullptr;
}

void StandardOutputView::removeOutput( int outputId )
{
    for (ToolViewData* td : std::as_const(m_toolViews)) {
        const auto outputIt = td->outputdata.find(outputId);
        if (outputIt != td->outputdata.end()) {
            for (const auto* const view : std::as_const(td->views)) {
                if (auto* const widget = view->widget()) {
                    qobject_cast<OutputWidget*>(widget)->removeOutput(outputId);
                }
            }
            td->outputdata.erase(outputIt);
        }
    }
}

void StandardOutputView::setTitle(int outputId, const QString& title)
{
    OutputWidget* widget = outputWidgetForId(outputId);
    if (widget) {
        widget->setTitle(outputId, title);
    }
}

#include "moc_standardoutputview.cpp"
