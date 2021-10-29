/*
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CLANG_CLANGHIGHLIGHTING_H
#define CLANG_CLANGHIGHLIGHTING_H

#include <language/highlighting/codehighlighting.h>

class ClangHighlighting : public KDevelop::CodeHighlighting
{
    Q_OBJECT
public:
    explicit ClangHighlighting(QObject* parent);

    KDevelop::CodeHighlightingInstance* createInstance() const override;

private:
    class Instance;
};

#endif // CLANG_CLANGHIGHLIGHTING_H
