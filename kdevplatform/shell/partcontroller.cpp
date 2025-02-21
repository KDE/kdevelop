/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2015 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "partcontroller.h"

#include <QAction>
#include <QMimeDatabase>
#include <QMimeType>

#include <KActionCollection>
#include <KToggleAction>
#include <KLocalizedString>

#include <KParts/Part>
#include <KParts/PartLoader>

#include <KTextEditor/View>
#include <KTextEditor/Editor>
#include <KTextEditor/Document>

#include "core.h"
#include "textdocument.h"
#include "debug.h"
#include "uicontroller.h"
#include "mainwindow.h"
#include <interfaces/isession.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <sublime/area.h>

namespace KDevelop
{
namespace {
QString mimeTypeForUrl(const QUrl& url)
{
    Q_ASSERT(url.isValid());
    Q_ASSERT(!url.isEmpty());
    return QMimeDatabase().mimeTypeForUrl(url).name();
}
}

class PartControllerPrivate
{
public:
    explicit PartControllerPrivate(Core* core)
        : m_core(core)
    {}

    bool m_showTextEditorStatusBar = false;
    QString m_editor;
    QStringList m_textTypes;

    Core* const m_core;
};

PartController::PartController(Core *core, QWidget *toplevel)
    : IPartController(toplevel)
    , d_ptr(new PartControllerPrivate(core))

{
    setObjectName(QStringLiteral("PartController"));

    //Cache this as it is too expensive when creating parts
    //     KConfig * config = Config::standard();
    //     config->setGroup( "General" );
    //
    //     d->m_textTypes = config->readEntry( "TextTypes", QStringList() );
    //
    //     config ->setGroup( "Editor" );
    //     d->m_editor = config->readPathEntry( "EmbeddedKTextEditor", QString() );

    // required early because some actions are checkable and need to be initialized
    loadSettings(false);

    if (!(Core::self()->setupFlags() & Core::NoUi))
        setupActions();
}

PartController::~PartController() = default;

bool PartController::showTextEditorStatusBar() const
{
    Q_D(const PartController);

    return d->m_showTextEditorStatusBar;
}

void PartController::setShowTextEditorStatusBar(bool show)
{
    Q_D(PartController);

    if (d->m_showTextEditorStatusBar == show)
        return;

    d->m_showTextEditorStatusBar = show;

    // update
    const auto areas = Core::self()->uiControllerInternal()->allAreas();
    for (Sublime::Area* area : areas) {
        const auto views = area->views();
        for (Sublime::View* view : views) {
            auto* const widget = view->widget();
            if (!widget) {
                continue;
            }

            auto* const textView = qobject_cast<KTextEditor::View*>(widget);
            if (textView) {
                textView->setStatusBarEnabled(show);
            }
        }
    }

    // also notify active view that it should update the "view status"
    auto* textView = qobject_cast<TextView*>(Core::self()->uiControllerInternal()->activeSublimeWindow()->activeView());
    if (textView) {
        emit textView->statusChanged(textView);
    }
}

//MOVE BACK TO DOCUMENTCONTROLLER OR MULTIBUFFER EVENTUALLY
bool PartController::isTextType(const QMimeType& mimeType)
{
    Q_D(PartController);

    bool isTextType = false;
    if (d->m_textTypes.contains(mimeType.name()))
    {
        isTextType = true;
    }

    // is this regular text - open in editor
    return ( isTextType
             || mimeType.inherits(QStringLiteral("text/plain"))
             || mimeType.inherits(QStringLiteral("text/html"))
             || mimeType.inherits(QStringLiteral("application/x-zerosize")));
}

KTextEditor::Editor* PartController::editorPart() const
{
    return KTextEditor::Editor::instance();
}

KTextEditor::Document* PartController::createTextPart()
{
    return editorPart()->createDocument(this);
}

bool PartController::canCreatePart(const QUrl& url)
{
    if (!url.isValid()) {
        return false;
    }

    return !KParts::PartLoader::partsForMimeType(mimeTypeForUrl(url)).isEmpty();
}

KParts::Part* PartController::createPart( const QUrl & url, const QString& preferredPart )
{
    if (!url.isValid()) {
        return nullptr;
    }

    qCDebug(SHELL) << "creating part with url" << url << "and pref part:" << preferredPart;
    KParts::Part* part = createPart(mimeTypeForUrl(url), preferredPart);
    if (!part) {
        return nullptr;
    }

    // only ReadOnlyParts are supported by PartController
    static_cast<KParts::ReadOnlyPart*>(part)->openUrl(url);

    // restrict keyboard shortcuts to the KParts view
    const auto actions = part->actionCollection()->actions();
    for (auto* action : actions) {
        if (action->shortcutContext() != Qt::WidgetShortcut) {
            action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        }
    }

    return part;
}

void PartController::loadSettings( bool projectIsLoaded )
{
    Q_D(PartController);

    Q_UNUSED( projectIsLoaded );

    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("UiSettings"));
    d->m_showTextEditorStatusBar = cg.readEntry("ShowTextEditorStatusBar", false);
}

void PartController::saveSettings( bool projectIsLoaded )
{
    Q_D(PartController);

    Q_UNUSED( projectIsLoaded );

    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("UiSettings"));
    cg.writeEntry("ShowTextEditorStatusBar", d->m_showTextEditorStatusBar);
}

void PartController::initialize()
{
}

void PartController::cleanup()
{
    saveSettings(false);
}

void PartController::setupActions()
{
    Q_D(PartController);

    KActionCollection* actionCollection =
        d->m_core->uiControllerInternal()->defaultMainWindow()->actionCollection();

    QAction* action;

    action = KStandardAction::showStatusbar(this, SLOT(setShowTextEditorStatusBar(bool)), actionCollection);
    action->setWhatsThis(i18nc("@info:whatsthis", "Use this command to show or hide the view's statusbar."));
    action->setChecked(showTextEditorStatusBar());
}

}

#include "moc_partcontroller.cpp"
