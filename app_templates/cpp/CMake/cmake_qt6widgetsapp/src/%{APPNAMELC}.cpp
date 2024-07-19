#include "%{APPNAMELC}.h"
#include "ui_%{APPNAMELC}.h"

%{APPNAMEID}::%{APPNAMEID}(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::%{APPNAMEID})
{
    m_ui->setupUi(this);
}

%{APPNAMEID}::~%{APPNAMEID}() = default;

#include "moc_%{APPNAMELC}.cpp"
