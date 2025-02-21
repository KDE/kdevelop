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
    explicit OutputViewFactory(const ToolViewData* data): m_data(data) {}
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
        //NOTE: id must be unique, see e.g. https://bugs.kde.org/show_bug.cgi?id=287093
        return QStringLiteral("org.kdevelop.OutputView.%1").arg(m_data->toolViewId);
    }
private:
    const ToolViewData *m_data;
};

StandardOutputView::StandardOutputView(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevstandardoutputview"), parent, metaData)
{
    connect(KDevelop::ICore::self()->uiController()->controller(), &Sublime::Controller::aboutToRemoveView,
            this, &StandardOutputView::removeSublimeView);

}

void StandardOutputView::removeSublimeView( Sublime::View* v )
{
    auto it = m_toolViews.begin();
    while (it != m_toolViews.end()) {
        ToolViewData* d = it.value();
        bool isErased = false;
        if( d->views.contains(v) )
        {
            if( d->views.count() == 1 )
            {
                isErased = true;
                it = m_toolViews.erase(it);
                m_ids.removeAll( d->toolViewId );
                delete d;
            } else
            {
                d->views.removeAll(v);
            }
        }
        if (!isErased) {
            ++it;
        }
    }
}

StandardOutputView::~StandardOutputView()
{
}

int StandardOutputView::standardToolView( KDevelop::IOutputView::StandardToolView view )
{
    const auto standardViewIt = m_standardViews.constFind(view);
    if (standardViewIt != m_standardViews.constEnd()) {
        return *standardViewIt;
    }

    int ret = -1;
    switch( view )
    {
    case KDevelop::IOutputView::BuildView:
        ret = registerToolView(QStringLiteral("Build"), i18nc("@title:window", "Build"),
                               KDevelop::IOutputView::HistoryView, QIcon::fromTheme(QStringLiteral("run-build")),
                               KDevelop::IOutputView::AddFilterAction);
        break;
    case KDevelop::IOutputView::RunView:
        ret = registerToolView(QStringLiteral("Run"), i18nc("@title:window", "Run"),
                               KDevelop::IOutputView::MultipleView, QIcon::fromTheme(QStringLiteral("system-run")),
                               KDevelop::IOutputView::AddFilterAction);
        break;
    case KDevelop::IOutputView::DebugView:
        ret = registerToolView(QStringLiteral("Debug"), i18nc("@title:window", "Debug"),
                               KDevelop::IOutputView::MultipleView, QIcon::fromTheme(QStringLiteral("debug-step-into")),
                               KDevelop::IOutputView::AddFilterAction);
        break;
    case KDevelop::IOutputView::TestView:
        // TODO: pass QByteArrayLiteral("Test") instead of QByteArray() and make the settings work.
        // The settings are disabled for the Test output tool view, because all Test output views are currently
        // unclosable and there is only one output tool view option right now: output view number limit, which
        // automatically closes its views and has no effect if the views are not closable.
        // AllowUserClose output view behavior is enabled by default, but 3 output jobs - CompileAnalyzeJob,
        // cppcheck::Job and Heaptrack::Job - disable it by calling `setBehaviours(KDevelop::IOutputView::AutoScroll);`.
        // 1cd05dd39bb67cbec7ce39a0237b64fb42e5fc95 introduced the first unclosable view in the cppcheck plugin with
        // no surviving explanation. Other Test output views followed the example later. Ideally all output views should
        // become closable, if there is no good reason to prevent closing and destroying them.
        ret = registerToolView(QString(), i18nc("@title:window", "Test"), KDevelop::IOutputView::HistoryView,
                               QIcon::fromTheme(QStringLiteral("system-run")));
        break;
    case KDevelop::IOutputView::VcsView:
        ret = registerToolView(QStringLiteral("VersionControl"), i18nc("@title:window", "Version Control"),
                               KDevelop::IOutputView::HistoryView, QIcon::fromTheme(QStringLiteral("system-run")));
        break;
    }

    Q_ASSERT(ret != -1);
    m_standardViews[view] = ret;
    return ret;
}

int StandardOutputView::registerToolView(const QString& configSubgroupName, const QString& title,
                                         KDevelop::IOutputView::ViewType type, const QIcon& icon, Options option,
                                         const QList<QAction*>& actionList)
{
    // try to reuse existing tool view
    for (ToolViewData* d : std::as_const(m_toolViews)) {
        if ( d->type == type && d->title == title ) {
            return d->toolViewId;
        }
    }

    // register new tool view
    const int newid = m_ids.isEmpty() ? 0 : (m_ids.last() + 1);
    qCDebug(PLUGIN_STANDARDOUTPUTVIEW) << "Registering view" << title << "with type:" << type << "id:" << newid;
    auto* tvdata = new ToolViewData( this );
    tvdata->toolViewId = newid;
    tvdata->configSubgroupName = configSubgroupName;
    tvdata->type = type;
    tvdata->title = title;
    tvdata->icon = icon;
    tvdata->plugin = this;
    tvdata->option = option;
    tvdata->actionList = actionList;
    core()->uiController()->addToolView( title, new OutputViewFactory( tvdata ) );
    m_ids << newid;
    m_toolViews[newid] = tvdata;
    return newid;
}

int StandardOutputView::registerOutputInToolView( int toolViewId,
                                                  const QString& title,
                                                  KDevelop::IOutputView::Behaviours behaviour )
{
    const auto toolViewIt = m_toolViews.constFind(toolViewId);
    if (toolViewIt == m_toolViews.constEnd())
        return -1;
    int newid;
    if( m_ids.isEmpty() )
    {
        newid = 0;
    } else
    {
        newid = m_ids.last()+1;
    }
    m_ids << newid;
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
//       StandardOutputView::registerToolView() calls IUiController::addToolView() but
//       StandardOutputView::unload() does not call IUiController::removeToolView() as all
//       other plugins do. This does not cause a crash when the user attempts to unload
//       the Output View plugin on the Plugins tab of the Configure KDevelop dialog, because
//       the unloading is prevented by the absence of the "X-KDevelop-Category": "Global"
//       entry in kdevstandardoutputview.json (see df99304843fbdb593398e3d7d912e15cbd7c56b9).
void StandardOutputView::removeToolView(int toolViewId)
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
            for (Sublime::View* view : std::as_const(td->views)) {
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
            for (Sublime::View* view : std::as_const(td->views)) {
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
