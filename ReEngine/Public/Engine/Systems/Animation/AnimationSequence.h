//
// Created by ragberr on 22.01.2025.
//

#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include <vector>
#include <string>
#include <algorithm>
#include "Engine/Core/AnimationTypes.h"

class AnimationSequencer {
private:
    static constexpr float TIMELINE_HEIGHT = 25.0f;
    static constexpr float TRACK_HEIGHT = 32.0f;
    static constexpr float HEADER_WIDTH = 200.0f;
    static constexpr float KEYFRAME_SIZE = 12.0f;  // Increased from 8.0f
    static constexpr float KEYFRAME_HOVER_SCALE = 1.2f;  // Scale factor when hovering


    bool isDragging = false;
    int selectedTrack = -1;
    Keyframe* selectedKeyframe = nullptr;
    float currentTime = 0.0f;
    Keyframe* hoveredKeyframe = nullptr;

    // Helper function to create a new keyframe at the specified time
    Keyframe CreateKeyframe(float time) {
        Keyframe keyframe;
        keyframe.time = time;
        // Set default transform values - you might want to get these from the current entity state
        keyframe.position = glm::vec3(0.0f);
        keyframe.rotation = glm::vec3(0.0f);
        return keyframe;
    }

    // Helper function to ensure an animation exists for the track
    Animation& GetOrCreateAnimation(AnimationPath& track) {
        if (track.Animations.empty()) {
            Animation newAnim;
            track.Animations.push_back(newAnim);
        }
        return track.Animations[0]; // For now, we'll work with the first animation
    }

    // Helper function to insert a keyframe in sorted order
    void InsertKeyframeSorted(std::vector<Keyframe>& keyframes, const Keyframe& newKeyframe) {
        auto insertPos = std::lower_bound(keyframes.begin(), keyframes.end(), newKeyframe,
            [](const Keyframe& a, const Keyframe& b) { return a.time < b.time; });
        keyframes.insert(insertPos, newKeyframe);
    }

    void HandleKeyframeInteraction(const ImVec2& kfPos, Keyframe& keyframe, int trackIndex) {
        // Calculate the interaction area (slightly larger than visual size for easier clicking)
        float interactionSize = KEYFRAME_SIZE * 1.5f;  // Larger clickable area
        ImVec2 minPos(kfPos.x - interactionSize/2, kfPos.y - interactionSize/2);
        ImRect bounds(minPos, ImVec2(minPos.x + interactionSize, minPos.y + interactionSize));

        // Check for hover
        ImVec2 mousePos = ImGui::GetMousePos();
        bool isHovered = bounds.Contains(mousePos);

        if (isHovered) {
            hoveredKeyframe = &keyframe;

            // Handle click
            if (ImGui::IsMouseClicked(0)) {
                selectedKeyframe = &keyframe;
                selectedTrack = trackIndex;

                if (ImGui::IsMouseDoubleClicked(0)) {
                    isDragging = true;
                }
            }
        }

        // Draw the keyframe
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        DrawKeyframe(kfPos, &keyframe == selectedKeyframe, &keyframe == hoveredKeyframe, draw_list);
    }


public:
    ReSequencer* sequencer;

public:
    AnimationSequencer(ReSequencer* seq) : sequencer(seq) {}

    void Draw() {
        if (!sequencer) return;
        // Calculate dimensions
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();
        float timeline_width = canvas_size.x - HEADER_WIDTH;

        DrawTimelineRuler(canvas_pos, timeline_width);

        // Draw tracks with keyframes
        for (size_t i = 0; i < sequencer->AnimPaths.size(); i++) {
            DrawTrack(i, canvas_pos, timeline_width);
        }

        HandleDragging(canvas_pos, timeline_width);

        if (ImGui::Button("Add Track")) {
            AnimationPath newPath;
            newPath.Name = "Track " + std::to_string(sequencer->AnimPaths.size());
        }

        ImGui::SameLine();
        if (ImGui::Button("Delete Track") && selectedTrack >= 0) {
            sequencer->AnimPaths.erase(sequencer->AnimPaths.begin() + selectedTrack);
            selectedTrack = -1;
        }
    }

    Keyframe* GetSelectedKeyframe() { return selectedKeyframe; }
    int GetSelectedTrack() { return selectedTrack; }

private:
    void DrawTimelineRuler(const ImVec2& canvas_pos, float width) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        float start_x = canvas_pos.x + HEADER_WIDTH;
        float y = canvas_pos.y;

        // Draw timeline background
        draw_list->AddRectFilled(
            ImVec2(start_x, y),
            ImVec2(start_x + width, y + TIMELINE_HEIGHT),
            IM_COL32(40, 40, 40, 255)
        );

        // Draw time markers
        float time_span = sequencer->timeline_end - sequencer->timeline_start;
        float pixels_per_second = width / time_span;

