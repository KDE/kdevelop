#include "%{APPNAMELC}.h"
#include "ui_%{APPNAMELC}.h"

%{APPNAME}::%{APPNAME}(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::%{APPNAME})
{
    ui->setupUi(this);
}

%{APPNAME}::~%{APPNAME}()
{
    delete ui;
}
