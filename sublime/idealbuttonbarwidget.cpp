/*
  Copyright 2007 Roberto Raggi <roberto@kdevelop.org>
  Copyright 2007 Hamish Rodda <rodda@kde.org>
  Copyright 2011 Alexander Dymo <adymo@kdevelop.org>

  Permission to use, copy, modify, distribute, and sell this software and its
  documentation for any purpose is hereby granted without fee, provided that
  the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation.

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  KDEVELOP TEAM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "idealbuttonbarwidget.h"
#include "mainwindow.h"
#include "idealdockwidget.h"
#include "ideallayout.h"
#include "idealtoolbutton.h"
#include "document.h"
#include "view.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>

#include <QBoxLayout>
#include <QApplication>

using namespace Sublime;

class ToolViewAction : public QAction
{
public:
    ToolViewAction(IdealDockWidget *dock, QObject* parent) : QAction(parent), m_dock(dock)
    {
        setCheckable(true);

        const QString title = dock->view()->document()->title();
        setIcon(dock->windowIcon());
        setToolTip(i18n("Toggle '%1' tool view.", title));
        setText(title);

        //restore toolview shortcut config
        KConfigGroup config = KSharedConfig::openConfig()->group("UI");
        QStringList shortcutStrings = config.readEntry(QStringLiteral("Shortcut for %1").arg(title), QStringList());
        setShortcuts({ QKeySequence::fromString(shortcutStrings.value(0)), QKeySequence::fromString(shortcutStrings.value(1)) });

        dock->setWindowTitle(title);
        dock->view()->widget()->installEventFilter(this);
        refreshText();
    }

    IdealDockWidget *dockWidget() const
    {
        Q_ASSERT(m_dock);
        return m_dock;
    }

    IdealToolButton* button() { return m_button; }
    void setButton(IdealToolButton* button) {
        m_button = button;
        refreshText();
    }

private:
    bool eventFilter(QObject * watched, QEvent * event) override
    {
        if (watched == m_dock->view()->widget() && event->type() == QEvent::EnabledChange) {
            refreshText();
        }

        return QObject::eventFilter(watched, event);
    }

    void refreshText()
    {
        const auto widget = m_dock->view()->widget();
        const QString title = m_dock->view()->document()->title();
        setText(widget->isEnabled() ? title : QStringLiteral("(%1)").arg(title));
    }

    QPointer<IdealDockWidget> m_dock;
    QPointer<IdealToolButton> m_button;
};

IdealButtonBarWidget::IdealButtonBarWidget(Qt::DockWidgetArea area,
        IdealController *controller, Sublime::MainWindow *parent)
    : QWidget(parent)
    , _area(area)
    , _controller(controller)
    , _corner(nullptr)
    , _showState(false)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setToolTip(i18nc("@info:tooltip", "Right click to add new tool views."));

    if (area == Qt::BottomDockWidgetArea)
    {
        QBoxLayout *statusLayout = new QBoxLayout(QBoxLayout::RightToLeft, this);
        statusLayout->setMargin(0);
        statusLayout->setSpacing(IDEAL_LAYOUT_SPACING);
        statusLayout->setContentsMargins(0, IDEAL_LAYOUT_MARGIN, 0, IDEAL_LAYOUT_MARGIN);

        IdealButtonBarLayout *l = new IdealButtonBarLayout(orientation());
        statusLayout->addLayout(l);

        _corner = new QWidget(this);
        QBoxLayout *cornerLayout = new QBoxLayout(QBoxLayout::LeftToRight, _corner);
        cornerLayout->setMargin(0);
        cornerLayout->setSpacing(0);
        statusLayout->addWidget(_corner);
        statusLayout->addStretch(1);
    }
    else
        (void) new IdealButtonBarLayout(orientation(), this);
}

QAction* IdealButtonBarWidget::addWidget(IdealDockWidget *dock,
                                         Area *area, View *view)
{
    if (_area == Qt::BottomDockWidgetArea || _area == Qt::TopDockWidgetArea)
        dock->setFeatures( dock->features() | IdealDockWidget::DockWidgetVerticalTitleBar );

    dock->setArea(area);
    dock->setView(view);
    dock->setDockWidgetArea(_area);

    bool wasEmpty = actions().isEmpty();

    auto action = new ToolViewAction(dock, this);
    addAction(action);

    if(wasEmpty)
        emit emptyChanged();
    return action;
}

QWidget* IdealButtonBarWidget::corner()
{
    return _corner;
}

void IdealButtonBarWidget::removeAction(QAction * widgetAction)
{
    auto action = dynamic_cast<ToolViewAction*>(widgetAction);
    delete action->button();
    delete action;
}

bool IdealButtonBarWidget::isEmpty()
{
    return actions().isEmpty();
}

bool IdealButtonBarWidget::isShown()
{
    return std::any_of(actions().cbegin(), actions().cend(),
                       [](const QAction* action){ return action->isChecked(); });
}

void IdealButtonBarWidget::saveShowState()
{
    _showState = isShown();
}

bool IdealButtonBarWidget::lastShowState()
{
    return _showState;
}

Qt::Orientation IdealButtonBarWidget::orientation() const
{
    if (_area == Qt::LeftDockWidgetArea || _area == Qt::RightDockWidgetArea)
        return Qt::Vertical;

    return Qt::Horizontal;
}

Qt::DockWidgetArea IdealButtonBarWidget::area() const
{
    return _area;
}

void IdealButtonBarWidget::showWidget(bool checked)
{
    Q_ASSERT(parentWidget() != nullptr);

    QAction *action = qobject_cast<QAction *>(sender());
    Q_ASSERT(action);

    showWidget(action, checked);
}

void IdealButtonBarWidget::showWidget(QAction *action, bool checked)
{
    auto widgetAction = dynamic_cast<ToolViewAction*>(action);

    IdealToolButton* button = widgetAction->button();
    Q_ASSERT(button);

    if (checked) {
        IdealController::RaiseMode mode = IdealController::RaiseMode(widgetAction->property("raise").toInt());
        if ( mode == IdealController::HideOtherViews ) {
            // Make sure only one widget is visible at any time.
            // The alternative to use a QActionCollection and setting that to "exclusive"
            // has a big drawback: QActions in a collection that is exclusive cannot
            // be un-checked by the user, e.g. in the View -> Tool Views menu.
            foreach(QAction *otherAction, actions()) {
                if ( otherAction != widgetAction && otherAction->isChecked() )
                    otherAction->setChecked(false);
            }
        }

        _controller->lastDockWidget[_area] = widgetAction->dockWidget();
    }

    _controller->showDockWidget(widgetAction->dockWidget(), checked);
    widgetAction->setChecked(checked);
    button->setChecked(checked);
}


void IdealButtonBarWidget::actionEvent(QActionEvent *event)
{
    auto action = dynamic_cast<ToolViewAction *>(event->action());
    if (!action)
      return;

    switch (event->type()) {
    case QEvent::ActionAdded: {
        bool wasEmpty = isEmpty();
        if (!action->button()) {
            IdealToolButton *button = new IdealToolButton(_area);
            //apol: here we set the usual width of a button for the vertical toolbars as the minimumWidth
            //this is done because otherwise when we remove all the buttons and re-add new ones we get all
            //the screen flickering. This is solved by not defaulting to a smaller width when it's empty
            int w = button->sizeHint().width();
            if(orientation()==Qt::Vertical && w>minimumWidth())
                setMinimumWidth(w);
            action->setButton(button);
            button->setDefaultAction(action);

            Q_ASSERT(action->dockWidget());

            layout()->addWidget(button);
            connect(action, &QAction::toggled, this, static_cast<void(IdealButtonBarWidget::*)(bool)>(&IdealButtonBarWidget::showWidget));
            connect(button, &IdealToolButton::clicked, this, &IdealButtonBarWidget::buttonPressed);
            connect(button, &IdealToolButton::customContextMenuRequested,
                    action->dockWidget(), &IdealDockWidget::contextMenuRequested);
            if ( wasEmpty ) {
                emit emptyChanged();
            }
        }
    } break;

    case QEvent::ActionRemoved: {
        IdealToolButton *button = action->button();
        if (button) {
            for (int index = 0; index < layout()->count(); ++index) {
                if (QLayoutItem *item = layout()->itemAt(index)) {
                    if (item->widget() == button) {
                        action->disconnect(this);
                        delete layout()->takeAt(index);
                        break;
                    }
                }
            }
        }
        if(layout()->isEmpty()) {
            emit emptyChanged();
        }
    } break;

    default:
        break;
    }
}

IdealDockWidget * IdealButtonBarWidget::widgetForAction(QAction *_action) const
{
    return dynamic_cast<ToolViewAction *>(_action)->dockWidget();
}

void IdealButtonBarWidget::buttonPressed(bool state)
{
    auto button = qobject_cast<IdealToolButton*>(sender());
    Q_ASSERT(button);
    ToolViewAction* action = nullptr;
    foreach(QAction* a, actions()) {
        auto tva = dynamic_cast<ToolViewAction *>(a);
        if (tva && tva->button() == button)
            action = tva;
    }
    Q_ASSERT(action);

    const bool forceGrouping = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);

    if (forceGrouping) {
        action->setProperty("raise", IdealController::GroupWithOtherViews);
    }

    action->setChecked(state);

    if (forceGrouping) {
        // need to reset the raise property so that subsequent
        // showWidget()'s will not do grouping unless explicitly asked
        action->setProperty("raise", IdealController::HideOtherViews);
    }
}
