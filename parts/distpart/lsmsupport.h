#ifndef __LSMSUPPORT_H__
#define __LSMSUPPORT_H__

#include <qwidget.h>
#include "packagebase.h"
#include "distpart_part.h"

class QLabel;

class LsmSupport : public packageBase
{
    Q_OBJECT

public:
    LsmSupport(DistpartPart * m_part);
    ~LsmSupport();

private:
    DistpartPart * m_part;
    QLabel * label;
};

#endif
