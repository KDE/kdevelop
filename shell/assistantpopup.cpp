/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2012 Milian Wolff <mail@milianw.de>
   Copyright 2014 Sven Brauch <svenbrauch@gmail.com>
   Copyright 2014 Kevin Funk <kfunk@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "assistantpopup.h"
#include "sublime/holdupdates.h"
#include "util/kdevstringhandler.h"

#include <QAction>
#include <QDeclarativeContext>
#include <QKeyEvent>
#include <QDebug>
#include <QEvent>
#include <QTimer>

#include <KDebug>
#include <KLocalizedString>
#include <KParts/MainWindow>
#include <KStandardDirs>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/ConfigInterface>
#include <KColorUtils>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>

using namespace KDevelop;

namespace {

/// Interval after which the state of the popup is re-evaluated
/// Used to avoid flickering caused when user is quickly inserting code
const int UPDATE_STATE_INTERVAL = 300; // ms

QWidget* findByClassname(const KTextEditor::View* view, const QString& klass)
{
    auto children = view->findChildren<QWidget*>();
    for ( auto child: children ) {
        if ( child->metaObject()->className() == klass ) {
            return child;
        }
    }
    return nullptr;
};

/**
 * @brief Get the geometry of the inner part (with the text) of the KTextEditor::View being used.
 */
QRect textWidgetGeometry(const KTextEditor::View *view)
{
    // Subtract the width of the right scrollbar
    int scrollbarWidth = 0;
    if ( auto scrollbar = findByClassname(view, "KateScrollBar") ) {
        scrollbarWidth = scrollbar->width();
    }
    // Subtract the width of the bottom scrollbar
    int bottomScrollbarWidth = 0;
    if ( auto bottom = findByClassname(view, "QScrollBar") ) {
        bottomScrollbarWidth = bottom->height();
    }
    auto geom = view->geometry();

    geom.adjust(0, 0, -scrollbarWidth, -bottomScrollbarWidth);
    return geom;
}

}

AssistantPopupConfig::AssistantPopupConfig(QObject *parent)
    : QObject(parent)
    , m_active(false)
    , m_useVerticalLayout(false)
{
}

void AssistantPopupConfig::setColorsFromView(QObject *view)
{
    auto iface = dynamic_cast<KTextEditor::ConfigInterface*>(view);
    Q_ASSERT(iface);
    m_foreground = iface->configValue("line-number-color").value<QColor>();
    m_background = iface->configValue("icon-border-color").value<QColor>();
    m_highlight = iface->configValue("folding-marker-color").value<QColor>();
    if ( KColorUtils::luma(m_background) < 0.3 ) {
        m_foreground = KColorUtils::lighten(m_foreground, 0.7);
    }
    const float lumaDiff = KColorUtils::luma(m_highlight) - KColorUtils::luma(m_background);
    if ( qAbs(lumaDiff) < 0.5 ) {
        m_highlight = QColor::fromHsv(m_highlight.hue(),
                                    qMin(255, m_highlight.saturation() + 80),
                                    lumaDiff > 0 ? qMin(255, m_highlight.value() + 120)
                                                 : qMax(80, m_highlight.value() - 40));
    }
    emit colorsChanged();
}

bool AssistantPopupConfig::isActive() const
{
    return m_active;
}

void AssistantPopupConfig::setActive(bool active)
{
    if (m_active == active) {
        return;
    }

    m_active = active;
    emit activeChanged(m_active);
}

void AssistantPopupConfig::setUseVerticalLayout(bool vertical)
{
    if (m_useVerticalLayout == vertical) {
        return;
    }

    m_useVerticalLayout = vertical;
    emit useVerticalLayoutChanged(m_useVerticalLayout);
}

void AssistantPopupConfig::setTitle(const QString& title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged(m_title);
}

void AssistantPopupConfig::setModel(const QList<QObject*>& model)
{
    if (m_model == model) {
        return;
    }

    m_model = model;
    emit modelChanged(model);
}

