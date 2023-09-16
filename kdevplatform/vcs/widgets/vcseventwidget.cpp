/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcseventwidget.h"

#include <QAction>
#include <QClipboard>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDesktopServices>

#include <KLocalizedString>
#include <KTextToHTML>

#include <interfaces/iplugin.h>

#include "ui_vcseventwidget.h"
#include "vcsdiffwidget.h"

#include "../interfaces/ibasicversioncontrol.h"
#include "../models/vcseventmodel.h"
#include "../models/vcsitemeventmodel.h"
#include "../vcsevent.h"
#include "../vcsjob.h"
#include "../vcsrevision.h"
#include "debug.h"


namespace KDevelop
{

class VcsEventWidgetPrivate
{
public:
    explicit VcsEventWidgetPrivate( VcsEventWidget* w )
        : q( w )
    {
        m_copyAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18nc("@action:inmenu", "Copy Revision Id"), q);
        m_copyAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_C));
        QObject::connect(m_copyAction, &QAction::triggered, q, [&] { copyRevision(); });
    }

    Ui::VcsEventWidget* m_ui;
    VcsItemEventModel* m_detailModel;
    VcsEventLogModel *m_logModel;
    QUrl m_url;
    QModelIndex m_contextIndex;
    VcsEventWidget* q;
    QAction* m_copyAction;
    IBasicVersionControl* m_iface;
    void eventViewCustomContextMenuRequested( const QPoint &point );
    void eventViewClicked( const QModelIndex &index );
    void jobReceivedResults( KDevelop::VcsJob* job );
    void copyRevision();
    void diffToPrevious();
    void diffRevisions();
    void currentRowChanged(const QModelIndex& start, const QModelIndex& end);
};

void VcsEventWidgetPrivate::eventViewCustomContextMenuRequested( const QPoint &point )
{
    m_contextIndex = m_ui->eventView->indexAt( point );
    if( !m_contextIndex.isValid() ){
        qCDebug(VCS) << "contextMenu is not in TreeView";
        return;
    }

    QMenu menu( m_ui->eventView );
    menu.addAction(m_copyAction);
    auto diffToPreviousAction = menu.addAction(i18nc("@action:inmenu", "Diff to Previous Revision"));
    QObject::connect(diffToPreviousAction, &QAction::triggered, q, [&] { diffToPrevious(); });

    auto diffRevisionsAction = menu.addAction(i18nc("@action:inmenu", "Diff between Revisions"));
    QObject::connect(diffRevisionsAction, &QAction::triggered, q, [&] { diffRevisions(); });
    diffRevisionsAction->setEnabled(m_ui->eventView->selectionModel()->selectedRows().size()>=2);

    menu.exec( m_ui->eventView->viewport()->mapToGlobal(point) );
}

void VcsEventWidgetPrivate::currentRowChanged(const QModelIndex& start, const QModelIndex& end)
{
    Q_UNUSED(end);
    if(start.isValid())
        eventViewClicked(start);
}

void VcsEventWidgetPrivate::eventViewClicked( const QModelIndex &index )
{
    KDevelop::VcsEvent ev = m_logModel->eventForIndex( index );
    m_detailModel->removeRows(0, m_detailModel->rowCount());

    if( ev.revision().revisionType() != KDevelop::VcsRevision::Invalid )
    {
        m_ui->itemEventView->setEnabled(true);
        m_ui->message->setEnabled(true);
        const KTextToHTML::Options markupOptions = KTextToHTML::PreserveSpaces;
        const QString markupMessage =
            QLatin1String("<tt>") + KTextToHTML::convertToHtml(ev.message(), markupOptions) + QLatin1String("</tt>");
        m_ui->message->setHtml(markupMessage);
        m_detailModel->addItemEvents( ev.items() );
    }else
    {
        m_ui->itemEventView->setEnabled(false);
        m_ui->message->setEnabled(false);
        m_ui->message->clear();
    }

    QHeaderView* header = m_ui->itemEventView->header();
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setStretchLastSection(true);
}

void VcsEventWidgetPrivate::copyRevision()
{
    qApp->clipboard()->setText(m_contextIndex.sibling(m_contextIndex.row(), 0).data().toString());
}

