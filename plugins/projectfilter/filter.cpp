/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "filter.h"

#include <KConfigGroup>

#include <array>

using namespace KDevelop;

namespace {
// loosely based on QRegularExpression::wildcardToRegularExpression
// but here we actually allow patterns to include slashes and we always assume that inputs use the
// Qt-internal way to represent paths with forward slashes, i.e. no native windows paths with backslashes
// furthermore we support escaping like in WildcardUnix
QString wildcardToPattern(const QString& pattern)
{
    QString ret;
    if (pattern.isEmpty())
        return ret;

    const auto inputLength = pattern.length();
    // +6 for anchor
    ret.reserve(inputLength + 6 + inputLength / 16);

    // anchor
    ret += QLatin1String("^.*");

    auto it = pattern.begin();
    const auto end = pattern.end();
    while (it != end) {
        const auto c = *it;
        ++it;

        switch (c.unicode()) {
        case '*':
            ret += QLatin1String(".*");
            break;
        case '?':
            ret += QLatin1Char('.');
            break;
        case '\\':
            // escape, as supported by QRegExp::WildcardUnix
            ret += c;
            if (it != end) {
                ret += *it;
                ++it;
            }
            break;
        case '/':
        case '$':
        case '(':
        case ')':
        case '+':
        case '.':
        case '^':
        case '{':
        case '|':
        case '}':
            ret += QLatin1Char('\\');
            ret += c;
            break;
        case '[':
            ret += c;
            // Support for the [!abc] or [!a-c] syntax
            if (it != end) {
                if (*it == QLatin1Char('!')) {
                    ret += QLatin1Char('^');
                    ++it;
                }

                while (it != end && *it != QLatin1Char(']')) {
                    if (*it == QLatin1Char('\\'))
                        ret += QLatin1Char('\\');
                    ret += *it;
                    ++it;
                }
            }
            break;
        default:
            ret += c;
            break;
        }
    }

    // anchor
    ret += QLatin1String(".*$");

    return ret;
}
}

Filter::Filter()
    : targets(Files | Folders)
{
}

Filter::Filter(const SerializedFilter& filter)
    : pattern(QString(), QRegularExpression::DontCaptureOption)
    , targets(filter.targets)
    , type(filter.type)
{
    switch (filter.style) {
    case Wildcard: {
        auto wildcardPattern = filter.pattern;
        if (!filter.pattern.startsWith(QLatin1Char('/')) && !filter.pattern.startsWith(QLatin1Char('*'))) {
            // implicitly match against trailing relative path
            wildcardPattern.prepend(QLatin1String("*/"));
        }

        if (wildcardPattern.endsWith(QLatin1Char('/')) && targets != Filter::Files) {
            // implicitly match against folders
            targets = Filter::Folders;
            wildcardPattern.chop(1);
        }

        pattern.setPattern(wildcardToPattern(wildcardPattern));
        break;
    }
    case RegularExpression:
        pattern.setPattern(filter.pattern);
        break;
    }

    pattern.optimize();
}

SerializedFilter::SerializedFilter()
    : targets(Filter::Files | Filter::Folders)

{

}

SerializedFilter::SerializedFilter(const QString& pattern, Filter::Targets targets, Filter::Type type,
                                   Filter::PatternStyle style)
    : pattern(pattern)
    , targets(targets)
    , type(type)
    , style(style)
{

}

