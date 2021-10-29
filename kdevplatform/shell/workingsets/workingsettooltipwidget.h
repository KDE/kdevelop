/*
    SPDX-FileCopyrightText: David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_WORKINGSETTOOLTIPWIDGET_H
#define KDEVPLATFORM_WORKINGSETTOOLTIPWIDGET_H

#include <QWidget>
#include <QMap>

class FileWidget;

class QLabel;
class QToolButton;
class QPushButton;

namespace KDevelop {

class WorkingSet;
class WorkingSetToolButton;

class MainWindow;

class WorkingSetToolTipWidget : public QWidget
{
    Q_OBJECT

public:
    WorkingSetToolTipWidget(QWidget* parent, WorkingSet* set, MainWindow* mainwindow);

Q_SIGNALS:
    void shouldClose();

public Q_SLOTS:
    void buttonClicked(bool);
    void updateFileButtons();
    void labelClicked();
    void nextDocument();
    void previousDocument();

private:
    bool isActiveSet() const;

    QVector<FileWidget*> m_orderedFileWidgets;
    QMap<QString, FileWidget*> m_fileWidgets;
    WorkingSet* const m_set;

    QPushButton* m_mergeButton;
    QPushButton* m_subtractButton;
    QPushButton* m_openButton;
    QLabel* m_documentsLabel;
    WorkingSetToolButton* m_setButton;

    QPushButton* m_deleteButton;
};

}

#endif // KDEVPLATFORM_WORKINGSETTOOLTIPWIDGET_H
