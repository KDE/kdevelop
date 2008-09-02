
#ifndef XTEST_CONFIG_CONFIGWIDGET_H_INCLUDED
#define XTEST_CONFIG_CONFIGWIDGET_H_INCLUDED

#include <QWidget>

class QToolButton;
namespace Ui { class VeritasConfig; }

namespace Veritas
{

class ConfigWidget : public QWidget
{
Q_OBJECT
public:
    ConfigWidget(QWidget* parent=0);
    virtual ~ConfigWidget();

    /*! For testers only. */
    void fto_clickExpandDetails() const;

private Q_SLOTS:
    void expandDetails(bool);

private:
    void setupButtons();
    QToolButton* expandDetailsButton() const;
    QToolButton* addExecutableButton() const;
    QToolButton* removeExecutableButton() const;

private:
    Ui::VeritasConfig* m_ui;
};

}

#endif // XTEST_CONFIG_CONFIGWIDGET_H_INCLUDED
