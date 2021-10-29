/*
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "codecompletionitemgrouper.h"

using namespace KDevelop;

///@todo make configurable. These are the attributes that can be respected for grouping.
const int SimplifiedAttributesExtractor::groupingProperties =
    CodeCompletionModel::Public
    | CodeCompletionModel::Protected
    | CodeCompletionModel::Private
    | CodeCompletionModel::Static
    | CodeCompletionModel::TypeAlias
    | CodeCompletionModel::Variable
    | CodeCompletionModel::Class
    | CodeCompletionModel::GlobalScope
    | CodeCompletionModel::LocalScope
    | CodeCompletionModel::NamespaceScope;
