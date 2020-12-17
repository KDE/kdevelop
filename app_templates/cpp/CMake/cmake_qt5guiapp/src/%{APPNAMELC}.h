#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H

#include <QMainWindow>
#include <QScopedPointer>

namespace Ui {
class %{APPNAMEID};
}

class %{APPNAMEID} : public QMainWindow
{
    Q_OBJECT

public:
    explicit %{APPNAMEID}(QWidget *parent = nullptr);
    ~%{APPNAMEID}() override;

private:
    QScopedPointer<Ui::%{APPNAMEID}> m_ui;
};

#endif // %{APPNAMEUC}_H
