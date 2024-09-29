/*
    SPDX-FileCopyrightText: David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "workingsettooltipwidget.h"

#include <KLocalizedString>

#include <QPushButton>
#include <QVBoxLayout>

#include "debug_workingset.h"
#include "core.h"
#include "documentcontroller.h"
#include "mainwindow.h"

#include <sublime/view.h>
#include <sublime/mainwindow.h>
#include <sublime/area.h>
#include <sublime/document.h>

#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <util/activetooltip.h>
#include <util/algorithm.h>

#include "workingset.h"
#include "workingsetcontroller.h"
#include "workingsetfilelabel.h"
#include "workingsettoolbutton.h"
#include "workingsethelpers.h"

using namespace KDevelop;

class FileWidget : public QWidget
{
    Q_OBJECT

public:
    QToolButton* m_button;
    class WorkingSetFileLabel* m_label;
};

WorkingSetToolTipWidget::WorkingSetToolTipWidget(QWidget* parent, WorkingSet* set, MainWindow* mainwindow) : QWidget(parent), m_set(set) {
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(0);

    layout->setContentsMargins(0, 0, 0, 0);

    connect(mainwindow->area(),
            &Sublime::Area::viewAdded, this, &WorkingSetToolTipWidget::updateFileButtons,
            Qt::QueuedConnection);
    connect(mainwindow->area(),
            &Sublime::Area::viewRemoved, this, &WorkingSetToolTipWidget::updateFileButtons,
            Qt::QueuedConnection);

    connect(Core::self()->workingSetControllerInternal(), &WorkingSetController::workingSetSwitched,
            this, &WorkingSetToolTipWidget::updateFileButtons);

    // title bar
    {
        auto* topLayout = new QHBoxLayout;
        m_setButton = new WorkingSetToolButton(this, set);
        m_setButton->hide();

        topLayout->addSpacing(5);
        auto* icon = new QLabel;
        topLayout->addWidget(icon);
        topLayout->addSpacing(5);

        QString label;
        if (m_set->isConnected(mainwindow->area())) {
            label = i18n("<b>Active Working Set</b>");
        } else {
            label = i18n("<b>Working Set</b>");
        }
        auto* name = new QLabel(label);
        name->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        topLayout->addWidget(name);
         topLayout->addSpacing(10);

        icon->setPixmap(m_setButton->icon().pixmap(name->sizeHint().height()+8, name->sizeHint().height()+8));

        topLayout->addStretch();

        m_openButton = new QPushButton;
        m_openButton->setFlat(true);
        topLayout->addWidget(m_openButton);

        m_deleteButton = new QPushButton;
        m_deleteButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
        m_deleteButton->setText(i18nc("@action:button", "Delete"));
        m_deleteButton->setToolTip(i18nc("@info:tooltip", "Remove this working set. The contained documents are not affected."));
        m_deleteButton->setFlat(true);
        connect(m_deleteButton, &QPushButton::clicked, m_set, [&] { m_set->deleteSet(false); });
        connect(m_deleteButton, &QPushButton::clicked, this, &WorkingSetToolTipWidget::shouldClose);
        topLayout->addWidget(m_deleteButton);
        layout->addLayout(topLayout);
        // horizontal line
        auto* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Raised);
        layout->addWidget(line);
    }

    // everything else is added to the following widget which just has a different background color
    auto* bodyLayout = new QVBoxLayout;
    {
        auto* body = new QWidget();
        body->setLayout(bodyLayout);
        layout->addWidget(body);
        body->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    }

    // document list actions
    {
        auto* actionsLayout = new QHBoxLayout;

        m_documentsLabel = new QLabel(i18nc("@label", "Documents:"));
        m_documentsLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        actionsLayout->addWidget(m_documentsLabel);

        actionsLayout->addStretch();

        m_mergeButton = new QPushButton;
        m_mergeButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
        m_mergeButton->setText(i18nc("@action:button", "Add All"));
        m_mergeButton->setToolTip(i18nc("@info:tooltip", "Add all documents that are part of this working set to the currently active working set"));
        m_mergeButton->setFlat(true);
        connect(m_mergeButton, &QPushButton::clicked, m_setButton, &WorkingSetToolButton::mergeSet);
        actionsLayout->addWidget(m_mergeButton);

        m_subtractButton = new QPushButton;
        m_subtractButton->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
        m_subtractButton->setText(i18nc("@action:button", "Remove All"));
        m_subtractButton->setToolTip(i18nc("@info:tooltip", "Remove all documents that are part of this working set from the currently active working set"));
        m_subtractButton->setFlat(true);
        connect(m_subtractButton, &QPushButton::clicked, m_setButton, &WorkingSetToolButton::subtractSet);
        actionsLayout->addWidget(m_subtractButton);
        bodyLayout->addLayout(actionsLayout);
    }

    QSet<QString> hadFiles;

    auto* filesLayout = new QVBoxLayout;
    filesLayout->setContentsMargins(0, 0, 0, 0);

    const auto setFiles = m_set->fileList();
    for (const QString& file : setFiles) {
        if (!Algorithm::insert(hadFiles, file).inserted) {
            continue;
        }

        auto* widget = new FileWidget;
        widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

        auto* fileLayout = new QHBoxLayout(widget);

        auto* plusButton = new QToolButton;
        plusButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
        fileLayout->addWidget(plusButton);

        auto* fileLabel = new WorkingSetFileLabel;
        fileLabel->setTextFormat(Qt::RichText);
        // We add spaces behind and after, to make it look nicer
        fileLabel->setText(QLatin1String("&nbsp;") + Core::self()->projectController()->prettyFileName(QUrl::fromUserInput(file)) + QLatin1String("&nbsp;"));
        fileLabel->setToolTip(i18nc("@info:tooltip", "Click to open and activate this document."));
        fileLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        fileLayout->addWidget(fileLabel);
        fileLayout->setContentsMargins(0, 0, 0, 0);

        plusButton->setMaximumHeight(fileLabel->sizeHint().height() + 4);
        plusButton->setMaximumWidth(plusButton->maximumHeight());

        plusButton->setObjectName(file);
        fileLabel->setObjectName(file);
        fileLabel->setCursor(QCursor(Qt::PointingHandCursor));

        widget->m_button = plusButton;
        widget->m_label = fileLabel;

        filesLayout->addWidget(widget);
        m_fileWidgets.insert(file, widget);
        m_orderedFileWidgets.push_back(widget);

        connect(plusButton, &QToolButton::clicked, this, &WorkingSetToolTipWidget::buttonClicked);
        connect(fileLabel, &WorkingSetFileLabel::clicked, this, &WorkingSetToolTipWidget::labelClicked);
    }

    bodyLayout->addLayout(filesLayout);

    updateFileButtons();
    connect(set, &WorkingSet::setChangedSignificantly, this, &WorkingSetToolTipWidget::updateFileButtons);
    connect(mainwindow->area(), &Sublime::Area::changedWorkingSet, this, &WorkingSetToolTipWidget::updateFileButtons, Qt::QueuedConnection);

    QMetaObject::invokeMethod(this, &WorkingSetToolTipWidget::updateFileButtons);
}

void WorkingSetToolTipWidget::nextDocument()
{
    int active = -1;
    for(int a = 0; a < m_orderedFileWidgets.size(); ++a)
        if(m_orderedFileWidgets[a]->m_label->isActive())
            active = a;

    if(active == -1)
    {
        qCWarning(WORKINGSET) << "Found no active document";
        return;
    }

    int next = (active + 1) % m_orderedFileWidgets.size();
    while(m_orderedFileWidgets[next]->isHidden() && next != active)
        next = (next + 1) % m_orderedFileWidgets.size();

    m_orderedFileWidgets[next]->m_label->emitClicked();
}

void WorkingSetToolTipWidget::previousDocument()
{
    int active = -1;
    for(int a = 0; a < m_orderedFileWidgets.size(); ++a)
        if(m_orderedFileWidgets[a]->m_label->isActive())
            active = a;

    if(active == -1)
    {
        qCWarning(WORKINGSET) << "Found no active document";
        return;
    }

    int next = active - 1;
    if(next < 0)
        next += m_orderedFileWidgets.size();

    while(m_orderedFileWidgets[next]->isHidden() && next != active)
    {
        next -= 1;
        if(next < 0)
            next += m_orderedFileWidgets.size();
    }

    m_orderedFileWidgets[next]->m_label->emitClicked();
}


void WorkingSetToolTipWidget::updateFileButtons()
{
    auto* mainWindow = qobject_cast<MainWindow*>(Core::self()->uiController()->activeMainWindow());
    Q_ASSERT(mainWindow);

    WorkingSetController* controller = Core::self()->workingSetControllerInternal();
    ActiveToolTip* tooltip = controller->tooltip();

    QString activeFile;
    if(mainWindow->area()->activeView())
        activeFile = mainWindow->area()->activeView()->document()->documentSpecifier();

    WorkingSet* currentWorkingSet = nullptr;
    QSet<QString> openFiles;

    if(!mainWindow->area()->workingSet().isEmpty())
    {
        currentWorkingSet = controller->workingSet(mainWindow->area()->workingSet());
        openFiles = currentWorkingSet->fileSet();
    }

    bool allOpen = true;
    bool noneOpen = true;

    bool needResize = false;

    bool allHidden = true;

    for(QMap< QString, FileWidget* >::iterator it = m_fileWidgets.begin(); it != m_fileWidgets.end(); ++it) {
        if(openFiles.contains(it.key())) {
            noneOpen = false;
            (*it)->m_button->setToolTip(i18nc("@info:tooltip", "Remove this file from the current working set"));
            (*it)->m_button->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
            (*it)->show();
        }else{
            allOpen = false;
            (*it)->m_button->setToolTip(i18nc("@info:tooltip", "Add this file to the current working set"));
            (*it)->m_button->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
            if(currentWorkingSet == m_set)
            {
                (*it)->hide();
                needResize = true;
            }
        }


        if(!(*it)->isHidden())
            allHidden = false;

        (*it)->m_label->setIsActiveFile(it.key() == activeFile);
    }

    // NOTE: always hide merge&subtract all on current working set
    // if we want to enable mergeButton, we have to fix it's behavior since it operates directly on the
    // set contents and not on the m_fileWidgets
    m_mergeButton->setHidden(allOpen || currentWorkingSet == m_set);
    m_subtractButton->setHidden(noneOpen || currentWorkingSet == m_set);
    m_deleteButton->setHidden(m_set->hasConnectedAreas());
    m_documentsLabel->setHidden(m_mergeButton->isHidden() && m_subtractButton->isHidden() && m_deleteButton->isHidden());

    if(currentWorkingSet == m_set) {
        disconnect(m_openButton, &QPushButton::clicked, m_setButton, &WorkingSetToolButton::loadSet);
        connect(m_openButton, &QPushButton::clicked, m_setButton, &WorkingSetToolButton::closeSet);
        connect(m_openButton, &QPushButton::clicked, this, &WorkingSetToolTipWidget::shouldClose);
        m_openButton->setIcon(QIcon::fromTheme(QStringLiteral("project-development-close")));
        m_openButton->setText(i18nc("@action:button", "Stash"));
    }else{
        disconnect(m_openButton, &QPushButton::clicked, m_setButton, &WorkingSetToolButton::closeSet);
        connect(m_openButton, &QPushButton::clicked, m_setButton, &WorkingSetToolButton::loadSet);
        disconnect(m_openButton, &QPushButton::clicked, this, &WorkingSetToolTipWidget::shouldClose);
        m_openButton->setIcon(QIcon::fromTheme(QStringLiteral("project-open")));
        m_openButton->setText(i18nc("@action:button", "Load"));
    }

    if(allHidden && tooltip)
        tooltip->hide();

    if(needResize && tooltip)
        tooltip->resize(tooltip->sizeHint());
}

void WorkingSetToolTipWidget::buttonClicked(bool)
{
    QPointer<WorkingSetToolTipWidget> stillExists(this);

    auto* s = qobject_cast<QToolButton*>(sender());
    Q_ASSERT(s);

    auto* mainWindow = qobject_cast<MainWindow*>(Core::self()->uiController()->activeMainWindow());
    Q_ASSERT(mainWindow);
    QSet<QString> openFiles = Core::self()->workingSetControllerInternal()->workingSet(mainWindow->area()->workingSet())->fileSet();

    if (openFiles.remove(s->objectName())) {
        filterViews(openFiles);
    } else {
        Core::self()->documentControllerInternal()->openDocument(QUrl::fromUserInput(s->objectName()));
    }

    if(stillExists)
        updateFileButtons();
}

void WorkingSetToolTipWidget::labelClicked()
{
    QPointer<WorkingSetToolTipWidget> stillExists(this);

    auto* s = qobject_cast<WorkingSetFileLabel*>(sender());
    Q_ASSERT(s);

    bool found = false;

    auto* window = static_cast<Sublime::MainWindow*>(ICore::self()->uiController()->activeMainWindow());

    const auto views = window->area()->views();
    for (Sublime::View* view : views) {
        if(view->document()->documentSpecifier() == s->objectName())
        {
            window->activateView(view);
            found = true;
            break;
        }
    }

    if(!found)
        Core::self()->documentControllerInternal()->openDocument(QUrl::fromUserInput(s->objectName()));

    if(stillExists)
        updateFileButtons();
}

#include "workingsettooltipwidget.moc"
#include "moc_workingsettooltipwidget.cpp"
