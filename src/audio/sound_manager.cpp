#include "sound_manager.hpp"

namespace sfx
{


void SoundManager::init()
{
    m_audioManager.init();
    registerSounds();
}

void SoundManager::destroy()
{
    m_audioManager.destroy();
}

void SoundManager::update()
{
    m_audioManager.update();
}

void SoundManager::playButtonClick()
{
    play("ui.button.click", false);
}

void SoundManager::playFootstep(
    const wld::BlockType &blockType, 
    glm::vec3 &pos
)
{
    switch (blockType) {
        case wld::BlockType::GRASS:
            play(m_soundGroups["step.grass"], pos, false);
            break;
        case wld::BlockType::DIRT:
            play(m_soundGroups["step.gravel"], pos, false);
            break;
        case wld::BlockType::STONE:
            play(m_soundGroups["step.stone"], pos, false);
            break;
        case wld::BlockType::SAND:
            play(m_soundGroups["step.sand"], pos, false);
            break;
        case wld::BlockType::COBBLESTONE:
            play(m_soundGroups["step.stone"], pos, false);
            break;
        default:
            break;
    }
}

void SoundManager::playPlaceBlock(
    const wld::BlockType &blockType, 
    glm::vec3 &pos
)
{
    switch (blockType) {
        case wld::BlockType::GRASS:
            play(m_soundGroups["step.grass"], pos, false);
            break;
        case wld::BlockType::DIRT:
            play(m_soundGroups["step.gravel"], pos, false);
            break;
        case wld::BlockType::STONE:
            play(m_soundGroups["step.stone"], pos, false);
            break;
        case wld::BlockType::SAND:
            play(m_soundGroups["step.sand"], pos, false);
            break;
        case wld::BlockType::COBBLESTONE:
            play(m_soundGroups["step.stone"], pos, false);
            break;
        default:
            break;
    }
}

void SoundManager::playBreakBlock(
    const wld::BlockType &blockType, 
    glm::vec3 &pos
)
{
    switch (blockType) {
        case wld::BlockType::GRASS:
            play(m_soundGroups["step.grass"], pos, false);
            break;
        case wld::BlockType::DIRT:
            play(m_soundGroups["step.gravel"], pos, false);
            break;
        case wld::BlockType::STONE:
            play(m_soundGroups["step.stone"], pos, false);
            break;
        case wld::BlockType::SAND:
            play(m_soundGroups["step.sand"], pos, false);
            break;
        case wld::BlockType::COBBLESTONE:
            play(m_soundGroups["step.stone"], pos, false);
            break;
        default:
            break;
    }
}

void SoundManager::registerSounds()
{
    registerSound(
        "ui.button.click",
        "assets/sounds/ui/click.ogg"
    );

    registerGroup(
        "step.grass",
        {
            "assets/sounds/step/grass1.ogg",
            "assets/sounds/step/grass2.ogg",
            "assets/sounds/step/grass3.ogg",
            "assets/sounds/step/grass4.ogg",
            "assets/sounds/step/grass5.ogg",
            "assets/sounds/step/grass6.ogg",
        }
    );

    registerGroup(
        "step.stone",
        {
            "assets/sounds/step/stone1.ogg",
            "assets/sounds/step/stone2.ogg",
            "assets/sounds/step/stone3.ogg",
            "assets/sounds/step/stone4.ogg",
            "assets/sounds/step/stone5.ogg",
            "assets/sounds/step/stone6.ogg",
        }
    );

    registerGroup(
        "step.sand",
        {
            "assets/sounds/step/sand1.ogg",
            "assets/sounds/step/sand2.ogg",
            "assets/sounds/step/sand3.ogg",
            "assets/sounds/step/sand4.ogg",
            "assets/sounds/step/sand5.ogg",
        }
    );

    registerGroup(
        "step.gravel",
        {
            "assets/sounds/step/gravel1.ogg",
            "assets/sounds/step/gravel2.ogg",
            "assets/sounds/step/gravel3.ogg",
            "assets/sounds/step/gravel4.ogg",
        }
    );
}

void SoundManager::registerSound(
    const std::string &id,
    const std::string &path
)
{
    m_soundMap[id] = path;
    m_audioManager.loadSound(path);
}

void SoundManager::registerGroup(
    const std::string &id,
    const std::vector<fs::path> &paths
)
{
    SoundGroup group;
    group.name = id;
    for (const auto &path : paths) {
        group.sounds.push_back(path.string());
        m_audioManager.loadSound(path);
    }
    m_soundGroups[id] = group;
}

void SoundManager::play(const SoundGroup &group, bool isLooping)
{
    std::uniform_int_distribution<int> dist(0, group.sounds.size() - 1);
    int index = dist(m_rng);
    m_audioManager.play(group.sounds[index], isLooping);
}

void SoundManager::play(const SoundGroup &group, glm::vec3 &pos, bool isLooping)
{
    std::uniform_int_distribution<int> dist(0, group.sounds.size() - 1);
    int index = dist(m_rng);
    m_audioManager.play(group.sounds[index], pos, isLooping);
}

} // namespace sfx