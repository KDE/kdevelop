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

/*! Examples and howto for unit under test generation from a test-driven
    implementation. */

/*!
This is your everyday test first test command. You have 
coded the Baz class by intention and now want to generate
a skeleton implementation.

Steps:
  1/ Move your cursor to baz
  2/ Notice the neat code highlighting
  3/ Right click to spawn a context menu 
  4/ Choose ``Generate Unit Under Test'' 
  5/ A dialog box will appear. Provide a filename
     This file will contain the generated code
  6/ Hit ok.
  7/ Unit Under Test skeleton is generated

*/
class Foo;
class Bar;
void FooTest::foo()
{
    Baz baz;
    baz.moo();
    baz.zoo(Foo(), Bar());
}

