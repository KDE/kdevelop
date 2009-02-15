/*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/

/*! Trivial example intended to showcase the KDevelop4 xTest runner */

#ifndef MONEY_H
#define MONEY_H

typedef struct Money Money;

Money *money_create (int amount, char *currency);
int money_amount (Money * m);
char *money_currency (Money * m);
void money_free (Money * m);

#endif /* MONEY_H */
