#pragma once
#include "System/System.h"
#include "ThreadPool.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <future>

class SystemGraph {
public:
    SystemGraph(ThreadPool* pool) : m_ThreadPool(pool) {}

    void AddSystem(System* sys) {
        if (!sys) return;
        m_Systems.push_back(sys);
        m_SystemMap[sys->SystemName] = sys;
    }

    void Build() {
        m_ExecutionWaves.clear();

        // 1. Build Adjacency List
        std::unordered_map<std::string, std::vector<std::string>> adjList;
        std::unordered_map<std::string, int> inDegree;

        // Init degrees
        for (auto* sys : m_Systems) inDegree[sys->SystemName] = 0;

        for (auto* sys : m_Systems) {
            // "RunAfter": Dependency -> Sys
            for (const auto& dep : sys->RunAfter) {
                if (m_SystemMap.find(dep) != m_SystemMap.end()) {
                    adjList[dep].push_back(sys->SystemName);
                    inDegree[sys->SystemName]++;
                }
            }
            // "RunBefore": Sys -> Dependant
            for (const auto& dep : sys->RunBefore) {
                if (m_SystemMap.find(dep) != m_SystemMap.end()) {
                    adjList[sys->SystemName].push_back(dep);
                    inDegree[dep]++;
                }
            }
        }

        // 2. Topological Sort (Kahn's Algo)
        std::vector<System*> currentWave;
        for (auto* sys : m_Systems) {
            if (inDegree[sys->SystemName] == 0) currentWave.push_back(sys);
        }

        while (!currentWave.empty()) {
            m_ExecutionWaves.push_back(currentWave);
            std::vector<System*> nextWave;

            for (auto* finishedSys : currentWave) {
                if (adjList.find(finishedSys->SystemName) != adjList.end()) {
                    for (const auto& neighbor : adjList[finishedSys->SystemName]) {
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

    void Execute(float dt) {
        for (auto& wave : m_ExecutionWaves) {
            if (wave.empty()) continue;

            // 1. If only one system, or specific constraints, run on Main Thread
            if (wave.size() == 1) {
                wave[0]->Update(dt);
            }
            else {
                // 2. Dispatch Parallel
                std::vector<std::future<void>> futures;
                for (auto* sys : wave) {
                    if (sys->RunOnMainThread) {
                        sys->Update(dt); // Run immediately (blocks main thread but safe)
                    }
                    else {
                        futures.push_back(m_ThreadPool->submit(JobType::General, [sys, dt]() {
                            sys->Update(dt);
                            }));
                    }
                }
                // SYNC POINT: Wait for wave completion
                for (auto& f : futures) f.wait();
            }
        }
    }

private:
    ThreadPool* m_ThreadPool;
    std::vector<System*> m_Systems;
    std::unordered_map<std::string, System*> m_SystemMap;
    std::vector<std::vector<System*>> m_ExecutionWaves;
};