void VcsEventWidgetPrivate::diffToPrevious()
{
    KDevelop::VcsEvent ev = m_logModel->eventForIndex( m_contextIndex );
    KDevelop::VcsRevision prev = KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Previous);
    KDevelop::VcsJob* job = m_iface->diff( m_url, prev, ev.revision() );

    auto* widget = new VcsDiffWidget( job );
    widget->setRevisions( prev, ev.revision() );
    auto* dlg = new QDialog( q );

    widget->connect(widget, &VcsDiffWidget::destroyed, dlg, &QDialog::deleteLater);

    dlg->setWindowTitle( i18nc("@title:window", "Difference To Previous") );

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    auto mainWidget = new QWidget;
    auto *mainLayout = new QVBoxLayout;
    dlg->setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
    dlg->connect(buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    dlg->connect(buttonBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
    mainLayout->addWidget(widget);
    mainLayout->addWidget(buttonBox);

    dlg->show();
}

void VcsEventWidgetPrivate::diffRevisions()
{
    QModelIndexList l = m_ui->eventView->selectionModel()->selectedRows();
    KDevelop::VcsEvent ev1 = m_logModel->eventForIndex( l.first() );
    KDevelop::VcsEvent ev2 = m_logModel->eventForIndex( l.last() );
    KDevelop::VcsJob* job = m_iface->diff( m_url, ev1.revision(), ev2.revision() );

    auto* widget = new VcsDiffWidget( job );
    widget->setRevisions( ev1.revision(), ev2.revision() );

    auto dlg = new QDialog( q );
    dlg->setWindowTitle( i18nc("@title:window", "Difference between Revisions") );

    widget->connect(widget, &VcsDiffWidget::destroyed, dlg, &QDialog::deleteLater);

    auto mainLayout = new QVBoxLayout(dlg);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    auto okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
    dlg->connect(buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    dlg->connect(buttonBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
    mainLayout->addWidget(widget);
    dlg->show();
}

VcsEventWidget::VcsEventWidget( const QUrl& url, const VcsRevision& rev, KDevelop::IBasicVersionControl* iface, QWidget* parent )
    : QWidget(parent)
    , d_ptr(new VcsEventWidgetPrivate(this))
{
    Q_D(VcsEventWidget);

    d->m_iface = iface;
    d->m_url = url;
    d->m_ui = new Ui::VcsEventWidget();
    d->m_ui->setupUi(this);

    d->m_logModel = new VcsEventLogModel(iface, rev, url, this);
    d->m_ui->eventView->setModel( d->m_logModel );
    d->m_ui->eventView->sortByColumn(0, Qt::DescendingOrder);
    d->m_ui->eventView->setContextMenuPolicy( Qt::CustomContextMenu );
    QHeaderView* header = d->m_ui->eventView->header();
    header->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
    header->setSectionResizeMode( 1, QHeaderView::Stretch );
    header->setSectionResizeMode( 2, QHeaderView::ResizeToContents );
    header->setSectionResizeMode( 3, QHeaderView::ResizeToContents );
    // Select first row as soon as the model got populated
    connect(d->m_logModel, &QAbstractItemModel::rowsInserted, this, [this]() {
        Q_D(VcsEventWidget);
        auto view = d->m_ui->eventView;
        view->setCurrentIndex(view->model()->index(0, 0));
    });

    d->m_detailModel = new VcsItemEventModel(this);
    d->m_ui->itemEventView->setModel( d->m_detailModel );

    connect(d->m_ui->eventView, &QTreeView::clicked, this, [this] (const QModelIndex& index) {
        Q_D(VcsEventWidget);
        d->eventViewClicked(index);
    });
    connect(d->m_ui->eventView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, [this] (const QModelIndex& start, const QModelIndex& end) {
        Q_D(VcsEventWidget);
        d->currentRowChanged(start, end);
    });
    connect(d->m_ui->eventView, &QTreeView::customContextMenuRequested,
            this, [this] (const QPoint& point) {
        Q_D(VcsEventWidget);
        d->eventViewCustomContextMenuRequested(point);
    });

    connect(d->m_ui->message, &QTextBrowser::anchorClicked,
            this, [&] (const QUrl& url) { QDesktopServices::openUrl(url); });
}

VcsEventWidget::~VcsEventWidget()
{
    Q_D(VcsEventWidget);

    delete d->m_ui;
}

}


#include "moc_vcseventwidget.cpp"
