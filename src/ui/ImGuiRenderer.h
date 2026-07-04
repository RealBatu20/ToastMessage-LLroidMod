#pragma once

namespace toast_message::ui {

// Installs the rendering hook and prepares ImGui. The actual ImGui context and
// GL backend are created lazily on the first rendered frame (when a valid GL
// context is guaranteed to be current).
bool initRenderer();

// Removes the render hook and destroys ImGui resources.
void shutdownRenderer();

} // namespace toast_message::ui