namespace KDevelop {

SerializedFilters defaultFilters()
{
    SerializedFilters ret;
    ret.reserve(41);

    // filter hidden files/folders but (potentially) allow files within hidden folders, if the folders
    // get matched by a specific inclusion rule further below
    ret << SerializedFilter(QStringLiteral("/\\.[^/]*$"), Filter::Targets(Filter::Files | Filter::Folders),
                            Filter::Exclusive, Filter::RegularExpression);
    // but do show some with special meaning

    static const std::array<QString, 19> configFiles = {
        // Version control
        QStringLiteral(".gitignore"),
        QStringLiteral(".gitmodules"),
        // https://pre-commit.com/
        QStringLiteral(".pre-commit-config.yaml"),

        // CI config files

        // https://docs.gitlab.com/ee/ci/yaml/
        QStringLiteral(".gitlab-ci.yml"),
        // https://travis-ci.org/
        QStringLiteral(".travis.yml"),

        // Code quality configs

        //   https://github.com/c4urself/bump2version/
        QStringLiteral(".bumpversion.cfg"),
        //   https://clang.llvm.org/docs/ClangFormat.html
        QStringLiteral(".clang-format"),
        //   https://github.com/rust-lang/rust-clippy#configure-the-behavior-of-some-lints
        QStringLiteral(".clippy.toml"),
        //   https://github.com/codespell-project/codespell#using-a-config-file
        QStringLiteral(".codespellrc"),
        //   https://editorconfig.org/
        QStringLiteral(".editorconfig"),
        //   https://pycqa.github.io/isort/docs/configuration/config_files.html
        QStringLiteral(".isort.cfg"),
        //   https://mypy.readthedocs.io/en/stable/config_file.html
        QStringLiteral(".mypy.ini"),
        //   https://pep8.readthedocs.io
        QStringLiteral(".pep8"),
        //   https://prettier.io/
        QStringLiteral(".prettierignore"),
        QStringLiteral(".prettierrc*"),
        //   https://www.pydocstyle.org/en/stable/usage.html#configuration-files
        QStringLiteral(".pydocstyle*"),
        //   https://pylint.readthedocs.io/en/stable/user_guide/usage/run.html#command-line-options
        QStringLiteral(".pylintrc"),
        //   https://docs.readthedocs.io/en/stable/config-file/v2.html
        QStringLiteral(".readthedocs.y*ml"),
        //   https://yamllint.readthedocs.io/en/stable/configuration.html#configuration
        QStringLiteral(".yamllint*"),
    };
    for (const QString& file : configFiles) {
        ret << SerializedFilter(file, Filter::Files, Filter::Inclusive);
    }

    static const std::array<QString, 1> configFolders = {
        // CI config folders

        // https://circleci.com/docs/
        QStringLiteral(".circleci"),
    };
    for (const QString& folder : configFolders) {
        ret << SerializedFilter(folder, Filter::Folders, Filter::Inclusive);
    }

    // common vcs folders which we want to hide
    static const std::array<QString, 9> invalidFolders = {
        QStringLiteral(".git"), QStringLiteral("CVS"), QStringLiteral(".svn"), QStringLiteral("_svn"),
        QStringLiteral("SCCS"), QStringLiteral("_darcs"), QStringLiteral(".hg"), QStringLiteral(".bzr"), QStringLiteral("__pycache__")
    };
    for (const QString& folder : invalidFolders) {
        ret << SerializedFilter(folder, Filter::Folders);
    }

    // common files which we want to hide
    static const std::array<QString, 20> filePatterns = {
        // binary files (Unix)
        QStringLiteral("*.o"), QStringLiteral("*.a"), QStringLiteral("*.so"), QStringLiteral("*.so.*"),
        // binary files (Windows)
        QStringLiteral("*.obj"), QStringLiteral("*.lib"), QStringLiteral("*.dll"), QStringLiteral("*.exp"), QStringLiteral("*.pdb"),
        // generated files
        QStringLiteral("moc_*.cpp"), QStringLiteral("*.moc"), QStringLiteral("ui_*.h"), QStringLiteral("*.qmlc"), QStringLiteral("qrc_*.cpp"),
        // backup files
        QStringLiteral("*~"), QStringLiteral("*.orig"), QStringLiteral(".*.kate-swp"), QStringLiteral(".*.swp"),
        // python cache and object files
        QStringLiteral("*.pyc"), QStringLiteral("*.pyo")
    };
    for (const QString& filePattern : filePatterns) {
        ret << SerializedFilter(filePattern, Filter::Files);
    }

    return ret;
}

SerializedFilters readFilters(const KSharedConfigPtr& config)
{
    if (!config->hasGroup("Filters")) {
        return defaultFilters();
    }
    const KConfigGroup& group = config->group("Filters");
    const int size = group.readEntry("size", -1);
    if (size == -1) {
        // fallback
        return defaultFilters();
    }

    SerializedFilters filters;
    filters.reserve(size);
    for (int i = 0; i < size; ++i) {
        const QByteArray subGroup = QByteArray::number(i);
        if (!group.hasGroup(subGroup)) {
            continue;
        }
        const KConfigGroup& subConfig = group.group(subGroup);
        const QString pattern = subConfig.readEntry("pattern", QString());
        Filter::Targets targets(subConfig.readEntry("targets", 0));
        Filter::Type type = static_cast<Filter::Type>(subConfig.readEntry("inclusive", 0));
        Filter::PatternStyle style = static_cast<Filter::PatternStyle>(subConfig.readEntry("style", 0));
        filters << SerializedFilter(pattern, targets, type, style);
    }

    return filters;
}

void writeFilters(const SerializedFilters& filters, KSharedConfigPtr config)
{
    // clear existing
    config->deleteGroup("Filters");

    // write new
    KConfigGroup group = config->group("Filters");
    group.writeEntry("size", filters.size());
    int i = 0;
    for (const SerializedFilter& filter : filters) {
        KConfigGroup subGroup = group.group(QByteArray::number(i++));
        subGroup.writeEntry("pattern", filter.pattern);
        subGroup.writeEntry("targets", static_cast<int>(filter.targets));
        subGroup.writeEntry("inclusive", static_cast<int>(filter.type));
        subGroup.writeEntry("style", static_cast<int>(filter.style));
    }
    config->sync();
}

Filters deserialize(const SerializedFilters& filters)
{
    Filters ret;
    ret.reserve(filters.size());
    for (const SerializedFilter& filter : filters) {
        ret << Filter(filter);
    }
    return ret;
}

}
