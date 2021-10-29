/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVCLAZY_COMMAND_LINE_WIDGET_H
#define KDEVCLAZY_COMMAND_LINE_WIDGET_H

#include <QWidget>

namespace Clazy
{

namespace Ui { class CommandLineWidget; }

class CommandLineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CommandLineWidget(QWidget* parent = nullptr);
    ~CommandLineWidget() override;

public Q_SLOTS:
    void setText(const QString& text);

private:
    void updateCommandLine();

private:
    QScopedPointer<Ui::CommandLineWidget> m_ui;
    QString m_text;
};

}

#endif
