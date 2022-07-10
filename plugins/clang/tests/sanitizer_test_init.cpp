/*
    SPDX-FileCopyrightText: 2022 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "sanitizer_test_init.h"

#include <QString>
#include <QtGlobal>

#if defined(Q_OS_UNIX)
#include <unistd.h>
#endif

namespace KDevelop
{
namespace
{
    /**
     * LLVM and ASAN don't play well together, see [1].
     * We have to disable sigaltstack or we crash.
     * The code below detects whether we need to disable this option.
     * If yes, we change the environment and re-execute ourselves with the new
     * environment. Sadly, it doesn't seem to work to use __asan_default_options
     * for this purpose here...
     *
     * [1]: https://github.com/google/sanitizers/issues/849
     */
    [[maybe_unused]] void setupAsan(char** argv)
    {
        auto asanOptions = qEnvironmentVariable("ASAN_OPTIONS");
        if (asanOptions.contains(QLatin1String("use_sigaltstack"))) {
            return; // nothing to do
        }

        asanOptions += QLatin1String(",use_sigaltstack=0");
        qputenv("ASAN_OPTIONS", asanOptions.toUtf8());

        execv(argv[0], argv);
    }
}

void sanitizerTestInit([[maybe_unused]] char** argv)
{
#if defined(Q_OS_UNIX)

// gcc
#if defined(__SANITIZE_ADDRESS__)
    setupAsan(argv);
    return;
#endif

// clang
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
    setupAsan(argv);
    return;
#endif
#endif

#endif
}
}
