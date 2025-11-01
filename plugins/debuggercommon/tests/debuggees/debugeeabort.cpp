/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/*



This padding is here to match relevant line numbers of debugeecrash.cpp.



*/

#undef NDEBUG
#include <cassert>
#include <cstdio>

int main()
{
    auto x = 0;
    std::puts("will abort now");
    assert(x);
}
