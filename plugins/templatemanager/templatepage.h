/*
 * This file is part of KDevelop
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
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

#ifndef KDEVPLATFORM_PLUGIN_TEMPLATEPAGE_H
#define KDEVPLATFORM_PLUGIN_TEMPLATEPAGE_H

#include <QWidget>

class QModelIndex;
namespace KDevelop
{
class ITemplateProvider;
}

namespace Ui
{
    class TemplatePage;
}

class TemplatePage : public QWidget
{
    Q_OBJECT
public:
    explicit TemplatePage(KDevelop::ITemplateProvider* provider, QWidget* parent = 0);
    virtual ~TemplatePage();

private slots:
    void loadFromFile();
    void getMoreTemplates();
    void shareTemplates();
    void currentIndexChanged(const QModelIndex& index);
    void extractTemplate();

private:
    KDevelop::ITemplateProvider* m_provider;
    Ui::TemplatePage* ui;
};

#endif // KDEVPLATFORM_PLUGIN_TEMPLATEPAGE_H
