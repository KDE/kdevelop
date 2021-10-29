/*
    SPDX-FileCopyrightText: 2008 Aleix Pol <aleixpol@gmail.com>
    SPDX-FileCopyrightText: 2009 Ramón Zarazúa <killerfox512+kde@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_APPLYCHANGESWIDGET_H
#define KDEVPLATFORM_APPLYCHANGESWIDGET_H

#include <QDialog>
#include <language/languageexport.h>

namespace KTextEditor {
class Document;
}

namespace KDevelop {
class IndexedString;
class ApplyChangesWidgetPrivate;

class KDEVPLATFORMLANGUAGE_EXPORT ApplyChangesWidget
    : public QDialog
{
    Q_OBJECT

public:
    explicit ApplyChangesWidget(QWidget* parent = nullptr);
    ~ApplyChangesWidget() override;

    void setInformation(const QString& info);

    bool hasDocuments() const;

    KTextEditor::Document* document() const;

    ///@param original may be an artificial code representation @ref KDevelop::InsertArtificialCodeRepresentation
    void addDocuments(const IndexedString& original);

    ///This will save all the modified files into their originals
    bool applyAllChanges();

    ///Update the comparison view fo @p index, in case changes have been done directly to the opened document.
    ///@param index the index to update, -1 for current index
    void updateDiffView(int index = -1);

public Q_SLOTS:
    ///Called to signal a change to the currently viewed index
    void indexChanged(int);

private:
    const QScopedPointer<class ApplyChangesWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ApplyChangesWidget)
};
}

#endif
