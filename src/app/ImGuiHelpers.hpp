#pragma once

#include "core/Common.hpp"
#include "core/Graph.hpp"

namespace ImGui
{
    // ImGui::InputText() with std::string
    // Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
    IMGUI_API bool  InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
    IMGUI_API bool  InputTextMultiline(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
    IMGUI_API bool  InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
}

struct TextCallbackInfo {
	const std::string& str;
	float width = 0.f;
};

void TextIndex(const std::string& str, bool multiChannel, ImFont* font);

float TextIndexSize(const std::string& str, bool multiChannel, float fontRatio);

int commentTextCallback(ImGuiInputTextCallbackData* data);

void applyStyleImGuiAndImNodes(ImFont*& defaultFont, ImFont*& smallFont);

bool getAttributeComboItemCallback(void* data, int index, const char** str);

const uint kMaxSlotCount = 12;

inline uint fromInputSlotToLink(Graph::Slot slot){
	return 2 * (slot.node * kMaxSlotCount + slot.slot);
}

inline uint fromOutputSlotToLink(Graph::Slot slot){
	return 2 * (slot.node * kMaxSlotCount + slot.slot) + 1;
}

inline Graph::Slot fromLinkToSlot(uint link, bool& input){
	input = link % 2u == 0u;
	uint baseLink = link / 2u;
	return {baseLink / kMaxSlotCount, (baseLink % kMaxSlotCount)};
}

void applyNodeErrorStyle();

void reverseNodeErrorStyle();
