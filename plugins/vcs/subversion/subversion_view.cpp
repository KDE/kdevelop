#include "subversion_view.h"
#include "subversion_core.h"
#include "svn_models.h"
#include "svn_logviewwidgets.h"
#include "svn_blamewidgets.h"
// #include "ui_svnlogviewwidget.h"
#include <kmessagebox.h>


#include <QList>
#include <QVariant>
#include <QModelIndex>

class KDevSubversionViewPrivate{
public:
    KDevSubversionPart *m_part;
    KDevSubversionView *m_view;
    QVBoxLayout *m_layout;
    QWidget *m_child;
};
// TODO first make empty widget by factory. This host container widget is parent
// of every other subwidgets, including logviewer, blame, notifier, ...
KDevSubversionView::KDevSubversionView( KDevSubversionPart *part, QWidget* parent )
    : QWidget( parent ), d(new KDevSubversionViewPrivate)
{
    d->m_part = part;
    d->m_view = this;
    d->m_layout = new QVBoxLayout(this);
    d->m_layout->setSpacing(0);
    d->m_layout->setMargin(0);
    d->m_child = 0L;
//     d->setupUi();
//     connect( d->ui->treeView, SIGNAL(clicked (const QModelIndex &)),
//              d->m_logviewDetailedModel, SLOT(setNewRevision(const QModelIndex &)) );
    
    setObjectName( i18n( "Subversion" ) );
    setWhatsThis( i18n( "<b>Subversion</b><p>"
            "This window contains an embedded Subversion View Reusults." ) );
    setWindowIcon( KIcon( "subversion" ) );
    setWindowTitle( i18n( "subversion" ) );
    
    connect( d->m_part->svncore(), SIGNAL(svnNotify(QString)),
             this, SLOT(printNotification(QString)) );    
    connect( d->m_part->svncore(), SIGNAL(logFetched(SubversionJob *)),
             this, SLOT(printLog(SubversionJob *)) );
    connect( d->m_part->svncore(), SIGNAL(blameFetched(SubversionJob *)),
             this, SLOT(printBlame(SubversionJob *)) );
    connect( d->m_part->svncore(), SIGNAL(jobFinished(SubversionJob*)),
             this, SLOT(slotJobFinished(SubversionJob*)) );

}

KDevSubversionView::~KDevSubversionView()
{
//     delete d->ui;
    delete d;
}

void KDevSubversionView::printNotification(QString msg)
{
    kDebug() << " KDevSubversionView::printNotification: " << msg << endl;
}
void KDevSubversionView::printLog(SubversionJob *j)
{
    if( d->m_child ){
        d->m_layout->removeWidget( d->m_child );
        delete d->m_child;
        d->m_child = 0L;
    }
    
    SvnLogviewJob *job = (SvnLogviewJob*)j;
    if (job->wasSuccessful() ){
        SvnLogviewWidget *logWidget = new SvnLogviewWidget( job->urlList()[0], d->m_part, this);
        
        logWidget->refreshWithNewData( job->m_loglist );
        
        d->m_child = logWidget;
        
        d->m_layout->addWidget( logWidget );
        this->show();
    }
    else{
        KMessageBox::error( this, job->errorMsg(), "error" );
// //         d->ui->textEdit->append( "log failed" );
    }
    
}
void KDevSubversionView::printBlame( SubversionJob* j )
{
    if( d->m_child ){
        d->m_layout->removeWidget( d->m_child );
        delete d->m_child;
        d->m_child = 0L;
    }
    SvnBlameJob *job = (SvnBlameJob*) j;
    if( job->wasSuccessful() ){
        SvnBlameWidget *widget = new SvnBlameWidget(this);
        widget->refreshWithNewData( job->m_blameList);
        d->m_child = widget;
        d->m_layout->addWidget( widget );
        this->show();
    }
    else{
        KMessageBox::error( this, job->errorMsg(), "error" );
    }
}
void KDevSubversionView::slotJobFinished( SubversionJob *job )
{
    switch( job->type() ){
        case SVN_LOGVIEW:
            break; // not used for logview
        case SVN_BLAME: // not used for blame
            break;
        case SVN_CHECKOUT:
            // not implemented yet
            break;
        case SVN_ADD:
            // note: these messagebox are just temporary ones.
            // don't try to i18n(), because it will be removed
            if( job->wasSuccessful() )
                KMessageBox::information( NULL, "Successfully Added" );
            else
                KMessageBox::information( NULL, "Addeding failed\n" + job->errorMsg() );
            break;
        case SVN_DELETE:
            if( job->wasSuccessful() )
                KMessageBox::information( NULL, "Successfully deleted" );
            else
                KMessageBox::information( NULL, "delete failed\n" + job->errorMsg() );
            break;
        case SVN_COMMIT:
            if( job->wasSuccessful() )
                KMessageBox::information( NULL, "Successfully committed" );
            else
                KMessageBox::information( NULL, "commit failed\n" + job->errorMsg() );
            break;
        case SVN_UPDATE:
            if( job->wasSuccessful() )
                KMessageBox::information( NULL, "Successfully updated" );
            else
                KMessageBox::information( NULL, "updating failed\n" + job->errorMsg() );
            break;
        case SVN_STATUS:
            break;
    }
}

#include "subversion_view.moc"
