#include "%{APPNAMELC}.h"
#include "ui_%{APPNAMELC}.h"

%{APPNAME}::%{APPNAME}(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::%{APPNAME})
{
    m_ui->setupUi(this);
}

%{APPNAME}::~%{APPNAME}() = default;
