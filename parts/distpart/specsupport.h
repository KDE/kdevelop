#ifndef __SPECSUPPORT_H__
#define __SPECSUPPORT_H__

#include <qwidget.h>
#include <qmap.h>
#include "packagebase.h"
#include "distpart_part.h"
#include "distpart_widget.h"

class QPushButton;
class QVBox;

class SpecSupport : public packageBase {
    Q_OBJECT

public:
    SpecSupport(DistpartPart * m_part);
    ~SpecSupport();

public slots:
    void slotbuildAllPushButtonPressed();
    void slotexportSPECPushButtonPressed();
    void slotimportSPECPushButtonPressed();    
    void slotsrcPackagePushButtonPressed();
    
private:
    void parseDotRpmmacros();
    bool createRpmDirectoryFromMacro(const QString & name);
    
    bool getInfo(QString s, QString motif, void (packageBase::*func)(QString));
    
    DistpartPart * m_part;
    QMap<QString,QString> map;
    QPushButton *buildAllPushButton, *exportSPECPushButton, *importSPECPushButton, *srcPackagePushButton;
    QVBox * box;
};

#endif
