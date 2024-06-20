/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/ui/imgui_dialog.h"

#include "third_party/imgui/imgui.h"
#include "xenia/base/assert.h"
#include "xenia/ui/imgui_drawer.h"

namespace xe {
namespace ui {

ImGuiDialog::ImGuiDialog(ImGuiDrawer* imgui_drawer)
    : imgui_drawer_(imgui_drawer) {
  imgui_drawer_->AddDialog(this);
}

ImGuiDialog::~ImGuiDialog() {
  imgui_drawer_->RemoveDialog(this);
  for (auto fence : waiting_fences_) {
    fence->Signal();
  }
}

void ImGuiDialog::Then(xe::threading::Fence* fence) {
  waiting_fences_.push_back(fence);
}

void ImGuiDialog::Close() { has_close_pending_ = true; }

ImGuiIO& ImGuiDialog::GetIO() { return imgui_drawer()->GetIO(); }

void ImGuiDialog::Draw() {
  // Draw UI.
  OnDraw(GetIO());

  // Check to see if the UI closed itself and needs to be deleted.
  if (has_close_pending_) {
    OnClose();
    delete this;
  }
}

class MessageBoxDialog final : public ImGuiDialog {
 public:
  MessageBoxDialog(ImGuiDrawer* imgui_drawer, std::string title,
                   std::string body)
      : ImGuiDialog(imgui_drawer),
        title_(std::move(title)),
        body_(std::move(body)) {}

  void OnDraw(ImGuiIO& io) override {
    if (!has_opened_) {
      ImGui::OpenPopup(title_.c_str());
      has_opened_ = true;
    }
    if (ImGui::BeginPopupModal(title_.c_str(), nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      char* text = const_cast<char*>(body_.c_str());
      ImGui::InputTextMultiline(
          "##body", text, body_.size(), ImVec2(600, 0),
          ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_ReadOnly);
      if (ImGui::Button("OK")) {
        ImGui::CloseCurrentPopup();
        Close();
      }
      ImGui::EndPopup();

                    ImGui::Begin(
          "Hello, world!", nullptr,
          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse ||
              ImGuiWindowFlags_AlwaysAutoResize);  // Create a window called
                                                   // "Hello, world!" and append
                                                   // into it.

      for (int row = 0; row < 4; row++) {
        ImGui::BeginGroup();
        for (int col = 0; col < 4; col++) {
          ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)(row * 4 + col)),
                            ImVec2(50.0f, 50.0f), true);
          ImGui::Text("Box %d,%d", row, col);
          ImGui::EndChild();
          ImGui::SameLine();
        }
        ImGui::EndGroup();
      }

      ImGui::End();

    } else {
      Close();
    }
  }

 private:
  bool has_opened_ = false;
  std::string title_;
  std::string body_;
};

ImGuiDialog* ImGuiDialog::ShowMessageBox(ImGuiDrawer* imgui_drawer,
                                         std::string title, std::string body) {
  return new MessageBoxDialog(imgui_drawer, std::move(title), std::move(body));
}

}  // namespace ui
}  // namespace xe
