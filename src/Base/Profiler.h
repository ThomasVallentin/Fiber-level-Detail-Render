#ifndef PROFILER_H
#define PROFILER_H

#include "Window.h"

#include <string>
#include <vector>


// The data hold by the profiler
struct ProfilingScopeData
{
    ProfilingScopeData(const std::string& name, 
                       const double& start) : 
            name(name), 
            start(start) {}

    std::string name;
    double start;
    double duration = 0.0;
    // ProfilingScopeData* parent;
};


// Utility class to create scopes
class ProfilingScope
{
public:
    ProfilingScope(const std::string& name);
    ~ProfilingScope();

    inline std::string GetName() const { return m_data->name; }
    inline double GetStart() const { return m_data->start; }
    inline double GetDuration() const { return m_data->duration; }

private:
    ProfilingScopeData* m_data;
};


// The manager class that handles all the scopes
class Profiler
{
public:
    static Profiler& Init(const Window& window);
    inline static Profiler& Get() { return *s_instance; };

    inline const std::vector<ProfilingScopeData>& GetScopes() const { return m_scopes; }
    inline void Clear() { m_scopes.clear(); }

private:
    Profiler(const Window& window);
    ~Profiler() = default;

    // Private functions only used by ProfilingScope that allow them to create new entries in the Profiler
    inline double GetTime() const { return m_window.GetTime(); }
    ProfilingScopeData* AddData(const std::string& name, const double& start);

    friend ProfilingScope;
    std::vector<ProfilingScopeData> m_scopes;

    const Window& m_window;
    static Profiler* s_instance;
};

#endif