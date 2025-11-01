/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/*




This padding is here to match relevant line numbers of debugeecrash.cpp.




*/

#include <cstdio>

int main()
{
    auto x = 0;
    std::puts("will perform an erroneous arithmetic operation now");
    const auto invalid = 1 / x;
    std::printf("%d\n", invalid);
}
