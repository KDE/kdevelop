/*
    Copyright David Nolden  <david.nolden.kdevelop@art-master.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
