/*
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2011 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LicenseRef-KDevelop-Ideal
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

    QSize sizeHint() const override;

    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    const Qt::DockWidgetArea _area;
};

#endif // IDEALTOOLBUTTON_H
