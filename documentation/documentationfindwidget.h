/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#ifndef KDEVPLATFORM_DOCUMENTATIONFINDWIDGET_H
#define KDEVPLATFORM_DOCUMENTATIONFINDWIDGET_H

#include <QWidget>
#include "documentationexport.h"

namespace Ui { class FindWidget; }
namespace KDevelop
{
    
class KDEVPLATFORMDOCUMENTATION_EXPORT DocumentationFindWidget : public QWidget
{
    Q_OBJECT
    public:
        enum FindOption {
            Next = 1,
            Previous = 2,
            MatchCase = 4
        };
        Q_DECLARE_FLAGS(FindOptions, FindOption);
        
        explicit DocumentationFindWidget(QWidget* parent = 0);
        virtual ~DocumentationFindWidget();
        
        virtual void showEvent ( QShowEvent* ) override;
        
    private Q_SLOTS:
        void searchNext();
        void searchPrevious();
        
    Q_SIGNALS:
        void newSearch(const QString& text, KDevelop::DocumentationFindWidget::FindOptions);

    private:
        Ui::FindWidget* m_ui;
};

}

#endif // KDEVPLATFORM_DOCUMENTATIONFINDWIDGET_H