AssistantPopup::AssistantPopup()
// main window as parent to use maximal space available in worst case
    : QDeclarativeView(ICore::self()->uiController()->activeMainWindow())
    , m_config(new AssistantPopupConfig(this))
    , m_shownAtBottom(false)
    , m_reopening(false)
    , m_updateTimer(new QTimer(this))
{
    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::transparent);
    setPalette(p);
    setBackgroundRole(QPalette::Window);
    setBackgroundBrush(QBrush(QColor(0, 0, 0, 0)));
    setResizeMode(QDeclarativeView::SizeViewToRootObject);
    setAttribute(Qt::WA_ShowWithoutActivating);

    rootContext()->setContextProperty("config", m_config);

    setSource(QUrl::fromLocalFile(KStandardDirs::locate("data", "kdevelop/assistantpopup.qml")));
    if (!rootObject()) {
        kWarning() << "Failed to load assistant markup! The assistant will not work.";
    }

    m_updateTimer->setInterval(UPDATE_STATE_INTERVAL);
    m_updateTimer->setSingleShot(true);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updateState()));
}

void AssistantPopup::grabFocus()
{
    m_config->setActive(true);
    if (m_view) {
        setFocus();
    }
}

void AssistantPopup::ungrabFocus()
{
    m_config->setActive(false);
    if (m_view && hasFocus()) {
        m_view->setFocus();
    }
}

void AssistantPopup::reset(KTextEditor::View* view, const IAssistant::Ptr& assistant)
{
    setView(view);
    setAssistant(assistant);
    updateLayoutType();

    m_updateTimer->start();
}

void AssistantPopup::setView(KTextEditor::View* view)
{
    if (m_view == view) {
        return;
    }

    ungrabFocus();

    if (m_view) {
        m_view->removeEventFilter(this);
        disconnect(m_view, SIGNAL(verticalScrollPositionChanged(KTextEditor::View*,KTextEditor::Cursor)),
                  this, SLOT(updatePosition(KTextEditor::View*,KTextEditor::Cursor)));
    }
    m_view = view;
    if (m_view) {
        m_view->installEventFilter(this);
        connect(m_view, SIGNAL(verticalScrollPositionChanged(KTextEditor::View*,KTextEditor::Cursor)),
                this, SLOT(updatePosition(KTextEditor::View*,KTextEditor::Cursor)));
    }
}

void AssistantPopup::setAssistant(const IAssistant::Ptr& assistant)
{
    if (m_assistant == assistant) {
        return;
    }

    if (m_assistant) {
        disconnect(m_assistant.data(), SIGNAL(actionsChanged()), m_updateTimer, SLOT(start()));
        disconnect(m_assistant.data(), SIGNAL(hide()), this, SLOT(hideAssistant()));
    }
    m_assistant = assistant;
    if (m_assistant) {
        connect(m_assistant.data(), SIGNAL(actionsChanged()), m_updateTimer, SLOT(start()));
        connect(m_assistant.data(), SIGNAL(hide()), this, SLOT(hideAssistant()));
    }
}


bool AssistantPopup::viewportEvent(QEvent *event)
{
    // For some reason, QGraphicsView posts a WindowActivate event
    // when it is shown, even if disabled through setting the WA_ShowWithoutActivate
    // attribute. This causes all focus-driven popups (QuickOpen, tooltips, ...)
    // to hide when the assistant opens. Thus, prevent it from processing the Show event here.
    if ( event->type() == QEvent::Show ) {
        return true;
    }
    return QGraphicsView::viewportEvent(event);
}

void AssistantPopup::keyPressEvent(QKeyEvent* event)
{
    if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9) {
        auto actions = m_config->model();
        const int field = event->key() - Qt::Key_0;
        if (field == 0) {
            executeHideAction();
        } else {
            auto action = m_assistant->actions().value(field - 1);
            if (action) {
                action->execute();
            }
        }
    } else {
        QDeclarativeView::keyPressEvent(event);
    }
}


void AssistantPopup::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Alt || event->modifiers() == Qt::AltModifier) {
        ungrabFocus();
    } else {
        QDeclarativeView::keyReleaseEvent(event);
    }
}

