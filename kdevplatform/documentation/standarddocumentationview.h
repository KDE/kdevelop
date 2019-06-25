/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
 * Copyright 2016 Igor Kushnir <igorkuo@gmail.com>
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

#ifndef KDEVPLATFORM_STANDARDDOCUMENTATIONVIEW_H
#define KDEVPLATFORM_STANDARDDOCUMENTATIONVIEW_H

#include <QWidget>
#include "documentationexport.h"
#include "documentationfindwidget.h"
#include <interfaces/idocumentation.h>

class QNetworkAccessManager;
class QMenu;

namespace KDevelop
{
class StandardDocumentationViewPrivate;

/**
 * A standard documentation view, based on QtWebKit or QtWebEngine, depending on your distribution preferences.
 */
class KDEVPLATFORMDOCUMENTATION_EXPORT StandardDocumentationView : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(StandardDocumentationView)
public:
    explicit StandardDocumentationView(DocumentationFindWidget* findWidget, QWidget* parent = nullptr );
    ~StandardDocumentationView() override;

    /**
     * @brief Enables zoom functionality
     *
     * @param configSubGroup KConfigGroup nested group name used to store zoom factor.
     *        Should uniquely describe current documentation provider.
     *
     * @warning Call this function at most once
     */
    void initZoom(const QString& configSubGroup);

    void setDocumentation(const IDocumentation::Ptr& doc);

    void setOverrideCss(const QUrl &url);

    void load(const QUrl &url);
    void setHtml(const QString &html);
    void setNetworkAccessManager(QNetworkAccessManager* manager);

    /**
     *
     */
    void setDelegateLinks(bool delegate);

    QMenu* createStandardContextMenu();

Q_SIGNALS:
    void linkClicked(const QUrl &link);

public Q_SLOTS:
    /**
     * Search for @p text in the documentation view.
     */
    void search(const QString& text, KDevelop::DocumentationFindWidget::FindOptions options);
    void searchIncremental(const QString& text, KDevelop::DocumentationFindWidget::FindOptions options);
    void finishSearch();

    /**
     * Updates the contents, in case it was initialized with a documentation instance,
     * doesn't change anything otherwise
     *
     * @sa setDocumentation(IDocumentation::Ptr)
     */
    void update();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    bool eventFilter(QObject* object, QEvent* event) override;

private Q_SLOTS:
    void updateZoomFactor(double zoomFactor);

private:
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    const QScopedPointer<class StandardDocumentationViewPrivate> d_ptr;
    Q_DECLARE_PRIVATE(StandardDocumentationView)
};

}
#endif // KDEVPLATFORM_STANDARDDOCUMENTATIONVIEW_H
