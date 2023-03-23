#include "Profiler.h"


Profiler* Profiler::s_instance = nullptr;


// == ProfilingScope ==

ProfilingScope::ProfilingScope(const std::string& name)
{
    auto& profiler = Profiler::Get();
    m_data = profiler.AddData(name, profiler.GetTime());
}

ProfilingScope::~ProfilingScope()
{
    auto& profiler = Profiler::Get();
    m_data->duration = profiler.GetTime() - m_data->start; 
}

// == Profiler ==

Profiler::Profiler(const Window& window) : m_window(window) {}

Profiler& Profiler::Init(const Window& window)
{
    if (!s_instance)
    {
        s_instance = new Profiler(window);
    }

    return *s_instance;
}

ProfilingScopeData* Profiler::AddData(const std::string& name, const double& start)
{
    return &m_scopes.emplace_back(name, m_window.GetTime());
}
