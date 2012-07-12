/*
 *
 */

#ifndef TESTCASESWIDGET_H
#define TESTCASESWIDGET_H

#include <QtGui/QWidget>

class KEditListWidget;

class TestCasesWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList testCases READ testCases WRITE setTestCases)

public:
    explicit TestCasesWidget (QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~TestCasesWidget();

    QStringList testCases() const;
    void setTestCases(const QStringList& testCases);

private:
    KEditListWidget* ui;

};

#endif // TESTCASESWIDGET_H
