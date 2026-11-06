#pragma once
// ============================================================================
//  Renderer2D  -  draws every shape in the game.         Owner: MEMBER 1
//  Coordinates are 1280x720, origin bottom-left, y up.
//  push/pop save the transform and the filters (damage tint, grayscale, alpha).
// ============================================================================
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

    const Color White = Color(1, 1, 1, 1);
}

class Renderer2D
{
public:
    bool init();
    void shutdown();

    void beginFrame(int screenW, int screenH);
    void endFrame();

    void push();
    void pop();
    void translate(float x, float y);
    void rotate(float radians);
    void scale(float sx, float sy);
    void scale(float s) { scale(s, s); }

    void setDamageTint(float amount);
    void setGrayscale(bool on);
    void setAlpha(float alpha);

    void triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, Color col);
    void rect(float x, float y, float w, float h, Color c);
    void rectGradient(float x, float y, float w, float h, Color bottom, Color top);
    void rectOutline(float x, float y, float w, float h, float thickness, Color c);
    void roundedRect(float x, float y, float w, float h, float radius, Color c);
    void circle(float cx, float cy, float radius, Color c, int segments = 32);
    void ellipse(float cx, float cy, float rx, float ry, Color c, int segments = 32);
    void pie(float cx, float cy, float radius, float a0, float a1, Color c, int segments = 24);
    void arc(float cx, float cy, float radius, float thickness, float a0, float a1, Color c, int segments = 24);
    void ring(float cx, float cy, float innerR, float outerR, Color c, int segments = 32);
    void line(glm::vec2 a, glm::vec2 b, float thickness, Color c);
    void polygon(const std::vector<glm::vec2>& points, Color c);
    void fullscreen(Color c);

    int   viewportX() const { return m_vpX; }
    int   viewportY() const { return m_vpY; }
    float viewportScale() const { return m_scale; }

private:
    struct Vertex { float x, y, r, g, b, a; };
    struct State
    {
        glm::mat4 transform = glm::mat4(1.0f);
        float damage = 0.0f;
        bool  gray = false;
        float alpha = 1.0f;
    };

    void  flush();
    void  addVertex(float x, float y, Color c);
    Color applyFilters(Color c) const;

    GLuint m_program = 0, m_vao = 0, m_vbo = 0;
    GLint  m_projLoc = -1;
    std::vector<Vertex> m_vertices;
    std::vector<State>  m_stack;
    State m_state;

    int   m_vpX = 0, m_vpY = 0;
    float m_scale = 1.0f;
};
