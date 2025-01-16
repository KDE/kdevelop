/*
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2011 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LicenseRef-MIT-KDevelop-Ideal
*/

#ifndef IDEALTOOLBUTTON_H
#define IDEALTOOLBUTTON_H

#include <QToolButton>

class IdealToolButton: public QToolButton
{
    Q_OBJECT

public:
    explicit IdealToolButton(Qt::DockWidgetArea area, QWidget *parent = nullptr);

    Qt::Orientation orientation() const;

    /**
     * @return whether this button is currently pressed down
     *
     * @note Unlike isDown(), isPressed() returns @c true in a slot connected to the
     *       defaultAction()->toggled() signal if the toggling is caused by a click on this button.
     */
    [[nodiscard]] bool isPressed() const;

    QSize sizeHint() const override;

    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    const Qt::DockWidgetArea _area;
    bool m_isPressed = false;
};

#endif // IDEALTOOLBUTTON_H
