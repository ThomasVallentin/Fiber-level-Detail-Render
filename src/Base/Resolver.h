#ifndef RESOLVER_H
#define RESOLVER_H

#include <filesystem>

namespace fs = std::filesystem;

class Resolver
{
public:
    static Resolver& Init(const fs::path& rootPath);
    inline static Resolver& Get() { return *s_instance; }

    // Identifier utils
    fs::path Resolve(const fs::path& identifier) const;
    fs::path AsIdentifier(const fs::path& path) const;

private:
    Resolver(const fs::path& rootPath) : m_rootPath(rootPath) {}
    ~Resolver() = default;

    fs::path m_rootPath;

    static Resolver* s_instance;
};

#endif  // RESOLVER_H
