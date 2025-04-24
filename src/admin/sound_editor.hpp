#pragma once

#include <nlohmann/json.hpp>
#include <imgui.h>

#include <string>
#include <algorithm>

#include "audio/sound_manager.hpp"

using json = nlohmann::json;

namespace adm
{

class SoundEditor
{

public:
    SoundEditor() = default;
    ~SoundEditor() = default;

    void render();

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }
    void toggleVisible() { m_visible = !m_visible; }

private:
    bool m_visible = false;

    char m_searchBuffer[128] = "";
    std::string m_selectedPath = "";
    sfx::Sound *m_selectedSound = nullptr;

    bool m_showAddSoundPopup = false;
    bool m_showRenameSoundPopup = false;

    void renderSoundList();
    bool renderJsonTree(
        const json &node,
        const std::string &path,
        const std::string &search
    );
    bool willAnyChildBeVisible(
        const json &node,
        const std::string &path,
        const std::string &search
    );
    void renderSoundDetail();
    void renderNoSoundSelected();
    void renderNewSound();
    void renderRenameSound();

    void handleInput();
    void handleAddSound();
    void handleDuplicateSound();
    void handleRenameSound();
    void deleteSelectedSound();

};

} // namespace adm