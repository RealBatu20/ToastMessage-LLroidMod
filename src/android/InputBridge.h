#pragma once

namespace toast_message::input {

// Registers a touch callback with the preloader input bridge and translates
// single-finger touches into an ImGui mouse cursor. Latest state is held in
// atomics and consumed by the renderer each frame (thread-safe, lock-free),
// avoiding cross-thread mutation of ImGui's event queue.
bool init();
void shutdown();

// Latest pointer state, read by the renderer before ImGui::NewFrame().
void latestPointer(float &x, float &y, bool &down);

} // namespace toast_message::input
