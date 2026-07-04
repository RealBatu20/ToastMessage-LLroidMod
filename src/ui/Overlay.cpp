#include "ui/Overlay.h"

#include <atomic>
#include <cctype>
#include <string>

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_virtual_keyboard.h"

#include "mod/Config.h"
#include "toast/ToastBridge.h"
#include "toast/ToastTypes.h"

namespace toast_message::ui {

namespace {

ModConfig *gConfig = nullptr;
std::atomic<bool> gVisible{false};

// Which text field the virtual keyboard currently edits.
enum class EditTarget { Title, Subtitle, MessageId, Link };
EditTarget gEditTarget = EditTarget::Title;

bool gShift = false;
float gLastAppliedUiScale = -1.0f;

std::string *targetBuffer() {
    if (!gConfig)
        return nullptr;
    switch (gEditTarget) {
    case EditTarget::Title:
        return &gConfig->title;
    case EditTarget::Subtitle:
        return &gConfig->subtitle;
    case EditTarget::MessageId:
        return &gConfig->messageId;
    case EditTarget::Link:
        return &gConfig->link;
    }
    return &gConfig->title;
}

// Translate an ImGuiKey returned by ImGui::VirtualKeyboard() into an edit
// action on the active buffer. Returns nothing; mutates the buffer in place.
void applyVirtualKey(ImGuiKey key) {
    std::string *buf = targetBuffer();
    if (!buf || key == ImGuiKey_COUNT || key == ImGuiKey_None)
        return;

    if (key == ImGuiKey_Backspace) {
        if (!buf->empty())
            buf->pop_back();
        return;
    }
    if (key == ImGuiKey_Space) {
        buf->push_back(' ');
        return;
    }
    if (key == ImGuiKey_LeftShift || key == ImGuiKey_RightShift || key == ImGuiKey_CapsLock) {
        gShift = !gShift;
        return;
    }
    if (key == ImGuiKey_Enter || key == ImGuiKey_KeypadEnter)
        return; // toast lines are single-line

    // Letters.
    if (key >= ImGuiKey_A && key <= ImGuiKey_Z) {
        char c = static_cast<char>('a' + (key - ImGuiKey_A));
        if (gShift)
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        buf->push_back(c);
        return;
    }
    // Top-row digits.
    if (key >= ImGuiKey_0 && key <= ImGuiKey_9) {
        static const char *shifted = ")!@#$%^&*(";
        int d = key - ImGuiKey_0;
        buf->push_back(gShift ? shifted[d] : static_cast<char>('0' + d));
        return;
    }
    // Keypad digits.
    if (key >= ImGuiKey_Keypad0 && key <= ImGuiKey_Keypad9) {
        buf->push_back(static_cast<char>('0' + (key - ImGuiKey_Keypad0)));
        return;
    }
    // Common punctuation.
    switch (key) {
    case ImGuiKey_Period:
    case ImGuiKey_KeypadDecimal:
        buf->push_back('.');
        break;
    case ImGuiKey_Comma:
        buf->push_back(gShift ? '<' : ',');
        break;
    case ImGuiKey_Minus:
    case ImGuiKey_KeypadSubtract:
        buf->push_back(gShift ? '_' : '-');
        break;
    case ImGuiKey_Equal:
        buf->push_back(gShift ? '+' : '=');
        break;
    case ImGuiKey_Slash:
    case ImGuiKey_KeypadDivide:
        buf->push_back(gShift ? '?' : '/');
        break;
    case ImGuiKey_Semicolon:
        buf->push_back(gShift ? ':' : ';');
        break;
    case ImGuiKey_Apostrophe:
        buf->push_back(gShift ? '"' : '\'');
        break;
    default:
        break;
    }
}

const char *editTargetName(EditTarget t) {
    switch (t) {
    case EditTarget::Title:
        return "Title";
    case EditTarget::Subtitle:
        return "Subtitle";
    case EditTarget::MessageId:
        return "Message Id";
    case EditTarget::Link:
        return "Link";
    }
    return "Title";
}

} // namespace

void setConfig(ModConfig *config) { gConfig = config; }
void setVisible(bool visible) { gVisible.store(visible); }
bool isVisible() { return gVisible.load(); }
void toggle() { gVisible.store(!gVisible.load()); }

void draw() {
    if (!gVisible.load() || !gConfig)
        return;

    ImGuiIO &io = ImGui::GetIO();

    // Live UI scale. FontGlobalScale is applied every frame (cheap). Style
    // sizes are only rescaled when the slider actually changes, because
    // ScaleAllSizes is cumulative.
    io.FontGlobalScale = gConfig->fontScale;
    if (gLastAppliedUiScale != gConfig->uiScale) {
        ImGuiStyle &style = ImGui::GetStyle();
        // Reset to a known base then scale, so repeated changes stay stable.
        static ImGuiStyle baseStyle = style;
        style = baseStyle;
        style.ScaleAllSizes(gConfig->uiScale);
        gLastAppliedUiScale = gConfig->uiScale;
    }

    ImGui::SetNextWindowSize(ImVec2(680, 620), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Toast Message", nullptr, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    // ---- Toast type dropdown ------------------------------------------
    std::size_t typeIndex = toastTypeIndexById(gConfig->toastType);
    const ToastTypeInfo &current = toastTypeInfoByIndex(typeIndex);
    if (ImGui::BeginCombo("Toast Type", std::string(current.label).c_str())) {
        for (std::size_t i = 0; i < kToastTypes.size(); ++i) {
            bool selected = (i == typeIndex);
            if (ImGui::Selectable(std::string(kToastTypes[i].label).c_str(), selected)) {
                typeIndex = i;
                gConfig->toastType = std::string(kToastTypes[i].id);
            }
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    const ToastMessageType selectedType = toastTypeInfoByIndex(typeIndex).type;
    const bool extended = typeUsesButtonAction(selectedType);

    ImGui::Separator();

    // ---- Text fields (physical keyboard) ------------------------------
    ImGui::InputText("Title", &gConfig->title);
    ImGui::InputText("Subtitle", &gConfig->subtitle);
    if (extended) {
        ImGui::InputText("Message Id", &gConfig->messageId);
        ImGui::InputText("Link", &gConfig->link);
        ImGui::Checkbox("Silent", &gConfig->silent);
        ImGui::InputInt("Button Action", &gConfig->buttonActionType);
    }

    ImGui::Separator();

    // ---- Live scale sliders -------------------------------------------
    ImGui::SliderFloat("UI Scale", &gConfig->uiScale, 0.75f, 4.0f, "%.2f");
    ImGui::SliderFloat("Font Scale", &gConfig->fontScale, 0.75f, 4.0f, "%.2f");

    ImGui::Separator();

    // ---- Send ----------------------------------------------------------
    ToastBridge &bridge = ToastBridge::instance();
    if (!bridge.hasManager()) {
        ImGui::TextWrapped("Waiting to capture the game's ToastManager. Join a "
                           "world (or trigger any toast once) to enable sending.");
    }
    if (ImGui::Button("Send Toast", ImVec2(220, 0))) {
        ToastRequest req;
        req.type = selectedType;
        req.title = gConfig->title;
        req.subtitle = gConfig->subtitle;
        req.messageId = gConfig->messageId;
        req.isSilent = gConfig->silent;
        req.link = gConfig->link;
        req.buttonActionType = gConfig->buttonActionType;
        bridge.push(req);
    }

    ImGui::Separator();

    // ---- On-screen (virtual) keyboard ---------------------------------
    ImGui::Text("On-screen keyboard target:");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##vk_target", editTargetName(gEditTarget))) {
        const EditTarget targets[] = {EditTarget::Title, EditTarget::Subtitle,
                                      EditTarget::MessageId, EditTarget::Link};
        for (EditTarget t : targets) {
            if (t == EditTarget::MessageId || t == EditTarget::Link) {
                if (!extended)
                    continue;
            }
            if (ImGui::Selectable(editTargetName(t), t == gEditTarget))
                gEditTarget = t;
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Shift", &gShift);

    // Only the base + arrow blocks keep the keyboard usable on a phone screen.
    const ImGuiKey pressed = ImGui::VirtualKeyboard(
        ImGui::VirtualKeyboardFlags_ShowBaseBlock | ImGui::VirtualKeyboardFlags_ShowArrowBlock,
        ImGui::KLL_QWERTY, ImGui::KPL_ISO);
    applyVirtualKey(pressed);

    ImGui::End();
}

} // namespace toast_message::ui
