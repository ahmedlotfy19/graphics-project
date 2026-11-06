#include "Audio.h"
#include <miniaudio.h>

#include <fstream>
#include <iostream>
#include <string>

static ma_engine engine;
static bool  ready = false;
static float vol = 0.7f;
static bool  isMuted = false;

static ma_sound    music;
static bool        musicLoaded = false;
static std::string currentMusic;

static std::string findFile(const std::string& name)
{
    std::string path = "assets/audio/" + name + ".wav";
    if (std::ifstream(path).good()) return path;

    std::cerr << "Missing sound: " << path << std::endl;
    return "";
}

static void applyVolume()
{
    if (ready) ma_engine_set_volume(&engine, isMuted ? 0.0f : vol);
}

namespace Audio
{
    bool init()
    {
        if (ma_engine_init(nullptr, &engine) != MA_SUCCESS)
        {
            std::cerr << "Could not start audio\n";
            return false;
        }
        ready = true;
        applyVolume();
        return true;
    }

    void shutdown()
    {
        if (!ready) return;
        stopMusic();
        ma_engine_uninit(&engine);
        ready = false;
    }

    void play(const char* name)
    {
        if (!ready || isMuted) return;
        std::string path = findFile(name);
        if (!path.empty()) ma_engine_play_sound(&engine, path.c_str(), nullptr);
    }

    void playMusic(const char* name)
    {
        if (!ready || (musicLoaded && currentMusic == name)) return;
        stopMusic();

        std::string path = findFile(name);
        if (path.empty()) return;
        if (ma_sound_init_from_file(&engine, path.c_str(), MA_SOUND_FLAG_STREAM, nullptr, nullptr, &music) != MA_SUCCESS)
            return;

        ma_sound_set_looping(&music, MA_TRUE);
        ma_sound_set_volume(&music, 0.55f);
        ma_sound_start(&music);
        musicLoaded = true;
        currentMusic = name;
    }

    void stopMusic()
    {
        if (!musicLoaded) return;
        ma_sound_stop(&music);
        ma_sound_uninit(&music);
        musicLoaded = false;
        currentMusic = "";
    }

    void volumeUp()
    {
        vol += 0.1f;
        if (vol > 1.0f) vol = 1.0f;
        applyVolume();
    }

    void volumeDown()
    {
        vol -= 0.1f;
        if (vol < 0.0f) vol = 0.0f;
        applyVolume();
    }

    float volume()        { return vol; }
    void  setMuted(bool m) { isMuted = m; applyVolume(); }
    bool  muted()         { return isMuted; }
}
