#include "lsmsupport.h"
#include <qlabel.h>

LsmSupport::LsmSupport(DistpartPart * part) : packageBase(part,"Lsm") {
    m_part = part;
    
    label = new QLabel("Not yet implemented !",area());
}

LsmSupport::~LsmSupport() {
}
