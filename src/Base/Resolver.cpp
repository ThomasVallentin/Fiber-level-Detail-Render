#include "Resolver.h"

#include "Logging.h"


Resolver* Resolver::s_instance = nullptr;


Resolver& Resolver::Init(const fs::path& rootPath) 
{
    if (Resolver::s_instance) {
        LOG_WARNING("Resolver already exists, cannot Init() it twice.");
        return *s_instance;
    }

    s_instance = new Resolver(rootPath);
    return *s_instance;
}

fs::path Resolver::Resolve(const fs::path& identifier) const
{
    fs::path normPath = fs::path(identifier).lexically_normal();
    if (normPath.is_relative())
    {
        return m_rootPath / normPath;
    }

    return identifier;
}

// Identifiers

fs::path Resolver::AsIdentifier(const fs::path& path) const
{
    fs::path normPath = path.lexically_normal();
    if (normPath.is_relative())
    {
        return normPath;
    }

    for (auto it=normPath.begin(), otherIt=m_rootPath.begin() ; otherIt != m_rootPath.end() ; it++, otherIt++)
    {
        if (it == normPath.end() || *it != *otherIt)
        {
            return normPath;
        }
    }

    return normPath.lexically_relative(m_rootPath);
}
