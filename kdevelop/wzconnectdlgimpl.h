#ifndef WZCONNECTDLGIMPL_H
#define WZCONNECTDLGIMPL_H
#include "wzconnectdlg.h"

class wzConnectDlgImpl : public wzConnectDlg
{ 
    Q_OBJECT

public:
    wzConnectDlgImpl( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~wzConnectDlgImpl();

public slots:
    void slotDoCreateMethod();

protected slots:
    void slotDoCreateSlot();
    void slotMemberSelected(const QString&);
    void slotMethodChanged();
    void slotMethodSelected(const QString&);
    void slotNewSlotChanged();
    void slotSignalSelected(const QString&);
    void slotSlotSelected(const QString&);
    void slotSourceClassSelected(const QString&);

};

#endif // WZCONNECTDLGIMPL_H
