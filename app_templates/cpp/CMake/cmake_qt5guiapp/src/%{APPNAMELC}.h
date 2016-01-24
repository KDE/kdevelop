#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H

#include <QMainWindow>

namespace Ui {
class %{APPNAME};
}

class %{APPNAME} : public QMainWindow
{
    Q_OBJECT

public:
    explicit %{APPNAME}(QWidget *parent = 0);
    ~%{APPNAME}();

private:
    Ui::%{APPNAME} *ui;
};

#endif // %{APPNAMEUC}_H
