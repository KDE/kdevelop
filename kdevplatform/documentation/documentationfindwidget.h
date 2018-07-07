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
        Q_DECLARE_FLAGS(FindOptions, FindOption)
        
        explicit DocumentationFindWidget(QWidget* parent = nullptr);
        ~DocumentationFindWidget() override;

        void hideEvent(QHideEvent* event) override;

    public Q_SLOTS:
        void startSearch();

    private Q_SLOTS:
        void searchNext();
        void searchPrevious();
        void emitDataChanged();

    Q_SIGNALS:
        /**
         * Emitted when the user requests a search.
         * @param text text to search in documentation
         * @param options MatchCase being set or empty flags (Next/Previous unused here)
         */
        void searchRequested(const QString& text, KDevelop::DocumentationFindWidget::FindOptions options);
        /**
         * Emitted when the user edits the search field or changes the case-sensitivity checkbox.
         * Allows documentation views capable of live searches to show live results while the user types.
         * @param text current text in search text field
         * @param options MatchCase being set or empty flags (Next/Previous unused here)
         */
        void searchDataChanged(const QString& text, KDevelop::DocumentationFindWidget::FindOptions options);
        /**
         * Emitted when the search tool view is closed, so no more search hits should be displayed.
         */
        void searchFinished();

    private:
        Ui::FindWidget* m_ui;
};

}

#endif // KDEVPLATFORM_DOCUMENTATIONFINDWIDGET_H
