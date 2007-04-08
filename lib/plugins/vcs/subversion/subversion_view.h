#ifndef KDEVSUBVERSIONVIEW_H
#define KDEVSUBVERSIONVIEW_H

#include "subversion_part.h"
#include "ui_uiprogress_dlg.h"
class SubversionJob;
class QWidget;
struct KDevSubversionViewPrivate;
class QModelIndex;
class KTabWidget;
class SvnLogHolder;
class SvnBlameHolder;
class QProgressBar;
class KUrl;

class KDevSubversionView : public QWidget/*, public Ui::SvnLogviewWidget*/ {
    Q_OBJECT
public:
    KDevSubversionView(KDevSubversionPart *part, QWidget* parent);
    virtual ~KDevSubversionView();
    
public Q_SLOTS:
    void printNotification( QString msg );
    void printLog( SubversionJob *job );
    void printBlame( SubversionJob *job );
    void printDiff( SubversionJob *job );
    void slotJobFinished( SubversionJob *job );
    void closeCurrentTab();    
private:
    KTabWidget *tab();

    KDevSubversionViewPrivate* const d;
    
};

///////////////////////////////////////////////////////////////////////////

/** @short Progress display dialog for subversion ASync jobs
 */
class SvnProgressDialog : public QDialog, public Ui::SvnProgressDialog
{
public:
    SvnProgressDialog( QWidget *parent, const QString &caption = QString() );
    virtual ~SvnProgressDialog();
    
    QProgressBar* progressBar();
    void setSource( const QString &src );
    void setDestination( const QString &dest );
};

#endif
