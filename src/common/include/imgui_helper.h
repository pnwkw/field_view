#ifndef COMMON_IMGUI_HELPER_H
#define COMMON_IMGUI_HELPER_H

namespace viz {
    static void HelpMarker(const char* desc) {
        ImGui::TextDisabled("(?)");

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
}

#endif //COMMON_IMGUI_HELPER_H
