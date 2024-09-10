/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_DOCUMENTSWITCHERITEM_H
#define KDEVPLATFORM_PLUGIN_DOCUMENTSWITCHERITEM_H

#include <QStandardItem>
#include <QMimeType>

namespace KDevelop
{
    class IDocument;
}

/** Helper for list model items.
 * Extracts icon and text from document.
 */
class DocumentSwitcherItem : public QStandardItem
{
public:
    explicit DocumentSwitcherItem(KDevelop::IDocument* document, std::unordered_map<KDevelop::IDocument*, QMimeType>& mimeCache);
    ~DocumentSwitcherItem() override;
};

#endif // KDEVPLATFORM_PLUGIN_DOCUMENTSWITCHERITEM_H
