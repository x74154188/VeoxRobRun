#pragma once
#include <string>

namespace VeoxMenu {

void Init();
void UpdateFonts(); // render thread, before ImGui::NewFrame
void Draw(bool attached, const std::string& statusText);

} // namespace VeoxMenu
