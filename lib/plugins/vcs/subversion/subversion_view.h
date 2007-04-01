#ifndef KDEVSUBVERSIONVIEW_H
#define KDEVSUBVERSIONVIEW_H

#include "subversion_part.h"
class SubversionJob;
class QWidget;
struct KDevSubversionViewPrivate;
class QModelIndex;
class KTabWidget;
class SvnLogHolder;
class SvnBlameHolder;

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
#endif