        for (float t = sequencer->timeline_start; t <= sequencer->timeline_end; t += sequencer->TimeInterval) {
            float x = start_x + (t - sequencer->timeline_start) * pixels_per_second;

            // Draw marker line
            draw_list->AddLine(
                ImVec2(x, y),
                ImVec2(x, y + TIMELINE_HEIGHT),
                IM_COL32(100, 100, 100, 255)
            );

            // Draw time label
            char label[32];
            snprintf(label, sizeof(label), "%.1f", t);
            draw_list->AddText(
                ImVec2(x + 2, y + 2),
                IM_COL32(255, 255, 255, 255),
                label
            );
        }
    }

    void HandleDragging(const ImVec2& canvas_pos, float timeline_width) {
        if (ImGui::IsMouseDragging(0) && selectedKeyframe) {
            // Convert mouse position to time
            float mouseX = ImGui::GetMousePos().x - (canvas_pos.x + HEADER_WIDTH);
            float timeSpan = sequencer->timeline_end - sequencer->timeline_start;
            float newTime = (mouseX / timeline_width) * timeSpan + sequencer->timeline_start;

            // Clamp time to valid range
            newTime = std::clamp(newTime, sequencer->timeline_start, sequencer->timeline_end);
            selectedKeyframe->time = newTime;
        }

        if (ImGui::IsMouseReleased(0)) {
            isDragging = false;
        }
    }

    void DrawKeyframe(const ImVec2& pos, bool isSelected, bool isHovered, ImDrawList* draw_list) {
        // Calculate size based on state
        float size = KEYFRAME_SIZE * (isHovered ? KEYFRAME_HOVER_SCALE : 1.0f);
        float halfSize = size * 0.5f;

        // Define diamond points
        ImVec2 diamondPoints[4] = {
            ImVec2(pos.x, pos.y - halfSize),          // Top
            ImVec2(pos.x + halfSize, pos.y),          // Right
            ImVec2(pos.x, pos.y + halfSize),          // Bottom
            ImVec2(pos.x - halfSize, pos.y)           // Left
        };

        // Colors for different states
        ImU32 fillColor;
        if (isSelected) {
            fillColor = IM_COL32(255, 255, 0, 255);        // Yellow for selected
        } else if (isHovered) {
            fillColor = IM_COL32(255, 215, 0, 255);        // Gold for hovered
        } else {
            fillColor = IM_COL32(200, 200, 0, 255);        // Default color
        }

        // Draw keyframe with outline
        draw_list->AddConvexPolyFilled(diamondPoints, 4, fillColor);
        draw_list->AddPolyline(diamondPoints, 4, IM_COL32(0, 0, 0, 255), true, 2.0f);  // Thicker outline

        // Add inner highlight for better visibility
        if (isSelected || isHovered) {
            float innerScale = 0.7f;
            ImVec2 innerPoints[4] = {
                ImVec2(pos.x, pos.y - halfSize * innerScale),
                ImVec2(pos.x + halfSize * innerScale, pos.y),
                ImVec2(pos.x, pos.y + halfSize * innerScale),
                ImVec2(pos.x - halfSize * innerScale, pos.y)
            };
            draw_list->AddPolyline(innerPoints, 4, IM_COL32(255, 255, 255, 100), true, 1.0f);
        }
    }

    void DrawTrack(size_t index, const ImVec2& canvas_pos, float width) {
        AnimationPath* track = sequencer->AnimPaths[index];
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        float y = canvas_pos.y + TIMELINE_HEIGHT + (index * TRACK_HEIGHT);
        float start_x = canvas_pos.x + HEADER_WIDTH;

        hoveredKeyframe = nullptr;

        // Track header
        ImGui::SetCursorScreenPos(ImVec2(canvas_pos.x, y));
        ImGui::PushID(static_cast<int>(index));

        char name_buf[128];
        strncpy_s(name_buf, track->Name.c_str(), sizeof(name_buf) - 1);
        ImGui::SetNextItemWidth(HEADER_WIDTH - 30);
        if (ImGui::InputText("##name", name_buf, sizeof(name_buf))) {
            track->Name = name_buf;
        }

        // Track content area
        ImGui::SetCursorScreenPos(ImVec2(start_x, y));

        // Track background and interaction area
        ImGui::InvisibleButton(("track_area##" + std::to_string(index)).c_str(),
            ImVec2(width, TRACK_HEIGHT - 2));

        float timeSpan = sequencer->timeline_end - sequencer->timeline_start;

        // Handle double-click to create new keyframe
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            // Calculate time position from mouse
            float mouseX = ImGui::GetMousePos().x - start_x;
            timeSpan = sequencer->timeline_end - sequencer->timeline_start;
            float clickTime = (mouseX / width) * timeSpan + sequencer->timeline_start;

            // Create and add new keyframe
            Animation& animation = GetOrCreateAnimation(*track);
            Keyframe newKeyframe = CreateKeyframe(clickTime);
            InsertKeyframeSorted(animation.keyframes, newKeyframe);

            // Select the new keyframe
            selectedTrack = static_cast<int>(index);
            selectedKeyframe = &animation.keyframes.back();

            // Update animation times
            if (animation.keyframes.size() >= 2) {
                track->BeginTime = animation.keyframes.front().time;
                track->EndTime = animation.keyframes.back().time;
                track->Duration = track->EndTime - track->BeginTime;
            }
        }

        // Draw track background
        draw_list->AddRectFilled(
            ImVec2(start_x, y),
            ImVec2(start_x + width, y + TRACK_HEIGHT - 2),
            IM_COL32(60, 60, 60, 255)
        );

        // Draw animation preview if we have at least 2 keyframes
        for (auto& anim : track->Animations) {
            if (anim.keyframes.size() >= 2) {
                float start_time = anim.keyframes.front().time;
                float end_time = anim.keyframes.back().time;

                float clip_start_x = start_x + (start_time - sequencer->timeline_start) * (width / timeSpan);
                float clip_end_x = start_x + (end_time - sequencer->timeline_start) * (width / timeSpan);

                // Draw animation region
                draw_list->AddRectFilled(
                    ImVec2(clip_start_x, y + 2),
                    ImVec2(clip_end_x, y + TRACK_HEIGHT - 4),
                    IM_COL32(100, 150, 250, 180)
                );
            }

            // Draw keyframes
            for (auto& keyframe : anim.keyframes) {
                float kf_x = start_x + (keyframe.time - sequencer->timeline_start) * (width / timeSpan);
                float kf_y = y + TRACK_HEIGHT * 0.5f;

                HandleKeyframeInteraction(ImVec2(kf_x, kf_y), keyframe, static_cast<int>(index));
            }
        }

        ImGui::PopID();
    }
};
