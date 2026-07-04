#pragma once

namespace toast_message {
struct ModConfig;
}

namespace toast_message::ui {

// Wire the overlay to the live config instance owned by MyMod. Scale sliders
// and content edits are written straight back into it so they persist through
// the normal config save path.
void setConfig(ModConfig *config);

// Visibility is toggled by the mod-menu button. When hidden, draw() early-outs
// and the overlay does not intercept touches.
void setVisible(bool visible);
bool isVisible();
void toggle();

// Called once per frame between ImGui::NewFrame() and ImGui::Render().
void draw();

} // namespace toast_message::ui
