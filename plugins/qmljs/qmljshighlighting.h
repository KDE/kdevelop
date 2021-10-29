/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMLJSHIGHLIGHTING_H
#define QMLJSHIGHLIGHTING_H

#include <language/highlighting/codehighlighting.h>

class QmlJsHighlighting : public KDevelop::CodeHighlighting
{
    Q_OBJECT

public:
    explicit QmlJsHighlighting(QObject* parent);
    KDevelop::CodeHighlightingInstance* createInstance() const override;
};

#endif // QMLJSHIGHLIGHTING_H
