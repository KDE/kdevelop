/*
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>
    SPDX-FileCopyrightText: 2010 Yannick Motta <yannick.motta@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MANPAGEDOCUMENTATIONWIDGET_H
#define MANPAGEDOCUMENTATIONWIDGET_H

#include <QStackedWidget>

class QString;
class QLabel;
class QTreeView;
class QProgressBar;

class ManPageDocumentationWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit ManPageDocumentationWidget(QWidget *parent = nullptr);
public Q_SLOTS:
    void manIndexLoaded();
    void sectionListUpdated();
    void sectionParsed();
    void handleError(const QString& errorString);
private:
    bool eventFilter(QObject* watched, QEvent* event) override;

    QWidget* m_loadingWidget;
    QTreeView* m_treeView;
    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;
};

#endif // MANPAGEDOCUMENTATIONWIDGET_H
