/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEST_SUBMODULE_HPP
#define TEST_SUBMODULE_HPP

#include <string>

#include "test_module_export.h"

namespace module_example {
    class SubModule {
    public:
        virtual void copy(const SubModule& rhs) = 0;
        static TEST_MODULE_EXPORT auto create() -> SubModule&;
        static TEST_MODULE_EXPORT auto create(const SubModule& rhs) -> SubModule&;
        static TEST_MODULE_EXPORT void destroy(SubModule& rhs);

        [[nodiscard]] virtual auto getString() -> std::string& = 0;
        [[nodiscard]] virtual auto getString() const -> const std::string& = 0;
        virtual auto enableString() -> std::string& = 0;
        virtual auto hasString() const -> bool = 0;
        virtual void setString(const std::string& str) = 0;
    protected:
        SubModule();
        ~SubModule() noexcept = default;
        // Rule of 5
        SubModule(const SubModule&);
        auto operator=(const SubModule& rhs) -> SubModule&;
        SubModule(SubModule&&) noexcept;
        auto operator=(SubModule&& rhs) noexcept -> SubModule&;
    };
} // namespace module_example

#endif // TEST_SUBMODULE_HPP
