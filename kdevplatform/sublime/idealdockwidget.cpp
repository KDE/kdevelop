/*
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2011 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LicenseRef-MIT-KDevelop-Ideal
*/

#include "idealdockwidget.h"
#include "area.h"
#include "document.h"
#include "idealcontroller.h"
#include "view.h"

#include <util/scopeddialog.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KShortcutWidget>

#include <QAbstractButton>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMainWindow>
#include <QMenu>
#include <QToolBar>
#include <QVBoxLayout>
#include <QActionGroup>

using namespace Sublime;

IdealDockWidget::IdealDockWidget(IdealController* controller, QMainWindow* parent)
    : QDockWidget(parent),
      m_area(nullptr),
      m_view(nullptr),
      m_docking_area(Qt::NoDockWidgetArea),
      m_controller(controller)
{
    setAutoFillBackground(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &IdealDockWidget::customContextMenuRequested,
            this, &IdealDockWidget::contextMenuRequested);

    auto *closeButton = findChild<QAbstractButton *>(QStringLiteral("qt_dockwidget_closebutton"));
    if (closeButton) {
    disconnect(closeButton, &QAbstractButton::clicked, nullptr, nullptr);

    connect(closeButton, &QAbstractButton::clicked, this, &IdealDockWidget::closeRequested);
    }

    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
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

    KDevelop::ScopedDialog<QMenu> menu(senderWidget);
    menu->addSection(windowIcon(), m_view->document()->title());

    const QList<QAction*> viewActions = m_view->contextMenuActions();
    if(!viewActions.isEmpty()) {
        // add the view's actions to the context menu,
        // checking each if it can be represented
        for (const auto action : viewActions) {
            if (!action->text().isEmpty() && !action->iconText().isEmpty()) {
                // avoid adding empty menu items
                menu->addAction(action);
            }
        }
        menu->addSeparator();
    }

    ///TODO: can this be cleaned up?
    if(auto* toolBar = widget()->findChild<QToolBar*>()) {
        menu->addAction(toolBar->toggleViewAction());
        menu->addSeparator();
    }

    /// start position menu
    QMenu* positionMenu = menu->addMenu(i18nc("@title:menu", "Tool View Position"));

    auto* g = new QActionGroup(positionMenu);

    auto* left = new QAction(i18nc("@option:radio tool view position", "Left"), g);
    auto* bottom = new QAction(i18nc("@option:radio tool view position", "Bottom"), g);
    auto* right = new QAction(i18nc("@option:radio tool view position", "Right"), g);
    auto* detach = new QAction(i18nc("@option:radio tool view position", "Detached"), g);

    for (auto action : {left, bottom, right, detach}) {
        positionMenu->addAction(action);
        action->setCheckable(true);
    }
    if (isFloating()) {
        detach->setChecked(true);
    } else if (m_docking_area == Qt::BottomDockWidgetArea)
        bottom->setChecked(true);
    else if (m_docking_area == Qt::LeftDockWidgetArea)
        left->setChecked(true);
    else if (m_docking_area == Qt::RightDockWidgetArea)
        right->setChecked(true);
    /// end position menu

    menu->addSeparator();

    QAction *setShortcut = menu->addAction(QIcon::fromTheme(QStringLiteral("configure-shortcuts")), i18nc("@action:inmenu", "Assign Shortcut..."));
    setShortcut->setToolTip(i18nc("@info:tooltip", "Use this shortcut to trigger visibility of the tool view."));

    menu->addSeparator();
    QAction* remove = menu->addAction(QIcon::fromTheme(QStringLiteral("dialog-close")), i18nc("@action:inmenu", "Remove Tool View"));

    QAction* triggered = menu->exec(senderWidget->mapToGlobal(point));

    if (triggered)
    {
        if ( triggered == remove ) {
            slotRemove();
            return;
        } else if ( triggered == setShortcut ) {
            auto* dialog = new QDialog(this);
            dialog->setWindowTitle(i18nc("@title:window", "Assign Shortcut For '%1' Tool View", m_view->document()->title()));
            auto *w = new KShortcutWidget(dialog);
            w->setShortcut(m_controller->actionForView(m_view)->shortcuts());
            auto* dialogLayout = new QVBoxLayout(dialog);
            dialogLayout->addWidget(w);
            auto* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
            dialogLayout->addWidget(buttonBox);
            connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
            connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

            if (dialog->exec() == QDialog::Accepted) {
                m_controller->actionForView(m_view)->setShortcuts(w->shortcut());

                //save shortcut config
                KConfigGroup config = KSharedConfig::openConfig()->group(QStringLiteral("UI"));
                const QStringList shortcuts{
                    w->shortcut().value(0).toString(),
                    w->shortcut().value(1).toString(),
                };
                config.writeEntry(QStringLiteral("Shortcut for %1").arg(m_view->document()->title()), shortcuts);
                config.sync();
            }
            delete dialog;

            return;
        } else if ( triggered == detach ) {
            setFloating(true);
            return;
        }

        Qt::DockWidgetArea dockingArea;
        if (triggered == left)
            dockingArea = Qt::LeftDockWidgetArea;
        else if (triggered == bottom)
            dockingArea = Qt::BottomDockWidgetArea;
        else if (triggered == right)
            dockingArea = Qt::RightDockWidgetArea;
        else
            return;

        // Attach this dock widget to dockingArea without changing its visibility.

        const auto isVisible = this->isVisible();

        auto* const mainWindow = qobject_cast<QMainWindow*>(parentWidget());
        Q_ASSERT(mainWindow);

        // The addDockWidget() and removeDockWidget() calls in IdealController::showDockWidget() mean that an
        // invisible dock widget is not associated with any dock widget area as far as QMainWindow is concerned.
        // Therefore, if this dock widget is invisible, the attaching via setFloating(false) below
        // has effect only if addDockWidget() is called first, even if dockingArea equals m_docking_area.

        if (dockingArea != m_docking_area || !isVisible) {
            // The two-argument overload of QMainWindow::addDockWidget() removes the dock widget from
            // the main window layout if it is already there, i.e. properly moves to the new area.
            mainWindow->addDockWidget(dockingArea, this);
        }

        if (isFloating()) {
            setFloating(false);
        }

        if (!isVisible) {
            // Restore the invariant that an invisible dock widget is absent from the main window layout.
            mainWindow->removeDockWidget(this);
        }
    }
}

#include "moc_idealdockwidget.cpp"
