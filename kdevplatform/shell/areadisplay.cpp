/*
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "areadisplay.h"

#include "mainwindow.h"
#include "workingsetcontroller.h"
#include "core.h"

#include <sublime/area.h>
#include <interfaces/iuicontroller.h>

#include <KLocalizedString>

#include <QMenu>
#include <QToolButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QEvent>

using namespace KDevelop;

AreaDisplay::AreaDisplay(KDevelop::MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent)
{
    setLayout(new QHBoxLayout);
    m_separator = new QLabel(QStringLiteral("|"), this);
    m_separator->setEnabled(false);
    m_separator->setVisible(false);
    layout()->addWidget(m_separator);

    layout()->setContentsMargins(0, 0, 0, 0);
    auto closedWorkingSets = Core::self()->workingSetControllerInternal()->createSetManagerWidget(m_mainWindow);
    closedWorkingSets->setParent(this);
    layout()->addWidget(closedWorkingSets);

    m_button = new QToolButton(this);
    m_button->setToolTip(i18n(
        "Execute actions to change the area.<br />"
        "An area is a tool view configuration for a specific use case. "
        "From here you can also navigate back to the default code area."));
    m_button->setAutoRaise(true);
    m_button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_button->setPopupMode(QToolButton::InstantPopup);
    layout()->addWidget(m_button);

    connect(parent, &MainWindow::areaChanged, this, &AreaDisplay::newArea);
}

void AreaDisplay::newArea(Sublime::Area* area)
{
    if(m_button->menu())
        m_button->menu()->deleteLater();

    Sublime::Area* currentArea = m_mainWindow->area();

    m_button->setText(currentArea->title());
    m_button->setIcon(QIcon::fromTheme(currentArea->iconName()));

    auto* m = new QMenu(m_button);
    m->addActions(area->actions());
    if (currentArea->objectName() != QLatin1String("code")) {
        if(!m->actions().isEmpty())
            m->addSeparator();
        m->addAction(QIcon::fromTheme(QStringLiteral("document-edit")), i18nc("@action:inmenu", "Back to Code"),
                     QKeySequence(Qt::AltModifier | Qt::Key_Backspace), this, &AreaDisplay::backToCode);
    }
    m_button->setMenu(m);

    //remove the additional widgets we might have added for the last area
    auto* l = qobject_cast<QBoxLayout*>(layout());
    if(l->count()>=4) {
        QLayoutItem* item = l->takeAt(0);
        delete item->widget();
        delete item;
    }
    QWidget* w = Core::self()->workingSetControllerInternal()->createSetManagerWidget(m_mainWindow, area);
    w->installEventFilter(this);
    m_separator->setVisible(w->isVisible());
    l->insertWidget(0, w);
}

bool AreaDisplay::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Show) {
        m_separator->setVisible(true);
        // Recalculate menu bar widget sizes after showing the working set button (not done automatically)
        QMetaObject::invokeMethod(m_mainWindow->menuBar(), &QMenuBar::adjustSize, Qt::QueuedConnection);
    } else if (event->type() == QEvent::Hide) {
        m_separator->setVisible(false);
        QMetaObject::invokeMethod(m_mainWindow->menuBar(), &QMenuBar::adjustSize, Qt::QueuedConnection);
    }

    return QObject::eventFilter(obj, event);
}

void AreaDisplay::backToCode()
{
    auto oldArea = m_mainWindow->area();
    QString workingSet = oldArea->workingSet();
    ICore::self()->uiController()->switchToArea(QStringLiteral("code"), IUiController::ThisWindow);
    m_mainWindow->area()->setWorkingSet(workingSet, oldArea->workingSetPersistent(), oldArea);
}

QSize AreaDisplay::minimumSizeHint() const
{
    QSize hint = QWidget::minimumSizeHint();
    hint = hint.boundedTo(QSize(hint.width(), m_mainWindow->menuBar()->height()-1));
    return hint;
}

QSize AreaDisplay::sizeHint() const
{
    QSize hint = QWidget::sizeHint();
    hint = hint.boundedTo(QSize(hint.width(), m_mainWindow->menuBar()->height()-1));
    return hint;
}

#include "moc_areadisplay.cpp"
