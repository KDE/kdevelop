/*
 * KDevelop Code Completion Support for file includes
 *
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
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

#ifndef KDEVPLATFORM_ABSTRACTINCLUDEFILECOMPLETIONITEM_H
#define KDEVPLATFORM_ABSTRACTINCLUDEFILECOMPLETIONITEM_H

#include "codecompletionitem.h"
#include "languageexport.h"
#include "../util/includeitem.h"
#include "../duchain/duchain.h"
#include "../duchain/duchainlock.h"
#include "codecompletionmodel.h"

namespace KDevelop {

//A completion item used for completing include-files
template<typename NavigationWidget>
class AbstractIncludeFileCompletionItem : public CompletionTreeItem {
public:
  AbstractIncludeFileCompletionItem(const IncludeItem& include) : includeItem(include) {
  }

  virtual QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const {
    DUChainReadLocker lock(DUChain::lock(), 500);
    if(!lock.locked()) {
      kDebug() << "Failed to lock the du-chain in time";
      return QVariant();
    }

    const IncludeItem& item( includeItem );

    switch (role) {
      case CodeCompletionModel::IsExpandable:
        return QVariant(true);
      case CodeCompletionModel::ExpandingWidget: {
        NavigationWidget* nav = new NavigationWidget(item, model->currentTopContext());
        model->addNavigationWidget(this, nav);

        QVariant v;
        v.setValue<QWidget*>((QWidget*)nav);
        return v;
      }
      case Qt::DisplayRole:
        switch (index.column()) {
          case CodeCompletionModel::Prefix:
            if(item.isDirectory)
              return QVariant("directory");
            else
              return QVariant("file");
          case CodeCompletionModel::Name: {
            return item.isDirectory ? item.name + '/' : item.name;
          }
        }
        break;
      case CodeCompletionModel::ItemSelected:
      {
        return QVariant( NavigationWidget::shortDescription(item) );
      }
    }

    return QVariant();
  }

  virtual void execute(KTextEditor::View* view, const KTextEditor::Range& word) = 0;

  virtual int inheritanceDepth() const {
    return includeItem.pathNumber;
  }
  virtual int argumentHintDepth() const {
    return 0;
  }

  IncludeItem includeItem;
};

}

#endif // KDEVPLATFORM_ABSTRACTINCLUDEFILECOMPLETIONITEM_H
