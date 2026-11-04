import math
import os
import random
import struct
import wave

RATE = 22050
OUT = os.path.join("assets", "audio")


def write(name, samples):
    os.makedirs(OUT, exist_ok=True)
    with wave.open(os.path.join(OUT, name + ".wav"), "wb") as f:
        f.setnchannels(1)
        f.setsampwidth(2)
        f.setframerate(RATE)
        f.writeframes(b"".join(struct.pack("<h", int(max(-1, min(1, s)) * 30000)) for s in samples))


def tone(freq0, freq1, dur, wave_fn=math.sin, vol=0.5, decay=4.0):
    n = int(RATE * dur)
    out, phase = [], 0.0
    for i in range(n):
        t = i / n
        phase += 2 * math.pi * (freq0 + (freq1 - freq0) * t) / RATE
        out.append(wave_fn(phase) * vol * math.exp(-decay * t))
    return out


def square(p):
    return 1.0 if math.sin(p) >= 0 else -1.0


def noise(dur, vol=0.5, decay=6.0):
    n = int(RATE * dur)
    return [random.uniform(-1, 1) * vol * math.exp(-decay * i / n) for i in range(n)]


def mix(*tracks):
    n = max(len(t) for t in tracks)
    return [sum(t[i] for t in tracks if i < len(t)) for i in range(n)]


def music(notes, beat, bars, bass):
    out = []
    for _ in range(bars):
        for k, f in enumerate(notes):
            lead = tone(f, f, beat, vol=0.18, decay=2.5)
            low = tone(bass[k % len(bass)], bass[k % len(bass)], beat, square, vol=0.06, decay=1.0)
            out += mix(lead, low)
    return out


random.seed(7)
write("click", tone(900, 700, 0.06, square, 0.25, 8))
write("lane_select", tone(500, 800, 0.10, vol=0.4))
write("place", mix(tone(300, 600, 0.18, square, 0.25), noise(0.08, 0.2)))
write("player_attack", mix(noise(0.12, 0.35, 10), tone(1200, 400, 0.12, vol=0.2)))
write("enemy_attack", tone(180, 90, 0.25, square, 0.3, 5))
write("hit", mix(noise(0.10, 0.5, 12), tone(200, 120, 0.10, vol=0.3)))
write("player_death", tone(500, 80, 0.6, square, 0.3, 3))
write("enemy_death", mix(tone(140, 40, 0.7, square, 0.3, 2.5), noise(0.4, 0.15, 4)))
write("coin", tone(988, 988, 0.07, square, 0.2, 3) + tone(1319, 1319, 0.25, square, 0.2, 5))
write("jump", tone(300, 700, 0.15, vol=0.35))
write("win", sum((tone(f, f, 0.18, square, 0.22, 1.5) for f in (523, 659, 784, 1047)), []))
write("lose", sum((tone(f, f * 0.97, 0.35, square, 0.22, 1.2) for f in (392, 349, 311, 262)), []))
write("music_menu", music([220, 262, 247, 196], 0.6, 4, [110, 131, 123, 98]))
write("music_game", music([294, 349, 330, 262, 294, 392, 349, 330], 0.3, 4, [147, 147, 175, 131]))
print("wrote placeholder sounds to", OUT)
