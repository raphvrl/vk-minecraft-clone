#include "sound_manager.hpp"
#include "world/block_registry.hpp"

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
    wld::Block block = wld::BlockRegistry::get().getBlock(blockType);

    std::string groupName = "step." + block.material;

    auto it = m_soundGroups.find(groupName);
    if (it != m_soundGroups.end()) {
        play(it->second, pos, false);
    }
}

void SoundManager::playPlaceBlock(
    const wld::BlockType &blockType, 
    glm::vec3 &pos
)
{
    wld::Block block = wld::BlockRegistry::get().getBlock(blockType);

    std::string groupName = "dig." + block.material;
    auto it = m_soundGroups.find(groupName);
    if (it != m_soundGroups.end()) {
        play(it->second, pos, false);
    }
}

void SoundManager::playBreakBlock(
    const wld::BlockType &blockType, 
    glm::vec3 &pos
)
{
    wld::Block block = wld::BlockRegistry::get().getBlock(blockType);

    std::string groupName = "dig." + block.material;
    auto it = m_soundGroups.find(groupName);
    if (it != m_soundGroups.end()) {
        play(it->second, pos, false);
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

    registerGroup(
        "step.wood",
        {
            "assets/sounds/step/wood1.ogg",
            "assets/sounds/step/wood2.ogg",
            "assets/sounds/step/wood3.ogg",
            "assets/sounds/step/wood4.ogg",
            "assets/sounds/step/wood5.ogg",
            "assets/sounds/step/wood6.ogg",
        }
    );

    registerGroup(
        "dig.grass",
        {
            "assets/sounds/dig/grass1.ogg",
            "assets/sounds/dig/grass2.ogg",
            "assets/sounds/dig/grass3.ogg",
            "assets/sounds/dig/grass4.ogg",
        }
    );

    registerGroup(
        "dig.stone",
        {
            "assets/sounds/dig/stone1.ogg",
            "assets/sounds/dig/stone2.ogg",
            "assets/sounds/dig/stone3.ogg",
            "assets/sounds/dig/stone4.ogg",
        }
    );

    registerGroup(
        "dig.sand",
        {
            "assets/sounds/dig/sand1.ogg",
            "assets/sounds/dig/sand2.ogg",
            "assets/sounds/dig/sand3.ogg",
            "assets/sounds/dig/sand4.ogg",
        }
    );

    registerGroup(
        "dig.gravel",
        {
            "assets/sounds/dig/gravel1.ogg",
            "assets/sounds/dig/gravel2.ogg",
            "assets/sounds/dig/gravel3.ogg",
            "assets/sounds/dig/gravel4.ogg",
        }
    );

    registerGroup(
        "dig.wood",
        {
            "assets/sounds/dig/wood1.ogg",
            "assets/sounds/dig/wood2.ogg",
            "assets/sounds/dig/wood3.ogg",
            "assets/sounds/dig/wood4.ogg",
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