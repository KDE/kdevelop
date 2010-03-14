#ifndef DOCWIDGET_H
#define DOCWIDGET_H

#include <QtGui/qwidget.h>


namespace Ui
{
class DocWidget;
}

class QHelpEngine;

class DocWidget : public QWidget
{
Q_OBJECT
public:
    DocWidget();
private:
    Ui::DocWidget* ui;
    QHelpEngine* engine;
public slots:
    void searchForIdentifier();
};

#endif // DOCWIDGET_H
