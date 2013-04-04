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
#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <kdevplatform/sublime/view.h>
#include <QBoxLayout>
#include <QApplication>

using namespace Sublime;

IdealButtonBarWidget::IdealButtonBarWidget(Qt::DockWidgetArea area,
        IdealController *controller, Sublime::MainWindow *parent)
    : QWidget(parent)
    , _area(area)
    , _controller(controller)
    , _corner(0)
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

KAction *IdealButtonBarWidget::addWidget(const QString& title, IdealDockWidget *dock,
                                         Area *area, View *view)
{
    KAction *action = new KAction(this);
    action->setCheckable(true);
    action->setText(title);
    action->setIcon(dock->windowIcon());
    
    //restore toolview shortcut config
    KConfigGroup config = KGlobal::config()->group("UI");
    QStringList shortcuts = config.readEntry(QString("Shortcut for %1").arg(view->document()->title()), QStringList());
    KShortcut shortcut;
    shortcut.setPrimary(shortcuts.value(0));
    shortcut.setAlternate(shortcuts.value(1));
    action->setShortcut(shortcut);

    if (_area == Qt::BottomDockWidgetArea || _area == Qt::TopDockWidgetArea)
        dock->setFeatures( dock->features() | IdealDockWidget::DockWidgetVerticalTitleBar );

    dock->setArea(area);
    dock->setView(view);
    dock->setDockWidgetArea(_area);

    _widgets[action] = dock;
    connect(action, SIGNAL(toggled(bool)), this, SLOT(showWidget(bool)));

    bool wasEmpty = actions().isEmpty();
    addAction(action);

    if(wasEmpty)
        emit emptyChanged();
    return action;
}

QWidget* IdealButtonBarWidget::corner()
{
    return _corner;
}

void IdealButtonBarWidget::removeAction(QAction * action)
{
    _widgets.remove(action);
    delete _buttons.take(action);
    delete action;
}

bool IdealButtonBarWidget::isEmpty()
{
    return actions().isEmpty();
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
    Q_ASSERT(parentWidget() != 0);

    QAction *action = qobject_cast<QAction *>(sender());
    Q_ASSERT(action);

    showWidget(action, checked, false);
}

void IdealButtonBarWidget::showWidget(QAction *widgetAction, bool checked, bool forceGrouping)
{
    IdealDockWidget *widget = _widgets.value(widgetAction);
    Q_ASSERT(widget);

    if (checked) {
        IdealController::RaiseMode mode = IdealController::RaiseMode(widgetAction->property("raise").toInt());
        if ( forceGrouping ) {
            mode = IdealController::GroupWithOtherViews;
        }
        if ( mode == IdealController::GroupWithOtherViews ) {
            // need to reset the raise property so that subsequent
            // showWidget()'s will not do grouping unless explicitly asked
            widgetAction->setProperty("raise", IdealController::HideOtherViews);
        }

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
    }

    _controller->showDockWidget(widget, checked);
    widgetAction->setChecked(checked);
}


void IdealButtonBarWidget::actionEvent(QActionEvent *event)
{
    QAction *action = qobject_cast<QAction *>(event->action());
    if (! action)
      return;

    switch (event->type()) {
    case QEvent::ActionAdded: {
        if (! _buttons.contains(action)) {
            IdealToolButton *button = new IdealToolButton(_area);
            //apol: here we set the usual width of a button for the vertical toolbars as the minimumWidth
            //this is done because otherwise when we remove all the buttons and re-add new ones we get all
            //the screen flickering. This is solved by not defaulting to a smaller width when it's empty
            int w = button->sizeHint().width();
            if(orientation()==Qt::Vertical && w>minimumWidth())
                setMinimumWidth(w);
            _buttons.insert(action, button);

            button->setText(action->text());
            button->setToolTip(i18n("Toggle '%1' tool view.", action->text()));
            button->setIcon(action->icon());
            button->setShortcut(QKeySequence());
            button->setChecked(action->isChecked());
            button->setProperty("buttonAction", QVariant::fromValue<QObject*>(action));

            Q_ASSERT(_widgets.contains(action));
            _widgets[action]->setWindowTitle(action->text());

            layout()->addWidget(button);
            connect(action, SIGNAL(toggled(bool)), SLOT(actionToggled(bool)));
            connect(button, SIGNAL(toggled(bool)), SLOT(buttonPressed(bool)));
            connect(button, SIGNAL(customContextMenuRequested(QPoint)),
                    _widgets[action], SLOT(contextMenuRequested(QPoint)));
        }
    } break;

    case QEvent::ActionRemoved: {
        if (IdealToolButton *button = _buttons.value(action)) {
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

    case QEvent::ActionChanged: {
        if (IdealToolButton *button = _buttons.value(action)) {
            button->setText(action->text());
            button->setIcon(action->icon());
            button->setShortcut(QKeySequence());
            Q_ASSERT(_widgets.contains(action));
            _widgets[action]->setWindowTitle(action->text());
        }
    } break;

    default:
        break;
    }
}

void IdealButtonBarWidget::buttonPressed(bool checked)
{
    QAction* a = qobject_cast<QAction*>(sender()->property("buttonAction").value<QObject*>());
    bool group = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
    showWidget(a, checked, group);
}

void IdealButtonBarWidget::actionToggled(bool state)
{
    QAction* action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);
    
    IdealToolButton* button = _buttons.value(action);
    Q_ASSERT(button);

    button->setChecked(state);

    if (state)
        _controller->lastDockWidget[_area] = widgetForAction(action);
}

MainWindow* IdealButtonBarWidget::parentWidget() const
{
    return static_cast<Sublime::MainWindow*>(QWidget::parentWidget());
}

IdealDockWidget * IdealButtonBarWidget::widgetForAction(QAction *action) const
{ return _widgets.value(action); }
