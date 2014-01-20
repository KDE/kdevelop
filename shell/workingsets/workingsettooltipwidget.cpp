/*
    Copyright David Nolden  <david.nolden.kdevelop@art-master.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "workingsettooltipwidget.h"

#include <KLocalizedString>
#include <KDebug>

#include <QPushButton>
#include <QVBoxLayout>

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

#include "workingset.h"
#include "workingsetcontroller.h"
#include "workingsetfilelabel.h"
#include "workingsettoolbutton.h"
#include "workingsethelpers.h"

using namespace KDevelop;

WorkingSetToolTipWidget::WorkingSetToolTipWidget(QWidget* parent, WorkingSet* set, MainWindow* mainwindow) : QWidget(parent), m_set(set) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);

    layout->setMargin(0);

    connect(static_cast<Sublime::MainWindow*>(mainwindow)->area(),
            SIGNAL(viewAdded(Sublime::AreaIndex*,Sublime::View*)), SLOT(updateFileButtons()),
            Qt::QueuedConnection);
    connect(static_cast<Sublime::MainWindow*>(mainwindow)->area(),
            SIGNAL(viewRemoved(Sublime::AreaIndex*,Sublime::View*)), SLOT(updateFileButtons()),
            Qt::QueuedConnection);

    connect(Core::self()->workingSetControllerInternal(), SIGNAL(workingSetSwitched()),
            SLOT(updateFileButtons()));

    // title bar
    {
        QHBoxLayout* topLayout = new QHBoxLayout;
        m_setButton = new WorkingSetToolButton(this, set);
        m_setButton->hide();

        topLayout->addSpacing(5);
        QLabel* icon = new QLabel;
        topLayout->addWidget(icon);
        topLayout->addSpacing(5);

        QString label;
        if (m_set->isConnected(mainwindow->area())) {
            label = i18n("<b>Active Working Set</b>");
        } else {
            label = i18n("<b>Working Set</b>");
        }
        QLabel* name = new QLabel(label);
        name->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        topLayout->addWidget(name);
         topLayout->addSpacing(10);

        icon->setPixmap(m_setButton->icon().pixmap(name->sizeHint().height()+8, name->sizeHint().height()+8));

        topLayout->addStretch();

        m_openButton = new QPushButton;
        m_openButton->setFlat(true);
        topLayout->addWidget(m_openButton);

        m_deleteButton = new QPushButton;
        m_deleteButton->setIcon(QIcon::fromTheme("edit-delete"));
        m_deleteButton->setText(i18n("Delete"));
        m_deleteButton->setToolTip(i18n("Remove this working set. The contained documents are not affected."));
        m_deleteButton->setFlat(true);
        connect(m_deleteButton, SIGNAL(clicked(bool)), m_set, SLOT(deleteSet()));
        connect(m_deleteButton, SIGNAL(clicked(bool)), this, SIGNAL(shouldClose()));
        topLayout->addWidget(m_deleteButton);
        layout->addLayout(topLayout);
        // horizontal line
        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Raised);
        layout->addWidget(line);
    }

    // everything else is added to the following widget which just has a different background color
    QVBoxLayout* bodyLayout = new QVBoxLayout;
    {
        QWidget* body = new QWidget();
        body->setLayout(bodyLayout);
        layout->addWidget(body);
        body->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    }

    // document list actions
    {
        QHBoxLayout* actionsLayout = new QHBoxLayout;

        m_documentsLabel = new QLabel(i18n("Documents:"));
        m_documentsLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        actionsLayout->addWidget(m_documentsLabel);

        actionsLayout->addStretch();

        m_mergeButton = new QPushButton;
        m_mergeButton->setIcon(QIcon::fromTheme("list-add"));
        m_mergeButton->setText(i18n("Add All"));
        m_mergeButton->setToolTip(i18n("Add all documents that are part of this working set to the currently active working set."));
        m_mergeButton->setFlat(true);
        connect(m_mergeButton, SIGNAL(clicked(bool)), m_setButton, SLOT(mergeSet()));
        actionsLayout->addWidget(m_mergeButton);

        m_subtractButton = new QPushButton;
        m_subtractButton->setIcon(QIcon::fromTheme("list-remove"));
        m_subtractButton->setText(i18n("Remove All"));
        m_subtractButton->setToolTip(i18n("Remove all documents that are part of this working set from the currently active working set."));
        m_subtractButton->setFlat(true);
        connect(m_subtractButton, SIGNAL(clicked(bool)), m_setButton, SLOT(subtractSet()));
        actionsLayout->addWidget(m_subtractButton);
        bodyLayout->addLayout(actionsLayout);
    }

    QSet<QString> hadFiles;

    QVBoxLayout* filesLayout = new QVBoxLayout;
    filesLayout->setMargin(0);

    foreach(const QString& file, m_set->fileList()) {

        if(hadFiles.contains(file))
            continue;

        hadFiles.insert(file);

        FileWidget* widget = new FileWidget;
        widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

        QHBoxLayout* fileLayout = new QHBoxLayout(widget);

        QToolButton* plusButton = new QToolButton;
        plusButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
        fileLayout->addWidget(plusButton);

        WorkingSetFileLabel* fileLabel = new WorkingSetFileLabel;
        fileLabel->setTextFormat(Qt::RichText);
        // We add spaces behind and after, to make it look nicer
        fileLabel->setText("&nbsp;" + Core::self()->projectController()->prettyFileName(KUrl(file)) + "&nbsp;");
        fileLabel->setToolTip(i18nc("@info:tooltip", "Click to open and activate this document."));
        fileLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        fileLayout->addWidget(fileLabel);
        fileLayout->setMargin(0);

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

        connect(plusButton, SIGNAL(clicked(bool)), this, SLOT(buttonClicked(bool)));
        connect(fileLabel, SIGNAL(clicked()), this, SLOT(labelClicked()));
    }

    bodyLayout->addLayout(filesLayout);

    updateFileButtons();
    connect(set, SIGNAL(setChangedSignificantly()), SLOT(updateFileButtons()));
    connect(mainwindow->area(), SIGNAL(changedWorkingSet(Sublime::Area*,QString,QString)), SLOT(updateFileButtons()), Qt::QueuedConnection);

    QMetaObject::invokeMethod(this, "updateFileButtons");
}

void WorkingSetToolTipWidget::nextDocument()
{
    int active = -1;
    for(int a = 0; a < m_orderedFileWidgets.size(); ++a)
        if(m_orderedFileWidgets[a]->m_label->isActive())
            active = a;

    if(active == -1)
    {
        kWarning() << "Found no active document";
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
        kWarning() << "Found no active document";
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
    MainWindow* mainWindow = dynamic_cast<MainWindow*>(Core::self()->uiController()->activeMainWindow());
    Q_ASSERT(mainWindow);

    WorkingSetController* controller = Core::self()->workingSetControllerInternal();
    ActiveToolTip* tooltip = controller->tooltip();

    QString activeFile;
    if(mainWindow->area()->activeView())
        activeFile = mainWindow->area()->activeView()->document()->documentSpecifier();

    WorkingSet* currentWorkingSet = 0;
    QSet<QString> openFiles;

    if(!mainWindow->area()->workingSet().isEmpty())
    {
        currentWorkingSet = controller->getWorkingSet(mainWindow->area()->workingSet());
        openFiles = currentWorkingSet->fileList().toSet();
    }
    
    bool allOpen = true;
    bool noneOpen = true;

    bool needResize = false;

    bool allHidden = true;

    for(QMap< QString, FileWidget* >::iterator it = m_fileWidgets.begin(); it != m_fileWidgets.end(); ++it) {
        if(openFiles.contains(it.key())) {
            noneOpen = false;
            (*it)->m_button->setToolTip(i18n("Remove this file from the current working set"));
            (*it)->m_button->setIcon(QIcon::fromTheme("list-remove"));
            (*it)->show();
        }else{
            allOpen = false;
            (*it)->m_button->setToolTip(i18n("Add this file to the current working set"));
            (*it)->m_button->setIcon(QIcon::fromTheme("list-add"));
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
        disconnect(m_openButton, SIGNAL(clicked(bool)), m_setButton, SLOT(loadSet()));
        connect(m_openButton, SIGNAL(clicked(bool)), m_setButton, SLOT(closeSet()));
        connect(m_openButton, SIGNAL(clicked(bool)), this, SIGNAL(shouldClose()));
        m_openButton->setIcon(QIcon::fromTheme("project-development-close"));
        m_openButton->setText(i18n("Close"));
    }else{
        disconnect(m_openButton, SIGNAL(clicked(bool)), m_setButton, SLOT(closeSet()));
        connect(m_openButton, SIGNAL(clicked(bool)), m_setButton, SLOT(loadSet()));
        disconnect(m_openButton, SIGNAL(clicked(bool)), this, SIGNAL(shouldClose()));
        m_openButton->setIcon(QIcon::fromTheme("project-open"));
        m_openButton->setText(i18n("Load"));
    }

    if(allHidden && tooltip)
        tooltip->hide();

    if(needResize && tooltip)
        tooltip->resize(tooltip->sizeHint());
}

void WorkingSetToolTipWidget::buttonClicked(bool)
{
    QPointer<WorkingSetToolTipWidget> stillExists(this);

    QToolButton* s = qobject_cast<QToolButton*>(sender());
    Q_ASSERT(s);

    MainWindow* mainWindow = dynamic_cast<MainWindow*>(Core::self()->uiController()->activeMainWindow());
    Q_ASSERT(mainWindow);
    QSet<QString> openFiles = Core::self()->workingSetControllerInternal()->getWorkingSet(mainWindow->area()->workingSet())->fileList().toSet();

    if(!openFiles.contains(s->objectName())) {
        Core::self()->documentControllerInternal()->openDocument(s->objectName());
    }else{
        openFiles.remove(s->objectName());
        filterViews(openFiles);
    }

    if(stillExists)
        updateFileButtons();
}

void WorkingSetToolTipWidget::labelClicked()
{
    QPointer<WorkingSetToolTipWidget> stillExists(this);

    WorkingSetFileLabel* s = qobject_cast<WorkingSetFileLabel*>(sender());
    Q_ASSERT(s);

    bool found = false;

    Sublime::MainWindow* window = static_cast<Sublime::MainWindow*>(ICore::self()->uiController()->activeMainWindow());

    foreach(Sublime::View* view, window->area()->views())
    {
        if(view->document()->documentSpecifier() == s->objectName())
        {
            window->activateView(view);
            found = true;
            break;
        }
    }

    if(!found)
        Core::self()->documentControllerInternal()->openDocument(s->objectName());

    if(stillExists)
        updateFileButtons();
}

#include "workingsettooltipwidget.moc"
