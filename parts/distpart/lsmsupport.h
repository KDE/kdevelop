#ifndef __LSMSUPPORT_H__
#define __LSMSUPPORT_H__

#include <qwidget.h>
#include "packagebase.h"


class LsmSupport : public packageBase
{
    Q_OBJECT

public:
    LsmSupport(DistpartPart * m_part);
    ~LsmSupport();

};

#endif
