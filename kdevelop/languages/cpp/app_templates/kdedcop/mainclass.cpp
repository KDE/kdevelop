%{CPP_TEMPLATE}

#include "mainclass.h"


MainClass::MainClass()
 : DCOPDemoIface(),
   DCOPObject("mainclass"), /* this is the object name DCOP will expose */
   m_intValue(0)
{
}

MainClass::~MainClass()
{
}

void MainClass::setStrVal(const QString & str) {
    m_strValue = str;
}

void MainClass::setIntVal(int val) {
    m_intValue = val;
}

QString MainClass::strVal() const {
    return m_strValue;
}

int MainClass::intVal() const {
    return m_intValue;
}
