#pragma once

#include <imgui/imgui.h>
#include <string>
#include <vector>
#include <map>
#include <variant> // or use a union struct if C++17 isn't available

// --- 1. Blackboard Variable Types ---
enum class AnimVarType { Bool, Float, Int, Trigger };

struct AnimVar {
    AnimVarType Type;
    union {
        bool bVal;
        float fVal;
        int iVal;
    };

    AnimVar() : Type(AnimVarType::Bool), bVal(false) {}
    AnimVar(bool v) : Type(AnimVarType::Bool), bVal(v) {}
    AnimVar(float v) : Type(AnimVarType::Float), fVal(v) {}
    AnimVar(int v) : Type(AnimVarType::Int), iVal(v) {}
};


// --- Graph Structures ---
enum class ConditionOp { Greater, Less, Equal, NotEqual };

struct GraphTransition {
    int ID;
    int FromNodeID;
    int ToNodeID;
    std::string ConditionParam; // e.g., "Speed"
    ConditionOp Operation;
    float Threshold;
};

struct GraphNode {
    int ID;

    std::string Name = "New State"; // The display name (e.g., "Idle")
    std::string AnimationPath;      // The actual asset path
    ImVec2 EditorPosition;     // For ImNodes
    bool IsLooping = true;
    std::vector<GraphTransition> Transitions;
};

struct AnimationGraphResource {
    std::vector<GraphNode> Nodes;
    std::vector<GraphTransition> Transitions;
    int EntryNodeID = -1;

    std::unordered_map<std::string, AnimVar> DefaultBlackboard;

    // Editor State helpers
    int NextNodeID = 0;
    int NextLinkID = 0;
};
