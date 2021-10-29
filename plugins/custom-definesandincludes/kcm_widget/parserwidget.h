/*
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PARSERWIDGET_H
#define PARSERWIDGET_H

#include <QWidget>
#include <QScopedPointer>

namespace Ui
{
class ParserWidget;
}

namespace KDevelop
{
class IProject;
}

struct ParserArguments;

class ParserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ParserWidget(QWidget* parent);
    ~ParserWidget() override;

    void setParserArguments(const ParserArguments& arguments);
    ParserArguments parserArguments() const;

Q_SIGNALS:
    void changed();

private Q_SLOTS:
    void textEdited();
    void languageStandardChangedC(const QString& standard);
    void languageStandardChangedCpp(const QString& standard);
    void languageStandardChangedOpenCl(const QString& standard);
    void languageStandardChangedCuda(const QString& standard);
    void updateEnablements();

private:
    QScopedPointer<Ui::ParserWidget> m_ui;
};

#endif // PARSERWIDGET_H
