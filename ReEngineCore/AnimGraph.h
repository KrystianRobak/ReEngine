#pragma once

#include <imgui/imgui.h>
#include <string>
#include <vector>
#include <map>
#include <variant>

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

enum class ConditionOp { Greater, Less, Equal, NotEqual };

struct GraphTransition {
    int ID;
    int FromNodeID;
    int ToNodeID;
    std::string ConditionParam;
    ConditionOp Operation;
    float Threshold;
};

struct GraphNode {
    int ID;

    std::string Name = "New State";
    std::string AnimationPath;
    ImVec2 EditorPosition;
    bool IsLooping = true;
    std::vector<GraphTransition> Transitions;
};

struct AnimationGraphResource {
    std::vector<GraphNode> Nodes;
    std::vector<GraphTransition> Transitions;
    int EntryNodeID = -1;

    std::unordered_map<std::string, AnimVar> DefaultBlackboard;

    int NextNodeID = 0;
    int NextLinkID = 0;
};
