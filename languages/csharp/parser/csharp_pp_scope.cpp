/*****************************************************************************
 * Copyright (c) 2006 Jakob Petsovits <jpetso@gmx.at>                        *
 *                                                                           *
 * This program is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This grammar is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "csharp_pp_scope.h"


namespace csharp_pp
{

scope::scope( csharp::parser* parser )
 : _M_type(scope::type_root), _M_parser(parser), _M_child(0), _M_parent(0)
{
  _M_is_active = true;
  _M_waiting_for_active_section = true;
}

scope::scope( scope::scope_type _type, csharp::parser* _parser, bool active )
 : _M_type(_type), _M_parser(_parser),  _M_child(0)
{
  if (active == true)
    {
      _M_waiting_for_active_section = true;
      _M_is_active = true;
    }
  else
    {
      _M_waiting_for_active_section = false;
      _M_is_active = false;
    }
}

scope::~scope()
{
  if (_M_child != 0)
    delete _M_child;
  if (_M_parent != 0)
    _M_parent->_M_child = 0;
}

scope* scope::current_scope()
{
  if (_M_child == 0)
    return this;
  else
    return _M_child->current_scope();
}

scope::scope_type scope::type()
{
  return _M_type;
}

bool scope::is_active()
{
  return _M_is_active;
}

bool scope::is_waiting_for_active_section()
{
  return _M_waiting_for_active_section;
}

void scope::set_activated( bool active )
{
  if (_M_type != scope::type_if)
    return;
  else if (_M_waiting_for_active_section == false)
    _M_is_active = false;
  else
    {
      _M_is_active = active;

      if (active == true)
        _M_waiting_for_active_section = false;
    }
}

bool scope::push_scope( scope::scope_type type, scope** new_current )
{
  if (_M_child != 0) {
    *new_current = _M_child->current_scope();
    return false; // only the deepest nested scope shall create children
  }

  _M_child = new scope(type, _M_parser, _M_is_active);
  _M_child->set_parent(this);
  *new_current = _M_child;
  return true;
}

bool scope::pop_scope( scope::scope_type type, scope** new_current )
{
  if (_M_parent == 0 || type != _M_type) {
    *new_current = this->current_scope();
    return false;
  }

  *new_current = _M_parent;
  delete this;
  return true;
}

void scope::set_csharp_parser( ::csharp::parser* parser )
{
  _M_parser = parser;
}

::csharp::parser* scope::csharp_parser()
{
  return _M_parser;
}

void scope::set_parent( scope* parent )
{
  _M_parent = parent;
}

scope* scope::parent()
{
  return _M_parent;
}

} // end of namespace csharp_pp
