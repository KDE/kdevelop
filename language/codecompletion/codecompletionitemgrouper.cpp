/*
 * KDevelop Generic Code Completion Support
 *
 * Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
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

#include "codecompletionitemgrouper.h"

using namespace KDevelop;

///@todo make configurable. These are the attributes that can be respected for grouping.
const int SimplifiedAttributesExtractor::groupingProperties = CodeCompletionModel::Public | CodeCompletionModel::Protected | CodeCompletionModel::Private | CodeCompletionModel::Static | CodeCompletionModel::TypeAlias | CodeCompletionModel::Variable | CodeCompletionModel::Class | CodeCompletionModel::GlobalScope | CodeCompletionModel::LocalScope | CodeCompletionModel::GlobalScope | CodeCompletionModel::NamespaceScope;

