#include "Audio.h"
#include <miniaudio.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <set>
#include <string>

namespace
{
    ma_engine g_engine;
    bool      g_ready = false;
    float     g_volume = 0.7f;
    bool      g_muted = false;

    ma_sound    g_music;
    bool        g_musicLoaded = false;
    std::string g_musicName;

    std::set<std::string> g_warned;

    bool fileExists(const std::string& path)
    {
        FILE* f = nullptr;
#ifdef _MSC_VER
        fopen_s(&f, path.c_str(), "rb");
#else
        f = std::fopen(path.c_str(), "rb");
#endif
        if (!f) return false;
        std::fclose(f);
        return true;
    }

    // Finds assets/audio/<name>.(wav|mp3|flac). Empty string if none.
    std::string resolve(const std::string& name)
    {
        static const char* exts[] = { ".wav", ".mp3", ".flac" };
        for (const char* ext : exts)
        {
            std::string p = "assets/audio/" + name + ext;
            if (fileExists(p)) return p;
        }
        if (g_warned.insert(name).second)
            std::cerr << "[Audio] missing sound: assets/audio/" << name << ".wav (skipped)\n";
        return "";
    }

    void applyVolume()
    {
        if (g_ready) ma_engine_set_volume(&g_engine, g_muted ? 0.0f : g_volume);
    }
}

namespace Audio
{
    bool init()
    {
        if (ma_engine_init(nullptr, &g_engine) != MA_SUCCESS)
        {
            std::cerr << "[Audio] could not start audio device - running silent\n";
            return false;
        }
        g_ready = true;
        applyVolume();
        return true;
    }

    void shutdown()
    {
        if (!g_ready) return;
        stopMusic();
        ma_engine_uninit(&g_engine);
        g_ready = false;
    }

    void play(const char* sfxName)
    {
        if (!g_ready || g_muted) return;
        std::string path = resolve(sfxName);
        if (path.empty()) return;
        ma_engine_play_sound(&g_engine, path.c_str(), nullptr);   // fire and forget
    }

    void playMusic(const char* musicName)
    {
        if (!g_ready) return;
        if (g_musicLoaded && g_musicName == musicName) return;
        stopMusic();
        std::string path = resolve(musicName);
        if (path.empty()) return;
        if (ma_sound_init_from_file(&g_engine, path.c_str(), MA_SOUND_FLAG_STREAM,
                                    nullptr, nullptr, &g_music) != MA_SUCCESS)
            return;
        ma_sound_set_looping(&g_music, MA_TRUE);
        ma_sound_set_volume(&g_music, 0.55f);   // keep music under the effects
        ma_sound_start(&g_music);
        g_musicLoaded = true;
        g_musicName = musicName;
    }

    void stopMusic()
    {
        if (!g_musicLoaded) return;
        ma_sound_stop(&g_music);
        ma_sound_uninit(&g_music);
        g_musicLoaded = false;
        g_musicName.clear();
    }

    void  setVolume(float v) { g_volume = std::clamp(v, 0.0f, 1.0f); applyVolume(); }
    float volume()           { return g_volume; }
    void  volumeUp()         { setVolume(std::round((g_volume + 0.1f) * 10.0f) / 10.0f); }
    void  volumeDown()       { setVolume(std::round((g_volume - 0.1f) * 10.0f) / 10.0f); }
    void  setMuted(bool m)   { g_muted = m; applyVolume(); }
    bool  muted()            { return g_muted; }
}
