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
#include <kdebug.h>
#include <QMenu>

#include <interfaces/icore.h>
#include <interfaces/isession.h>
#include <QWidgetAction>

using namespace KDevelop;

GrepOutputViewFactory::GrepOutputViewFactory()
{}

QWidget* GrepOutputViewFactory::create(QWidget* parent)
{
    return new GrepOutputView(parent);
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

GrepOutputView::GrepOutputView(QWidget* parent)
  : QWidget(parent)
{
    Ui::GrepOutputView::setupUi(this);

    setWindowTitle(i18n("Replace output view"));
    setWindowIcon(SmallIcon("edit-find"));
    
    m_prev = new QAction(KIcon("go-previous"), i18n("&Previous item"), this);
    m_prev->setEnabled(false);
    m_next = new QAction(KIcon("go-next"), i18n("&Next item"), this);
    m_next->setEnabled(false);
    m_collapseAll = new QAction(KIcon("arrow-left-double"), i18n("C&ollapse all"), this); // TODO change icon
    m_collapseAll->setEnabled(false);
    m_expandAll = new QAction(KIcon("arrow-right-double"), i18n("&Expand all"), this); // TODO change icon
    m_expandAll->setEnabled(false);
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    QAction *change_criteria = new QAction(KIcon("configure"), i18n("&Change criteria"), this);
    m_clearSearchHistory = new QAction(KIcon("edit-clear-list"), i18n("Clear search history"), this);
    
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
    connect(modelSelector, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(modelSelectorContextMenu(const QPoint&)));
    connect(modelSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(changeModel(int)));
    
    resultsTreeView->setItemDelegate(GrepOutputDelegate::self());
    resultsTreeView->setHeaderHidden(true);

    connect(m_prev, SIGNAL(triggered(bool)), this, SLOT(selectPreviousItem()));
    connect(m_next, SIGNAL(triggered(bool)), this, SLOT(selectNextItem()));
    connect(m_collapseAll, SIGNAL(triggered(bool)), this, SLOT(collapseAllItems()));
    connect(m_expandAll, SIGNAL(triggered(bool)), this, SLOT(expandAllItems()));
    connect(applyButton, SIGNAL(clicked()),  this, SLOT(onApply()));
    connect(m_clearSearchHistory, SIGNAL(triggered(bool)), this, SLOT(clearSearchHistory()));
    
    KConfigGroup cg = ICore::self()->activeSession()->config()->group( "GrepDialog" );
    replacementCombo->addItems( cg.readEntry("LastReplacementItems", QStringList()) );
    replacementCombo->setInsertPolicy(QComboBox::InsertAtTop);
    applyButton->setIcon(KIcon("dialog-ok-apply"));
    
    connect(change_criteria, SIGNAL(triggered(bool)), this, SLOT(showDialog()));
}

GrepOutputView::~GrepOutputView()
{
    KConfigGroup cg = ICore::self()->activeSession()->config()->group( "GrepDialog" );
    cg.writeEntry("LastReplacementItems", qCombo2StringList(replacementCombo, true));
    emit outputViewIsClosed();
}

GrepOutputModel* GrepOutputView::renewModel(QString name)
{
    // Crear oldest model
    while(modelSelector->count() > GrepOutputView::HISTORY_SIZE) {
        QVariant var = modelSelector->itemData(GrepOutputView::HISTORY_SIZE - 1);
        qvariant_cast<QObject*>(var)->deleteLater();
        modelSelector->removeItem(GrepOutputView::HISTORY_SIZE - 1);
    }

    GrepOutputModel* newModel = new GrepOutputModel(resultsTreeView);
    applyButton->setEnabled(false);
    // text may be already present
    newModel->setReplacement(replacementCombo->currentText());
    connect(newModel, SIGNAL(rowsRemoved(QModelIndex, int, int)),
            this, SLOT(rowsRemoved()));
    connect(resultsTreeView, SIGNAL(activated(QModelIndex)), newModel, SLOT(activate(QModelIndex)));
    connect(replacementCombo, SIGNAL(editTextChanged(QString)), newModel, SLOT(setReplacement(QString)));
    connect(newModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(expandRootElement(QModelIndex)));
    connect(newModel, SIGNAL(showErrorMessage(QString,int)), this, SLOT(showErrorMessage(QString)));
    
    
    // appends new model to history
    QString displayName = QTime::currentTime().toString("[hh:mm] ")+name;
    modelSelector->insertItem(0, displayName, qVariantFromValue<QObject*>(newModel));
    
    modelSelector->setCurrentIndex(0);//setCurrentItem(displayName);
    
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
                                model()->getRootItem()->checkState() != Qt::Unchecked);
        if(model()->hasResults())
        {
            expandRootElement(QModelIndex());
        }
    }
}

void GrepOutputView::setPlugin(GrepViewPlugin* plugin)
{
    m_plugin = plugin;
}

void GrepOutputView::setMessage(const QString& msg)
{
    m_statusLabel->setText(msg);
}

void GrepOutputView::showErrorMessage( const QString& errorMessage )
{
    setStyleSheet("QLabel { color : red; }");
    setMessage(errorMessage);
}

void GrepOutputView::showMessage( KDevelop::IStatus* , const QString& message )
{
    setStyleSheet("");
    setMessage(message);
}

void GrepOutputView::onApply()
{
    if(model()) 
    {
        Q_ASSERT(model()->rowCount());
        // ask a confirmation before an empty string replacement
        if(replacementCombo->currentText().length() == 0 &&
        KMessageBox::questionYesNo(this, i18n("Would you want to replace by empty string?"),
                                            i18n("Start replacement")) == KMessageBox::ButtonCode::No)
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

void GrepOutputView::expandRootElement(const QModelIndex& parent)
{
    if(!parent.isValid())
    {
        resultsTreeView->setExpanded(model()->index(0,0), true);
    }

    m_prev->setEnabled(true);
    m_next->setEnabled(true);
    m_collapseAll->setEnabled(true);
    m_expandAll->setEnabled(true);
    applyButton->setEnabled(true);
}

void GrepOutputView::selectPreviousItem()
{
    QModelIndex idx = resultsTreeView->currentIndex();
    if(idx.isValid())
    {
        QModelIndex prev_idx = model()->previousItemIndex(idx);
        resultsTreeView->setCurrentIndex(prev_idx);
        model()->activate(prev_idx);
    }
}

void GrepOutputView::selectNextItem()
{
    QModelIndex idx = resultsTreeView->currentIndex();
    if(idx.isValid())
    {
        QModelIndex next_idx = model()->nextItemIndex(idx);
        resultsTreeView->setCurrentIndex(next_idx);
        model()->activate(next_idx);
    }
}


void GrepOutputView::collapseAllItems()
{
    resultsTreeView->collapseAll();
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
    // we only care about root item
    if(!topLeft.parent().isValid())
    {
        applyButton->setEnabled(topLeft.data(Qt::CheckStateRole) != Qt::Unchecked);
    }
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
    
    // clear list of recently used search patterns
    KConfigGroup cg = ICore::self()->activeSession()->config()->group( "GrepDialog" );
    cg.writeEntry("LastSearchItems", QStringList());
    
    m_statusLabel->setText(QString());
}

void GrepOutputView::modelSelectorContextMenu(const QPoint& pos)
{
    QPoint globalPos = modelSelector->mapToGlobal(pos);
    QMenu myMenu;
    myMenu.addAction(m_clearSearchHistory);
    myMenu.exec(globalPos);
}
