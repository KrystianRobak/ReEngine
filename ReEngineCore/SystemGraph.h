#pragma once
#include "System/System.h"
#include "ThreadPool.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <future>
#include <iostream>

class SystemGraph {
public:
    SystemGraph(ThreadPool* pool) : m_ThreadPool(pool) {}

    // Add a system to be scheduled
    void AddSystem(System* sys) {
        if (!sys) return;
        m_Systems.push_back(sys);
        m_SystemMap[sys->SystemName] = sys;
    }

    // Call this once after adding all systems (e.g., in Application::Init)
    void Build() {
        m_ExecutionWaves.clear();

        // 1. Build Adjacency List & In-Degrees
        std::unordered_map<std::string, std::vector<std::string>> adjList;
        std::unordered_map<std::string, int> inDegree;

        // Initialize counts
        for (auto* sys : m_Systems) inDegree[sys->SystemName] = 0;

        for (auto* sys : m_Systems) {
            // "RunAfter" means: Dependency -> Sys
            for (const auto& dep : sys->RunAfter) {
                if (m_SystemMap.find(dep) != m_SystemMap.end()) {
                    adjList[dep].push_back(sys->SystemName);
                    inDegree[sys->SystemName]++;
                }
            }
            // "RunBefore" means: Sys -> Dependent
            for (const auto& dep : sys->RunBefore) {
                if (m_SystemMap.find(dep) != m_SystemMap.end()) {
                    adjList[sys->SystemName].push_back(dep);
                    inDegree[dep]++;
                }
            }

            // "WriteComponents" implicit dependency:
            // If SysA writes 'Transform' and SysB writes 'Transform', make deterministic order?
            // For now, we rely on user explicit dependencies for write conflicts.
        }

        // 2. Kahn's Algorithm (Topological Sort into Layers)
        std::vector<System*> currentWave;
        for (auto* sys : m_Systems) {
            if (inDegree[sys->SystemName] == 0) currentWave.push_back(sys);
        }

        while (!currentWave.empty()) {
            m_ExecutionWaves.push_back(currentWave);
            std::vector<System*> nextWave;

            // Simulate completion of current wave
            for (auto* completedSys : currentWave) {
                if (adjList.find(completedSys->SystemName) != adjList.end()) {
                    for (const auto& neighbor : adjList[completedSys->SystemName]) {
                        inDegree[neighbor]--;
                        if (inDegree[neighbor] == 0) {
                            nextWave.push_back(m_SystemMap[neighbor]);
                        }
                    }
                }
            }
            currentWave = nextWave;
        }
    }

    // Call this every frame
    void Execute(float dt) {
        for (auto& wave : m_ExecutionWaves) {
            if (wave.empty()) continue;

            // Optimization: If wave has only 1 job, run it directly to avoid thread overhead
            if (wave.size() == 1) {
                wave[0]->Update(dt);
                continue;
            }

            // Dispatch Parallel Jobs
            std::vector<std::future<void>> futures;
            for (auto* sys : wave) {
                if (sys->RunOnMainThread) {
                    // Execute immediately on this thread
                    sys->Update(dt);
                }
                else {
                    // Send to ThreadPool
                    futures.push_back(m_ThreadPool->submit(JobType::General, [sys, dt]() {
                        sys->Update(dt);
                        }));
                }
            }

            // BARRIER: Wait for this wave to complete before starting the next
            for (auto& f : futures) {
                if (f.valid()) f.wait();
            }
        }
    }

private:
    ThreadPool* m_ThreadPool;
    std::vector<System*> m_Systems;
    std::unordered_map<std::string, System*> m_SystemMap;
    std::vector<std::vector<System*>> m_ExecutionWaves;
};