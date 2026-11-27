#pragma once
#include <string>

struct ImFont;
struct ImFontAtlas;

// Render-thread font state. Prepare must run before ImGui::NewFrame; selecting
// a path during a menu frame only changes config and queues the next refresh.
namespace EspFont {
bool Prepare(const std::string& utf8Path);
void RequestReload();
void AddToAtlas(ImFontAtlas& atlas);
ImFont* Get();
const char* StatusKey();
std::string PickFile(void* owner, bool chinese, const std::string& currentPath);

// The caller limits this scope to ESP. Menu/status drawing keeps its own font.
class Scope {
public:
    Scope();
    ~Scope();
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
private:
    bool pushed_ = false;
};
}
