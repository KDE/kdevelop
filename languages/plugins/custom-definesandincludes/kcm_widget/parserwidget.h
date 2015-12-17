/*
 * This file is part of KDevelop
 *
 * Copyright 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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

class ParserWidget : public QWidget
{
    Q_OBJECT
public:
    ParserWidget(QWidget* parent);
    ~ParserWidget() override;

    void setParserArguments(const QString& arguments);
    QString parserArguments() const;

signals:
    void changed();

private slots:
    void textEdited();
    void languageStandardChanged(const QString& standard);
    void updateEnablements();

private:
    QScopedPointer<Ui::ParserWidget> m_ui;
};

#endif // PARSERWIDGET_H
