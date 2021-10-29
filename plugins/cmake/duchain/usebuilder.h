/*
    SPDX-FileCopyrightText: 2014 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef USEBUILDER_H
#define USEBUILDER_H

#include <util/stack.h>

#include "contextbuilder.h"

class UseBuilder
{
public:
    explicit UseBuilder(const KDevelop::ReferencedTopDUContext& ctx);

    void startVisiting(CMakeContentIterator* node);

private:
    void newUse(const KDevelop::RangeInRevision& sr, const KDevelop::DeclarationPointer& d);

    KDevelop::ReferencedTopDUContext m_ctx;
};

#endif // USEBUILDER_H
