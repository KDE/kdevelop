/*
 * This file is part of KDevelop
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TEMPLATESELECTIONPAGE_H
#define TEMPLATESELECTIONPAGE_H

#include <QWidget>
#include <KUrl>

class QModelIndex;
namespace KDevelop
{
class TemplatesModel;
}

namespace Ui
{
class TemplateSelectionPage;
}

class TemplateSelectionPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString selectedTemplate READ selectedTemplate)
    Q_PROPERTY(QString identifier READ identifier)
    Q_PROPERTY(KUrl baseUrl READ baseUrl WRITE setBaseUrl)

public:
    TemplateSelectionPage(QWidget* parent = 0);
    virtual ~TemplateSelectionPage();

    QString selectedTemplate() const;

    QString identifier() const;

    KUrl baseUrl() const;
    void setBaseUrl(const KUrl& url);

private:
    Ui::TemplateSelectionPage* ui;
    KDevelop::TemplatesModel* m_model;

signals:
    void templateValid(bool valid);

public slots:
    void contentsChanged();
};

#endif // TEMPLATESELECTIONPAGE_H
