/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QString>

#include <cstdio>

int f()
{
    return 5;
}

int g()
{
    return 7;
}

void h()
{
    std::puts("hi");
}

double fp()
{
    return 2.5;
}

QString theName()
{
    return "Bob";
}

int main()
{
    f();
    g();
    h();

    const auto x = fp();
    const auto name = theName();
    return x + name.size();
}
