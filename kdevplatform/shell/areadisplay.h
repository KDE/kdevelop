/*
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef AREADISPLAY_H
#define AREADISPLAY_H

#include <QWidget>

class QLabel;
class QToolButton;

namespace KDevelop {
class MainWindow;
}
namespace Sublime {
class MainWindow;
class Area;
}

/**
 * This widget displays the current area by the menu
 */

class AreaDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit AreaDisplay(KDevelop::MainWindow* parent);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private Q_SLOTS:
    void newArea(Sublime::Area* area);
    void backToCode();

private:
    QLabel* m_separator;
    QToolButton* m_button;
    KDevelop::MainWindow* const m_mainWindow;
};

#endif // AREADISPLAY_H
