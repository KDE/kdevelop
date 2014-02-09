/**************************************************************************
*   Copyright 2010 Silvère Lestang <silvere.lestang@gmail.com>            *
*   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "grepoutputview.h"
#include "grepoutputmodel.h"
#include "grepoutputdelegate.h"
#include "ui_grepoutputview.h"
#include "grepviewplugin.h"
#include "grepdialog.h"
#include "greputil.h"
#include "grepjob.h"

#include <QtGui/QAction>
#include <QtGui/QStringListModel>
#include <KMessageBox>
#include <KColorScheme>
#include <kdebug.h>
#include <QMenu>

#include <interfaces/icore.h>
#include <interfaces/isession.h>
#include <QWidgetAction>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iplugincontroller.h>

using namespace KDevelop;

GrepOutputViewFactory::GrepOutputViewFactory(GrepViewPlugin* plugin)
: m_plugin(plugin)
{}

QWidget* GrepOutputViewFactory::create(QWidget* parent)
{
    return new GrepOutputView(parent, m_plugin);
}

Qt::DockWidgetArea GrepOutputViewFactory::defaultPosition()
{
    return Qt::BottomDockWidgetArea;
}

QString GrepOutputViewFactory::id() const
{
    return "org.kdevelop.GrepOutputView";
}


const int GrepOutputView::HISTORY_SIZE = 5;

GrepOutputView::GrepOutputView(QWidget* parent, GrepViewPlugin* plugin)
  : QWidget(parent)
  , m_next(0)
  , m_prev(0)
  , m_collapseAll(0)
  , m_expandAll(0)
  , m_clearSearchHistory(0)
  , m_statusLabel(0)
  , m_plugin(plugin)
{
    Ui::GrepOutputView::setupUi(this);

    setWindowTitle(i18nc("@title:window", "Find/Replace Output View"));
    setWindowIcon(SmallIcon("edit-find"));
    
    m_prev = new QAction(KIcon("go-previous"), i18n("&Previous Item"), this);
    m_prev->setEnabled(false);
    m_next = new QAction(KIcon("go-next"), i18n("&Next Item"), this);
    m_next->setEnabled(false);
    m_collapseAll = new QAction(KIcon("arrow-left-double"), i18n("C&ollapse All"), this); // TODO change icon
    m_collapseAll->setEnabled(false);
    m_expandAll = new QAction(KIcon("arrow-right-double"), i18n("&Expand All"), this); // TODO change icon
    m_expandAll->setEnabled(false);
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    QAction *change_criteria = new QAction(KIcon("configure"), i18n("&Change Search Settings"), this);
    m_clearSearchHistory = new QAction(KIcon("edit-clear-list"), i18n("Clear Search History"), this);
    
    addAction(m_prev);
    addAction(m_next);
    addAction(m_collapseAll);
    addAction(m_expandAll);
    addAction(separator);
    addAction(change_criteria);
    addAction(m_clearSearchHistory);
    
    separator = new QAction(this);
    separator->setSeparator(true);
    addAction(separator);
    
    QWidgetAction *statusWidget = new QWidgetAction(this);
    m_statusLabel = new QLabel(this);
    statusWidget->setDefaultWidget(m_statusLabel);
    addAction(statusWidget);
    
    modelSelector->setEditable(false);
    modelSelector->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(modelSelector, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(modelSelectorContextMenu(QPoint)));
    connect(modelSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(changeModel(int)));
    
    resultsTreeView->setItemDelegate(GrepOutputDelegate::self());
    resultsTreeView->setHeaderHidden(true);
    resultsTreeView->setUniformRowHeights(false);
    resultsTreeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(m_prev, SIGNAL(triggered(bool)), this, SLOT(selectPreviousItem()));
    connect(m_next, SIGNAL(triggered(bool)), this, SLOT(selectNextItem()));
    connect(m_collapseAll, SIGNAL(triggered(bool)), this, SLOT(collapseAllItems()));
    connect(m_expandAll, SIGNAL(triggered(bool)), this, SLOT(expandAllItems()));
    connect(applyButton, SIGNAL(clicked()),  this, SLOT(onApply()));
    connect(m_clearSearchHistory, SIGNAL(triggered(bool)), this, SLOT(clearSearchHistory()));
    connect(resultsTreeView, SIGNAL(collapsed(QModelIndex)), this, SLOT(updateScrollArea(QModelIndex)));
    connect(resultsTreeView, SIGNAL(expanded(QModelIndex)), this, SLOT(updateScrollArea(QModelIndex)));

    IPlugin *outputView = ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
    connect(outputView, SIGNAL(selectPrevItem()), this, SLOT(selectPreviousItem()));
    connect(outputView, SIGNAL(selectNextItem()), this, SLOT(selectNextItem()));

    KConfigGroup cg = ICore::self()->activeSession()->config()->group( "GrepDialog" );
    replacementCombo->addItems( cg.readEntry("LastReplacementItems", QStringList()) );
    replacementCombo->setInsertPolicy(QComboBox::InsertAtTop);
    applyButton->setIcon(KIcon("dialog-ok-apply"));
    
    connect(replacementCombo, SIGNAL(editTextChanged(QString)), SLOT(replacementTextChanged(QString)));
    
    connect(change_criteria, SIGNAL(triggered(bool)), this, SLOT(showDialog()));
    
    updateCheckable();
}

void GrepOutputView::replacementTextChanged(QString)
{
    updateCheckable();

    if (model()) {
        // see https://bugs.kde.org/show_bug.cgi?id=274902 - renewModel can trigger a call here without an active model
        updateApplyState(model()->index(0, 0), model()->index(0, 0));
    }
}

GrepOutputView::~GrepOutputView()
{
    KConfigGroup cg = ICore::self()->activeSession()->config()->group( "GrepDialog" );
    cg.writeEntry("LastReplacementItems", qCombo2StringList(replacementCombo, true));
    emit outputViewIsClosed();
}

GrepOutputModel* GrepOutputView::renewModel(QString name, QString descriptionOrUrl)
{
    // Crear oldest model
    while(modelSelector->count() > GrepOutputView::HISTORY_SIZE) {
        QVariant var = modelSelector->itemData(GrepOutputView::HISTORY_SIZE - 1);
        qvariant_cast<QObject*>(var)->deleteLater();
        modelSelector->removeItem(GrepOutputView::HISTORY_SIZE - 1);
    }

    replacementCombo->clearEditText();

    GrepOutputModel* newModel = new GrepOutputModel(resultsTreeView);
    applyButton->setEnabled(false);
    // text may be already present
    newModel->setReplacement(replacementCombo->currentText());
    connect(newModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(rowsRemoved()));
    connect(resultsTreeView, SIGNAL(activated(QModelIndex)), newModel, SLOT(activate(QModelIndex)));
    connect(replacementCombo, SIGNAL(editTextChanged(QString)), newModel, SLOT(setReplacement(QString)));
    connect(newModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(expandElements(QModelIndex)));
    connect(newModel, SIGNAL(showErrorMessage(QString,int)), this, SLOT(showErrorMessage(QString)));
    
    QString prettyUrl = descriptionOrUrl;
    if(descriptionOrUrl.startsWith('/'))
        prettyUrl = ICore::self()->projectController()->prettyFileName(descriptionOrUrl, KDevelop::IProjectController::FormatPlain);
    
    // appends new model to history
    QString displayName = i18n("Search \"%1\" in %2 (at time %3)", name, prettyUrl, QTime::currentTime().toString("hh:mm"));
    modelSelector->insertItem(0, displayName, qVariantFromValue<QObject*>(newModel));
    
    modelSelector->setCurrentIndex(0);//setCurrentItem(displayName);
    
    updateCheckable();
    
    return newModel;
}


GrepOutputModel* GrepOutputView::model()
{
    return static_cast<GrepOutputModel*>(resultsTreeView->model());
}

void GrepOutputView::changeModel(int index)
{
    disconnect(model(), SIGNAL(showMessage(KDevelop::IStatus*,QString)), 
               this, SLOT(showMessage(KDevelop::IStatus*,QString)));
    disconnect(model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), 
               this, SLOT(updateApplyState(QModelIndex,QModelIndex)));

    replacementCombo->clearEditText();
    
    //after deleting the whole search history, index is -1
    if(index >= 0)
    {
        QVariant var = modelSelector->itemData(index);
        GrepOutputModel *resultModel = static_cast<GrepOutputModel *>(qvariant_cast<QObject*>(var));
        resultsTreeView->setModel(resultModel);
        
        connect(model(), SIGNAL(showMessage(KDevelop::IStatus*,QString)), 
                this, SLOT(showMessage(KDevelop::IStatus*,QString)));
        connect(model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), 
                this, SLOT(updateApplyState(QModelIndex,QModelIndex)));
        model()->showMessageEmit();
        applyButton->setEnabled(model()->hasResults() && 
                                model()->getRootItem() && 
                                model()->getRootItem()->checkState() != Qt::Unchecked &&
                                !replacementCombo->currentText().isEmpty());
        if(model()->hasResults())
            expandElements(QModelIndex());
    }
    
    updateCheckable();
    updateApplyState(model()->index(0, 0), model()->index(0, 0));
}

void GrepOutputView::setMessage(const QString& msg, MessageType type)
{
    if (type == Error) {
        QPalette palette = m_statusLabel->palette();
        KColorScheme::adjustForeground(palette, KColorScheme::NegativeText, QPalette::WindowText);
        m_statusLabel->setPalette(palette);
    } else {
        m_statusLabel->setPalette(QPalette());
    }
    m_statusLabel->setText(msg);
}

void GrepOutputView::showErrorMessage( const QString& errorMessage )
{
    setMessage(errorMessage, Error);
}

void GrepOutputView::showMessage( KDevelop::IStatus* , const QString& message )
{
    setMessage(message, Information);
}

void GrepOutputView::onApply()
{
    if(model()) 
    {
        Q_ASSERT(model()->rowCount());
        // ask a confirmation before an empty string replacement
        if(replacementCombo->currentText().length() == 0 &&
           KMessageBox::questionYesNo(this, i18n("Do you want to replace with an empty string?"),
                                            i18n("Start replacement")) == KMessageBox::No)
        {
            return;
        }

        setEnabled(false);
        model()->doReplacements();
        setEnabled(true);
    }
}

void GrepOutputView::showDialog()
{
    m_plugin->showDialog(true);
}

void GrepOutputView::expandElements(const QModelIndex&)
{
    m_prev->setEnabled(true);
    m_next->setEnabled(true);
    m_collapseAll->setEnabled(true);
    m_expandAll->setEnabled(true);
    
    resultsTreeView->expandAll();
    for (int col = 0; col < model()->columnCount(); ++col)
        resultsTreeView->resizeColumnToContents(col);
}

void GrepOutputView::selectPreviousItem()
{
    if (!model()) {
        return;
    }

    QModelIndex prev_idx = model()->previousItemIndex(resultsTreeView->currentIndex());
    if (prev_idx.isValid()) {
        resultsTreeView->setCurrentIndex(prev_idx);
        model()->activate(prev_idx);
    }
}

void GrepOutputView::selectNextItem()
{
    if (!model()) {
        return;
    }

    QModelIndex next_idx = model()->nextItemIndex(resultsTreeView->currentIndex());
    if (next_idx.isValid()) {
        resultsTreeView->setCurrentIndex(next_idx);
        model()->activate(next_idx);
    }
}


void GrepOutputView::collapseAllItems()
{
    // Collapse everything
    resultsTreeView->collapseAll();

    // Now reopen the first children, which correspond to the files.
    resultsTreeView->expand(resultsTreeView->model()->index(0, 0));
}

void GrepOutputView::expandAllItems()
{
    resultsTreeView->expandAll();
}
    
void GrepOutputView::rowsRemoved()
{
    m_prev->setEnabled(model()->rowCount());
    m_next->setEnabled(model()->rowCount());
}

void GrepOutputView::updateApplyState(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    Q_UNUSED(bottomRight);

    if (!model() || !model()->hasResults()) {
        applyButton->setEnabled(false);
        return;
    }

    // we only care about the root item
    if(!topLeft.parent().isValid())
    {
        applyButton->setEnabled(topLeft.data(Qt::CheckStateRole) != Qt::Unchecked && model()->itemsCheckable());
    }
}

void GrepOutputView::updateCheckable()
{
    if(model())
        model()->makeItemsCheckable(!replacementCombo->currentText().isEmpty() || model()->itemsCheckable());
}

void GrepOutputView::clearSearchHistory()
{
    GrepJob *runningJob = m_plugin->grepJob();
    if(runningJob)
    {
        runningJob->kill();
    }
    while(modelSelector->count() > 0)
    {
        QVariant var = modelSelector->itemData(0);
        qvariant_cast<QObject*>(var)->deleteLater();
        modelSelector->removeItem(0);
    }
    applyButton->setEnabled(false);
    m_statusLabel->setText(QString());
}

void GrepOutputView::modelSelectorContextMenu(const QPoint& pos)
{
    QPoint globalPos = modelSelector->mapToGlobal(pos);
    QMenu myMenu;
    myMenu.addAction(m_clearSearchHistory);
    myMenu.exec(globalPos);
}

void GrepOutputView::updateScrollArea(const QModelIndex& index)
{
    resultsTreeView->resizeColumnToContents( index.column() );
}