bool AssistantPopup::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object);

    if (!m_view || (object != m_view.data()))
        return false;

    if (event->type() == QEvent::Resize) {
        updateLayoutType();
        updatePosition(m_view.data(), KTextEditor::Cursor::invalid());
    } else if (event->type() == QEvent::Hide) {
        executeHideAction();
    } else if (event->type() == QEvent::WindowDeactivate) {
        ungrabFocus();
    } else if (event->type() == QEvent::KeyPress) {
        auto keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->modifiers() == Qt::AltModifier) {
            grabFocus();
        }
        if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape) {
            executeHideAction();
        }
    }
    return false;
}

void AssistantPopup::updatePosition(KTextEditor::View* view, const KTextEditor::Cursor& newPos)
{
    static const int MARGIN = 12;

    if (newPos.isValid() && newPos.line() == 0 && !m_shownAtBottom) {
        // the position is not going to change; don't waste time
        return;
    }

    auto editorGeometry = textWidgetGeometry(view);
    const auto startCursorCoordinate = view->cursorToCoordinate(KTextEditor::Cursor(0, 0));

    // algorithm for popup positioning:
    // if we are scrolled to the top: show at bottom
    // else:
    //   if: current cursor position is in upper half => show at bottom
    //   else: show at top
    const bool showAtBottom = startCursorCoordinate.y() == 0 ? true :
        view->cursorPositionCoordinates().y() < view->height()/2;
    const QPoint targetLocation = showAtBottom ?
        parentWidget()->mapFromGlobal(view->mapToGlobal(editorGeometry.bottomRight()
                                      + QPoint(-width() - MARGIN, -MARGIN - height()))) :
        parentWidget()->mapFromGlobal(view->mapToGlobal(editorGeometry.topRight()
                                      + QPoint(-width() - MARGIN, MARGIN)));
    if (pos() == targetLocation) {
        return;
    }

    if ( m_reopening ) {
        // When the assistant is already visible, close to no flickering will occur anyways,
        // so we can avoid the full repaint of the window.
        move(targetLocation);
    }
    else {
        Sublime::HoldUpdates hold(ICore::self()->uiController()->activeMainWindow());
        move(targetLocation);
    }
}

IAssistant::Ptr AssistantPopup::assistant() const
{
    return m_assistant;
}

void AssistantPopup::executeHideAction()
{
    if ( isVisible() ) {
        m_assistant->doHide();
    }
}

void AssistantPopup::hideAssistant()
{
    reset(nullptr, {}); // indirectly calls hide()
}

void AssistantPopup::updateLayoutType()
{
    if ( ! assistant() ) {
        return;
    }
    // Make a rough estimate of the width the assistant will need
    // and decide on whether to use vertical layout or not.
    const auto& metrics = fontMetrics();
    auto textWidth = 0;

    textWidth += metrics.boundingRect(KDevelop::htmlToPlainText(assistant()->title())).width();
    for ( const auto& action: assistant()->actions() ) {
        textWidth += metrics.boundingRect(KDevelop::htmlToPlainText(action->description())).width();
        textWidth += 10;
    }
    m_config->setUseVerticalLayout(textWidth > textWidgetGeometry(m_view).width()*0.75);
    updateState();
}

void AssistantPopup::updateState()
{
    if (!m_assistant || m_assistant->actions().isEmpty()) {
        hide();
        return;
    }

    QList<QObject*> items;
    foreach (IAssistantAction::Ptr action, m_assistant->actions()) {
        items << action->toKAction();
    }
    auto hideAction = new QAction(i18n("Hide"), m_assistant.data());
    connect(hideAction, SIGNAL(triggered()), this, SLOT(executeHideAction()));
    items << hideAction;

    auto view = ICore::self()->documentController()->activeTextDocumentView();
    m_config->setColorsFromView(view);
    m_config->setModel(items);
    m_config->setTitle(m_assistant->title());
    m_config->setActive(false);

    // both changed title or actions may change the appearance of the popup
    // force recomputing the size hint
    resize(sizeHint());
    updatePosition(m_view, KTextEditor::Cursor::invalid());

    show();
}

