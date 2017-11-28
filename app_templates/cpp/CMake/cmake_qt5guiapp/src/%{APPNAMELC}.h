#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H

#include <QMainWindow>
#include <QScopedPointer>

namespace Ui {
class %{APPNAME};
}

class %{APPNAME} : public QMainWindow
{
    Q_OBJECT

public:
    explicit %{APPNAME}(QWidget *parent = nullptr);
    ~%{APPNAME}() override;

private:
    QScopedPointer<Ui::%{APPNAME}> m_ui;
};

#endif // %{APPNAMEUC}_H
