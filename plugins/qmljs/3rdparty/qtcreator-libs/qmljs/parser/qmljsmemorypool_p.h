/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qmljsglobal_p.h"
#include <QtCore/qstring.h>
#include <QtCore/qvector.h>

#include <cstdlib>

QT_QML_BEGIN_NAMESPACE

namespace QmlJS {

class Managed;

class MemoryPool
{
    Q_DISABLE_COPY_MOVE(MemoryPool);

public:
    MemoryPool() = default;
    ~MemoryPool()
    {
        if (_blocks) {
            for (int i = 0; i < _allocatedBlocks; ++i) {
                if (char *b = _blocks[i])
                    free(b);
            }

            free(_blocks);
        }
        qDeleteAll(strings);
    }

    inline void *allocate(size_t size)
    {
        size = (size + 7) & ~size_t(7);
        if (Q_LIKELY(_ptr && (_ptr + size < _end))) {
            void *addr = _ptr;
            _ptr += size;
            return addr;
        }
        return allocate_helper(size);
    }

    void reset()
    {
        _blockCount = -1;
        _ptr = _end = nullptr;
    }

    template <typename Tp> Tp *New() { return new (this->allocate(sizeof(Tp))) Tp(); }
    template <typename Tp, typename... Ta> Tp *New(Ta... args)
    { return new (this->allocate(sizeof(Tp))) Tp(args...); }

    QStringView newString(const QString &string) {
        strings.append(new QString(string));
        return QStringView(*strings.last());
    }

private:
    Q_NEVER_INLINE void *allocate_helper(size_t size)
    {
        size_t currentBlockSize = DEFAULT_BLOCK_SIZE;
        while (Q_UNLIKELY(size >= currentBlockSize))
            currentBlockSize *= 2;

        if (++_blockCount == _allocatedBlocks) {
            if (! _allocatedBlocks)
                _allocatedBlocks = DEFAULT_BLOCK_COUNT;
            else
                _allocatedBlocks *= 2;

            _blocks = reinterpret_cast<char **>(realloc(_blocks, sizeof(char *) * size_t(_allocatedBlocks)));
            Q_CHECK_PTR(_blocks);

            for (int index = _blockCount; index < _allocatedBlocks; ++index)
                _blocks[index] = nullptr;
        }

        char *&block = _blocks[_blockCount];

        if (! block) {
            block = reinterpret_cast<char *>(malloc(currentBlockSize));
            Q_CHECK_PTR(block);
        }

        _ptr = block;
        _end = _ptr + currentBlockSize;

        void *addr = _ptr;
        _ptr += size;
        return addr;
    }

private:
    char **_blocks = nullptr;
    int _allocatedBlocks = 0;
    int _blockCount = -1;
    char *_ptr = nullptr;
    char *_end = nullptr;
    QVector<QString*> strings;

    enum
    {
        DEFAULT_BLOCK_SIZE = 8 * 1024,
        DEFAULT_BLOCK_COUNT = 8
    };
};

class Managed
{
    Q_DISABLE_COPY(Managed)
public:
    Managed() = default;
    ~Managed() = default;

    void *operator new(size_t size, MemoryPool *pool) { return pool->allocate(size); }
    void operator delete(void *) {}
    void operator delete(void *, MemoryPool *) {}
};

} // namespace QmlJS

QT_QML_END_NAMESPACE

