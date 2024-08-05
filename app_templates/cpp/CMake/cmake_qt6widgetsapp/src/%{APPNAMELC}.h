#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H

#include <QMainWindow>

#include <memory>

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
    const std::unique_ptr<Ui::%{APPNAMEID}> m_ui;
};

#endif // %{APPNAMEUC}_H
