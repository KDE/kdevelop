#include "subversion_view.h"
#include "subversion_core.h"
#include "svn_models.h"
#include "svn_logviewwidgets.h"
#include "svn_blamewidgets.h"
// #include "ui_svnlogviewwidget.h"
#include <kmessagebox.h>
#include <ktabwidget.h>
#include <ktextedit.h>
#include <ktextbrowser.h>

#include <QList>
#include <QVariant>
#include <QModelIndex>
#include <QPushButton>
#include <QTextStream>
#include <QFile>

class KDevSubversionViewPrivate{
public:
    KDevSubversionPart *m_part;
    KDevSubversionView *m_view;
    QVBoxLayout *m_layout;
    KTabWidget *m_tab;
    KTextEdit *m_edit;
    QPushButton *m_closeButton;
};
// TODO first make empty widget by factory. This host container widget is parent
// of every other subwidgets, including logviewer, blame, notifier, ...
KDevSubversionView::KDevSubversionView( KDevSubversionPart *part, QWidget* parent )
    : QWidget( parent ), d(new KDevSubversionViewPrivate)
{
    d->m_part = part;
    d->m_view = this;
    
    d->m_tab = new KTabWidget(this);
    d->m_layout = new QVBoxLayout( this );
    d->m_layout->setMargin(0);
    d->m_layout->addWidget( tab() );
    
    d->m_edit = new KTextEdit( tab() );
    d->m_edit->setReadOnly( TRUE );
    tab()->addTab( d->m_edit, i18n("Notification") );
    
    d->m_closeButton = new QPushButton( tab() );
    d->m_closeButton->setText( i18n("Close") );
    tab()->setCornerWidget( d->m_closeButton );
    connect( d->m_closeButton, SIGNAL(clicked()), this, SLOT(closeCurrentTab()) );
    
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
    connect( d->m_part->svncore(), SIGNAL(diffFetched(SubversionJob *)),
             this, SLOT(printDiff(SubversionJob *)) );
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
    if( !d->m_edit ){
        // should not happen
        d->m_edit = new KTextEdit(this);
    }
    d->m_edit->append( msg );
    tab()->setCurrentIndex( tab()->indexOf(d->m_edit) );
}
void KDevSubversionView::printLog(SubversionJob *j)
{
    SvnLogviewJob *job = (SvnLogviewJob*)j;
    if (job->wasSuccessful() ){
        SvnLogviewWidget *widget= new SvnLogviewWidget( job->urlList()[0], d->m_part, tab() );
        
        widget->refreshWithNewData( job->m_loglist );

        tab()->addTab( widget, i18n("Log History") );
        tab()->setTabEnabled( tab()->indexOf(widget), true );
        tab()->setCurrentIndex( tab()->indexOf(widget) );
    }
    else{
        KMessageBox::error( this, job->errorMsg(), "error" );
// //         d->ui->textEdit->append( "log failed" );
    }
    
}
void KDevSubversionView::printBlame( SubversionJob* j )
{
    SvnBlameJob *job = (SvnBlameJob*) j;
    if( job->wasSuccessful() ){
        SvnBlameWidget *widget = new SvnBlameWidget( tab() );
        widget->refreshWithNewData( job->m_blameList);
        tab()->addTab( widget, i18n("Blame") );
        tab()->setTabEnabled( tab()->indexOf(widget), true );
        tab()->setCurrentIndex( tab()->indexOf(widget) );
    }
    else{
        KMessageBox::error( this, job->errorMsg(), "error" );
    }
}

void KDevSubversionView::printDiff( SubversionJob *j )
{
    SvnDiffJob *job = dynamic_cast<SvnDiffJob*>(j);
    if( !job ) return;
    if( !job->wasSuccessful() ){
        KMessageBox::error( this, job->errorMsg() );
        return;
    }
    QFile file( job->out_name );
    if( !file.exists() ){
        KMessageBox::error( this, i18n("The output diff file cannot be located") );
        return;
    }
    // end of error check
    
    KTextBrowser *widget = new KTextBrowser( tab() );
    if ( file.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &file );
        QString line;
        while ( !stream.atEnd() ) {
            line = stream.readLine();
            widget->append( line );
        }
        file.close();
    }
    
    widget->setReadOnly( true );
    tab()->addTab( widget, i18n("Diff") );
    tab()->setCurrentIndex( tab()->indexOf(widget) );

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
                KMessageBox::information( NULL, "Deleting failed\n" + job->errorMsg() );
            break;
        case SVN_COMMIT:
            if( job->wasSuccessful() )
                KMessageBox::information( NULL, "Successfully committed" );
            else
                KMessageBox::information( NULL, "Commit failed\n" + job->errorMsg() );
            break;
        case SVN_UPDATE:
            if( job->wasSuccessful() )
                KMessageBox::information( NULL, "Successfully updated" );
            else
                KMessageBox::information( NULL, "Updating failed\n" + job->errorMsg() );
            break;
        case SVN_STATUS:
            break;

    }
}

void KDevSubversionView::closeCurrentTab()
{
    QWidget *current = tab()->currentWidget();
    KTextEdit *edit = static_cast<KTextEdit*>(current);
    if( edit ){
        if( edit == d->m_edit ) // main notification output should not be deleted
            return;
    }
    tab()->removePage( current );
    delete current;
}

KTabWidget* KDevSubversionView::tab()
{
    return d->m_tab;
}
#include "subversion_view.moc"
