#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

using Color = glm::vec4;

namespace Colors
{
    inline Color rgb(int r, int g, int b, int a = 255)
    {
        return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }
    inline Color withAlpha(Color c, float a) { c.a = a; return c; }
    inline Color lerp(Color a, Color b, float t) { return a + (b - a) * t; }

    const Color White  = Color(1, 1, 1, 1);
    const Color Black  = Color(0, 0, 0, 1);
    const Color Red    = Color(0.85f, 0.12f, 0.10f, 1);
    const Color Green  = Color(0.20f, 0.75f, 0.25f, 1);
    const Color Yellow = Color(0.98f, 0.82f, 0.20f, 1);
    const Color Gray   = Color(0.5f, 0.5f, 0.5f, 1);
    const Color Clear  = Color(0, 0, 0, 0);
}

class Renderer2D
{
public:
    bool init();
    void shutdown();

    void beginFrame(int framebufferW, int framebufferH);
    void endFrame();
    void flush();

    void push();
    void pop();
    void translate(float x, float y);
    void rotate(float radians);
    void rotateDeg(float degrees);
    void scale(float sx, float sy);
    void scale(float s) { scale(s, s); }

    void  setDamageTint(float amount);
    void  setGrayscale(bool on);
    void  setAlpha(float alpha);
    float damageTint() const { return m_state.damage; }

    void rect(float x, float y, float w, float h, Color c);
    void rectCentered(float cx, float cy, float w, float h, Color c);
    void rectGradient(float x, float y, float w, float h, Color bottom, Color top);
    void rectHGradient(float x, float y, float w, float h, Color left, Color right);
    void rectOutline(float x, float y, float w, float h, float thickness, Color c);
    void roundedRect(float x, float y, float w, float h, float radius, Color c);
    void triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, Color col);
    void quad(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, Color col);
    void circle(float cx, float cy, float radius, Color c, int segments = 32);
    void ellipse(float cx, float cy, float rx, float ry, Color c, int segments = 32);
    void ring(float cx, float cy, float innerR, float outerR, Color c, int segments = 32);
    void pie(float cx, float cy, float radius, float a0, float a1, Color c, int segments = 24);
    void arc(float cx, float cy, float radius, float thickness, float a0, float a1, Color c, int segments = 24);
    void line(glm::vec2 a, glm::vec2 b, float thickness, Color c);
    void polygon(const std::vector<glm::vec2>& points, Color c);

    void fullscreen(Color c) { rect(0, 0, (float)m_virtualW, (float)m_virtualH, c); }

    int   viewportX() const { return m_vpX; }
    int   viewportY() const { return m_vpY; }
    int   viewportW() const { return m_vpW; }
    int   viewportH() const { return m_vpH; }
    float viewportScale() const { return m_scale; }

private:
    struct Vertex { float x, y; float r, g, b, a; };
    struct State
    {
        glm::mat4 transform = glm::mat4(1.0f);
        float damage = 0.0f;
        bool  gray = false;
        float alpha = 1.0f;
    };

    glm::vec2 xf(float x, float y) const;
    Color     filter(Color c) const;
    void      pushVertex(glm::vec2 p, Color c);

    GLuint m_program = 0, m_vao = 0, m_vbo = 0;
    GLint  m_locProj = -1;
    std::vector<Vertex> m_vertices;
    std::vector<State>  m_stack;
    State m_state;

    int   m_virtualW = 1280, m_virtualH = 720;
    int   m_vpX = 0, m_vpY = 0, m_vpW = 1280, m_vpH = 720;
    float m_scale = 1.0f;
};
