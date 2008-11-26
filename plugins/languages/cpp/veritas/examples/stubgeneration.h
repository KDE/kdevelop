/*
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

/*! Examples and howto for stub code generation */

/*!
This is your everyday interface. Now we want to construct
a stub class skeleton for the virtual methods.

Steps:
  1/ Point your mouse cursor to the first 'Foo'
  2/ Notice the neat code highlighting
  3/ Right click to spawn a context menu 
  4/ Choose ``Generate Stub Class'' 
  5/ A dialog box will appear. Provide a filename
     This file will contain the generated code
  6/ Hit ok.
  7/ Stub is generated and focused.
*/
class Foo
{
public:
    Foo();
    virtual ~Foo();
    virtual int foo(char);
};

/*! Only virtual methods are stubbed. No code is generated 
 *  for non-virtual, signals nor for private methods. */
class Bar
{
public:
    void bar();
signals:
    void barr();
private:
    void barrr();
};

namespace Zoo
{
class Loo
{
public:
    virtual void moo();
};
}


