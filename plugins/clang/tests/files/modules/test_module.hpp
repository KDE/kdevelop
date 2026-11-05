/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEST_MODULE_HPP
#define TEST_MODULE_HPP

#include "test_module_export.h"

namespace module_example {
    class SubModule;
    class Module {
    public:
        virtual void copy(const Module& rhs) = 0;
        static TEST_MODULE_EXPORT auto create() -> Module&;
        static TEST_MODULE_EXPORT auto create(const Module& rhs) -> Module&;
        static TEST_MODULE_EXPORT void destroy(Module& rhs);

        virtual auto getSubModule() const -> const SubModule& = 0;
        virtual auto getSubModule() -> SubModule& = 0;
        virtual void setSubModule(const SubModule& sub) = 0;
    protected:
        Module();
        ~Module() noexcept;
        // Rule of 5
        Module(const Module&);
        auto operator=(const Module& rhs) -> Module&;
        Module(Module&&) noexcept;
        auto operator=(Module&& rhs) noexcept -> Module&;
    };
} // namespace module_example

#endif // TEST_MODULE_HPP
