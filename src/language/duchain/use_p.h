/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2006 Hamish Rodda <rodda@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef USE_P_H
#define USE_P_H

namespace KDevelop
{
class Use;
class UsePrivate
{
public:
  UsePrivate( Use* );
  DUContext* m_context;
  Declaration* m_declaration;
  Use* m_use;
  /**
   * Set the declaration for this use.
   *
   * \note Does not require the chain to be locked.
   */
  void setDeclaration(Declaration* definition);
};
}

#endif

//kate: space-indent on; indent-width 2; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
