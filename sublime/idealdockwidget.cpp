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

#include "idealdockwidget.h"
#include "mainwindow.h"
#include "area.h"
#include "document.h"
#include "view.h"
#include <KMenu>
#include <KLocalizedString>
#include <KDialog>
#include <KShortcutWidget>
#include <KSharedConfig>

#include <QAbstractButton>
#include <QToolBar>

using namespace Sublime;

IdealDockWidget::IdealDockWidget(IdealController *controller, Sublime::MainWindow *parent)
    : QDockWidget(parent),
      m_area(0),
      m_view(0),
      m_docking_area(Qt::NoDockWidgetArea),
      m_controller(controller)
{
    setAutoFillBackground(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenuRequested(QPoint)));

    QAbstractButton *closeButton =
    qFindChild<QAbstractButton *>(this, QLatin1String("qt_dockwidget_closebutton"));

    if (closeButton) {
    disconnect(closeButton, SIGNAL(clicked()), 0, 0);

    connect(closeButton, SIGNAL(clicked(bool)), SIGNAL(closeRequested()));
    }

    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    // do not allow to move docks to the top dock area (no buttonbar there in our current UI)
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
}

IdealDockWidget::~IdealDockWidget()
{
}

Area *IdealDockWidget::area() const
{ return m_area; }

void IdealDockWidget::setArea(Area *area)
{ m_area = area; }

View *IdealDockWidget::view() const
{ return m_view; }

void IdealDockWidget::setView(View *view)
{ m_view = view; }

Qt::DockWidgetArea IdealDockWidget::dockWidgetArea() const
{ return m_docking_area; }

void IdealDockWidget::setDockWidgetArea(Qt::DockWidgetArea dockingArea)
{ m_docking_area = dockingArea; }

void IdealDockWidget::slotRemove()
{
    m_area->removeToolView(m_view);
}

void IdealDockWidget::contextMenuRequested(const QPoint &point)
{
    QWidget* senderWidget = qobject_cast<QWidget*>(sender());
    Q_ASSERT(senderWidget);

    KMenu menu;
    menu.addTitle(windowIcon(), windowTitle());

    QList< QAction* > viewActions = m_view->contextMenuActions();
    if(!viewActions.isEmpty()) {
        menu.addActions(viewActions);
        menu.addSeparator();
    }

    ///TODO: can this be cleaned up?
    if(QToolBar* toolBar = widget()->findChild<QToolBar*>()) {
        menu.addAction(toolBar->toggleViewAction());
        menu.addSeparator();
    }

    /// start position menu
    QMenu* positionMenu = menu.addMenu(i18n("Toolview Position"));

    QActionGroup *g = new QActionGroup(this);

    QAction *left = new QAction(i18nc("toolview position", "Left"), g);
    QAction *bottom = new QAction(i18nc("toolview position", "Bottom"), g);
    QAction *right = new QAction(i18nc("toolview position", "Right"), g);
    QAction *top = new QAction(i18nc("toolview position", "Top"), g);

    QAction* actions[] = {left, bottom, right, top};
    for (int i = 0; i < 4; ++i)
    {
        // do not show "move to top" toolview position
        // we never show the top buttonbar and we shouldn't allow docks to go there
        if (top == actions[i])
            continue;
        positionMenu->addAction(actions[i]);
        actions[i]->setCheckable(true);
    }
    if (m_docking_area == Qt::TopDockWidgetArea)
        top->setChecked(true);
    else if (m_docking_area == Qt::BottomDockWidgetArea)
        bottom->setChecked(true);
    else if (m_docking_area == Qt::LeftDockWidgetArea)
        left->setChecked(true);
    else
        right->setChecked(true);
    /// end position menu

    menu.addSeparator();
    QAction *setShortcut = menu.addAction(QIcon::fromTheme("configure-shortcuts"), i18n("Assign Shortcut..."));
    setShortcut->setToolTip(i18n("Use this shortcut to trigger visibility of the toolview."));

    menu.addSeparator();
    QAction* remove = menu.addAction(QIcon::fromTheme("dialog-close"), i18n("Remove Toolview"));

    QAction* triggered = menu.exec(senderWidget->mapToGlobal(point));

    if (triggered)
    {
        if ( triggered == remove ) {
            slotRemove();
            return;
        } else if ( triggered == setShortcut ) {
            KDialog *dialog = new KDialog(this);
            dialog->setCaption(i18n("Assign Shortcut For '%1' Tool View", m_view->document()->title()));
            dialog->setButtons( KDialog::Ok | KDialog::Cancel );
            KShortcutWidget *w = new KShortcutWidget(dialog);
            KShortcut shortcut;
            shortcut.setPrimary(m_controller->actionForView(m_view)->shortcuts().value(0));
            shortcut.setAlternate(m_controller->actionForView(m_view)->shortcuts().value(1));
            w->setShortcut(shortcut);
            dialog->setMainWidget(w);

            if (dialog->exec() == QDialog::Accepted) {
                m_controller->actionForView(m_view)->setShortcuts(w->shortcut().toList());

                //save shortcut config
                KConfigGroup config = KGlobal::config()->group("UI");
                QStringList shortcuts;
                shortcuts << w->shortcut().primary().toString();
                shortcuts << w->shortcut().alternate().toString();
                config.writeEntry(QString("Shortcut for %1").arg(m_view->document()->title()), shortcuts);
                config.sync();
            }

            delete dialog;
            return;
        }

        Sublime::Position pos;
        if (triggered == left)
            pos = Sublime::Left;
        else if (triggered == bottom)
            pos = Sublime::Bottom;
        else if (triggered == right)
            pos = Sublime::Right;
        else if (triggered == top)
            pos = Sublime::Top;
        else
            return;

        Area *area = m_area;
        View *view = m_view;
        /* This call will delete *this, so we no longer
           can access member variables. */
        m_area->moveToolView(m_view, pos);
        area->raiseToolView(view);
    }
}
