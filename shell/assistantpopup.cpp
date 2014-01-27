/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2012 Milian Wolff <mail@milianw.de>
   Copyright 2014 Sven Brauch <svenbrauch@gmail.com>

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
#include <cmath>

#include <QAction>
#include <QVBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QScrollBar>
#include <QStyle>
#include <QDebug>
#include <QEvent>

#include <KLocalizedString>
#include <KParts/MainWindow>
#include <KStandardDirs>
#include <KTextEditor/HighlightInterface>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/ConfigInterface>
#include <KColorUtils>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>

using namespace KDevelop;

AssistantPopup::AssistantPopup(KTextEditor::View* parent, const IAssistant::Ptr& assistant)
// main window as parent to use maximal space available in worst case
    : QDeclarativeView(ICore::self()->uiController()->activeMainWindow())
    , m_assistant(assistant)
    , m_view(parent)
{
    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::transparent);
    setPalette(p);
    setBackgroundRole(QPalette::Window);
    setBackgroundBrush(QBrush(QColor(0, 0, 0, 0)));
    setResizeMode(QDeclarativeView::SizeViewToRootObject);

    m_config = new AssistantPopupConfig(this);
    auto doc = ICore::self()->documentController()->activeDocument();
    m_config->setColorsFromView(doc->textDocument()->activeView());
    updateActions();
    rootContext()->setContextProperty("config", QVariant::fromValue<QObject*>(m_config));
    setSource(QUrl(KStandardDirs::locate("data", "kdevelop/assistantpopup.qml")));
    Q_ASSERT(assistant);
    if ( ! rootObject() ) {
        kWarning() << "Failed to load assistant markup! The assistant will not work.";
        return;
    }
    connect(m_view, SIGNAL(verticalScrollPositionChanged(KTextEditor::View*,KTextEditor::Cursor)),
            this, SLOT(updatePosition()));

    updatePosition();
    connect(this, SIGNAL(sceneResized(QSize)),
            this, SLOT(updatePosition()));
    connect(m_view, SIGNAL(destroyed(QObject*)),
            this, SLOT(deleteLater()));
    m_view->installEventFilter(this);
    m_view->setFocus();
}

AssistantPopupConfig::AssistantPopupConfig(QObject *parent): QObject(parent)
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
    if ( fabs(lumaDiff) < 0.5 ) {
        m_highlight = QColor::fromHsv(m_highlight.hue(),
                                    qMin(255, m_highlight.saturation() + 80),
                                    lumaDiff > 0 ? qMin(255, m_highlight.value() + 120)
                                                 : qMax(80, m_highlight.value() - 40));
    }
}

static QWidget* findByClassname(KTextEditor::View* view, const QString& klass) {
    auto children = view->findChildren<QWidget*>();
    for ( auto child: children ) {
        if ( child->metaObject()->className() == klass ) {
            return child;
        }
    }
    return static_cast<QWidget*>(nullptr);
};

QRect AssistantPopup::textWidgetGeometry(KTextEditor::View *view) const
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

void AssistantPopup::keyReleaseEvent(QKeyEvent *event)
{
    if ( event->key() == Qt::Key_Alt ) {
        m_view->setFocus();
        emit m_config->shouldShowHighlight(false);
    }
    QGraphicsView::keyReleaseEvent(event);
}

bool AssistantPopup::eventFilter(QObject* object, QEvent* event)
{
    Q_ASSERT(object == m_view);
    Q_UNUSED(object);
    if (event->type() == QEvent::Resize) {
        updatePosition();
    } else if (event->type() == QEvent::Hide) {
        executeHideAction();
    } else if (event->type() == QEvent::KeyPress) {
        // While the Alt key is pressed, give focus to the assistant widget
        // and notify it about that.
        auto modifiers = static_cast<QKeyEvent*>(event)->modifiers();
        if (modifiers == Qt::AltModifier) {
            setFocus();
            emit m_config->shouldShowHighlight(true);
            return true;
        }
        if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape) {
            executeHideAction();
        }
    }
    return false;
}

void AssistantPopup::updatePosition()
{
    auto editorGeometry = textWidgetGeometry(m_view);
    auto cursor = m_view->cursorToCoordinate(KTextEditor::Cursor(0, 0));
    const int margin = 12;
    Sublime::HoldUpdates hold(ICore::self()->uiController()->activeMainWindow());
    if ( cursor.y() < 0 ) {
        // Only when the view is not scrolled to the top, place the widget there; otherwise it easily gets
        // in the way.
        move(parentWidget()->mapFromGlobal(m_view->mapToGlobal(editorGeometry.topRight()
             + QPoint(-width() - margin, margin))));
    }
    else {
        move(parentWidget()->mapFromGlobal(m_view->mapToGlobal(editorGeometry.bottomRight()
             + QPoint(-width() - margin, -margin - height()))));
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
        m_view->setFocus();
    }
}

void AssistantPopup::notifyReopened()
{
    emit m_config->shouldCancelAnimation();
}

void AssistantPopup::updateActions()
{
    m_assistantActions = m_assistant->actions();
    QList<QObject*> items;
    foreach(IAssistantAction::Ptr action, m_assistantActions)
    {
        items << new AssistantButton(action->toKAction(), action->description(), this);
    }
    auto hideAction = new KAction(i18n("Hide"), this);
    connect(hideAction, SIGNAL(triggered()), this, SLOT(executeHideAction()));
    items << new AssistantButton(hideAction, hideAction->text(), this);
    m_config->setModel(items);
    m_config->setTitle(m_assistant->title());
}


#include "assistantpopup.moc"
