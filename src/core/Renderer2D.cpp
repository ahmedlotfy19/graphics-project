#include "Renderer2D.h"
#include "../Config.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace
{
    const char* kVertexSrc = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;
uniform mat4 u_Proj;
out vec4 vColor;
void main()
{
    gl_Position = u_Proj * vec4(aPos, 0.0, 1.0);
    vColor = aColor;
}
)";

    const char* kFragmentSrc = R"(
#version 330 core
in vec4 vColor;
out vec4 FragColor;
void main()
{
    FragColor = vColor;
}
)";

    GLuint compile(GLenum type, const char* src)
    {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        int ok = 0;
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok)
        {
            char log[1024];
            glGetShaderInfoLog(s, sizeof(log), nullptr, log);
            std::cerr << "Shader compile error:\n" << log << std::endl;
        }
        return s;
    }

    const size_t kMaxVertices = 60000;
    const float  kPi = 3.14159265358979f;
}

bool Renderer2D::init()
{
    m_virtualW = VIRTUAL_W;
    m_virtualH = VIRTUAL_H;

    GLuint vs = compile(GL_VERTEX_SHADER, kVertexSrc);
    GLuint fs = compile(GL_FRAGMENT_SHADER, kFragmentSrc);
    m_program = glCreateProgram();
    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    glLinkProgram(m_program);
    int ok = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!ok)
    {
        char log[1024];
        glGetProgramInfoLog(m_program, sizeof(log), nullptr, log);
        std::cerr << "Shader link error:\n" << log << std::endl;
        return false;
    }
    m_locProj = glGetUniformLocation(m_program, "u_Proj");

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, kMaxVertices * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    m_vertices.reserve(kMaxVertices);
    return true;
}

void Renderer2D::shutdown()
{
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_program) glDeleteProgram(m_program);
    m_vbo = m_vao = m_program = 0;
}

void Renderer2D::beginFrame(int fbW, int fbH)
{
    float sx = fbW / (float)m_virtualW;
    float sy = fbH / (float)m_virtualH;
    m_scale = std::max(0.0001f, std::min(sx, sy));
    m_vpW = (int)(m_virtualW * m_scale);
    m_vpH = (int)(m_virtualH * m_scale);
    m_vpX = (fbW - m_vpW) / 2;
    m_vpY = (fbH - m_vpH) / 2;

    glViewport(0, 0, fbW, fbH);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(m_vpX, m_vpY, m_vpW, m_vpH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    m_stack.clear();
    m_state = State();
    m_vertices.clear();
}

void Renderer2D::endFrame()
{
    flush();
}

void Renderer2D::flush()
{
    if (m_vertices.empty()) return;

    glm::mat4 proj = glm::ortho(0.0f, (float)m_virtualW, 0.0f, (float)m_virtualH, -1.0f, 1.0f);
    glUseProgram(m_program);
    glUniformMatrix4fv(m_locProj, 1, GL_FALSE, glm::value_ptr(proj));

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, kMaxVertices * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertices.size() * sizeof(Vertex), m_vertices.data());
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_vertices.size());
    glBindVertexArray(0);

    m_vertices.clear();
}

void Renderer2D::push() { m_stack.push_back(m_state); }

void Renderer2D::pop()
{
    if (m_stack.empty()) return;
    m_state = m_stack.back();
    m_stack.pop_back();
}

void Renderer2D::translate(float x, float y)
{
    m_state.transform = glm::translate(m_state.transform, glm::vec3(x, y, 0.0f));
}

void Renderer2D::rotate(float radians)
{
    m_state.transform = glm::rotate(m_state.transform, radians, glm::vec3(0, 0, 1));
}

void Renderer2D::rotateDeg(float degrees) { rotate(glm::radians(degrees)); }

void Renderer2D::scale(float sx, float sy)
{
    m_state.transform = glm::scale(m_state.transform, glm::vec3(sx, sy, 1.0f));
}

void Renderer2D::setDamageTint(float amount) { m_state.damage = glm::clamp(amount, 0.0f, 1.0f); }
void Renderer2D::setGrayscale(bool on)       { m_state.gray = on; }
void Renderer2D::setAlpha(float alpha)       { m_state.alpha = glm::clamp(alpha, 0.0f, 1.0f); }

glm::vec2 Renderer2D::xf(float x, float y) const
{
    glm::vec4 p = m_state.transform * glm::vec4(x, y, 0.0f, 1.0f);
    return glm::vec2(p.x, p.y);
}

Color Renderer2D::filter(Color c) const
{
    if (m_state.damage > 0.0f)
    {
        const glm::vec3 hurt(0.95f, 0.05f, 0.05f);
        glm::vec3 rgb = glm::mix(glm::vec3(c), hurt, m_state.damage * 0.8f);
        c = Color(rgb, c.a);
    }
    if (m_state.gray)
    {
        float l = 0.299f * c.r + 0.587f * c.g + 0.114f * c.b;
        l = 0.15f + l * 0.6f;
        c = Color(l, l, l, c.a);
    }
    c.a *= m_state.alpha;
    return c;
}

void Renderer2D::pushVertex(glm::vec2 p, Color c)
{
    m_vertices.push_back({ p.x, p.y, c.r, c.g, c.b, c.a });
}

void Renderer2D::triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, Color col)
{
    if (m_vertices.size() + 3 > kMaxVertices) flush();
    Color f = filter(col);
    pushVertex(xf(a.x, a.y), f);
    pushVertex(xf(b.x, b.y), f);
    pushVertex(xf(c.x, c.y), f);
}

void Renderer2D::quad(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, Color col)
{
    triangle(a, b, c, col);
    triangle(a, c, d, col);
}

void Renderer2D::rect(float x, float y, float w, float h, Color c)
{
    quad({ x, y }, { x + w, y }, { x + w, y + h }, { x, y + h }, c);
}

void Renderer2D::rectCentered(float cx, float cy, float w, float h, Color c)
{
    rect(cx - w * 0.5f, cy - h * 0.5f, w, h, c);
}

void Renderer2D::rectGradient(float x, float y, float w, float h, Color bottom, Color top)
{
    if (m_vertices.size() + 6 > kMaxVertices) flush();
    Color b = filter(bottom), t = filter(top);
    glm::vec2 p0 = xf(x, y), p1 = xf(x + w, y), p2 = xf(x + w, y + h), p3 = xf(x, y + h);
    pushVertex(p0, b); pushVertex(p1, b); pushVertex(p2, t);
    pushVertex(p0, b); pushVertex(p2, t); pushVertex(p3, t);
}

void Renderer2D::rectHGradient(float x, float y, float w, float h, Color left, Color right)
{
    if (m_vertices.size() + 6 > kMaxVertices) flush();
    Color l = filter(left), r = filter(right);
    glm::vec2 p0 = xf(x, y), p1 = xf(x + w, y), p2 = xf(x + w, y + h), p3 = xf(x, y + h);
    pushVertex(p0, l); pushVertex(p1, r); pushVertex(p2, r);
    pushVertex(p0, l); pushVertex(p2, r); pushVertex(p3, l);
}

void Renderer2D::rectOutline(float x, float y, float w, float h, float t, Color c)
{
    rect(x, y, w, t, c);
    rect(x, y + h - t, w, t, c);
    rect(x, y + t, t, h - 2 * t, c);
    rect(x + w - t, y + t, t, h - 2 * t, c);
}

void Renderer2D::roundedRect(float x, float y, float w, float h, float radius, Color c)
{
    radius = std::min(radius, std::min(w, h) * 0.5f);
    rect(x + radius, y, w - 2 * radius, h, c);
    rect(x, y + radius, radius, h - 2 * radius, c);
    rect(x + w - radius, y + radius, radius, h - 2 * radius, c);
    pie(x + radius,     y + radius,     radius, kPi,        1.5f * kPi, c, 8);
    pie(x + w - radius, y + radius,     radius, 1.5f * kPi, 2.0f * kPi, c, 8);
    pie(x + w - radius, y + h - radius, radius, 0.0f,       0.5f * kPi, c, 8);
    pie(x + radius,     y + h - radius, radius, 0.5f * kPi, kPi,        c, 8);
}

void Renderer2D::ellipse(float cx, float cy, float rx, float ry, Color c, int segments)
{
    segments = std::max(3, segments);
    for (int i = 0; i < segments; ++i)
    {
        float a0 = 2 * kPi * i / segments;
        float a1 = 2 * kPi * (i + 1) / segments;
        triangle({ cx, cy },
                 { cx + std::cos(a0) * rx, cy + std::sin(a0) * ry },
                 { cx + std::cos(a1) * rx, cy + std::sin(a1) * ry }, c);
    }
}

void Renderer2D::circle(float cx, float cy, float radius, Color c, int segments)
{
    ellipse(cx, cy, radius, radius, c, segments);
}

void Renderer2D::ring(float cx, float cy, float innerR, float outerR, Color c, int segments)
{
    arc(cx, cy, (innerR + outerR) * 0.5f, outerR - innerR, 0.0f, 2 * kPi, c, segments);
}

void Renderer2D::pie(float cx, float cy, float radius, float a0, float a1, Color c, int segments)
{
    segments = std::max(1, segments);
    for (int i = 0; i < segments; ++i)
    {
        float t0 = a0 + (a1 - a0) * i / segments;
        float t1 = a0 + (a1 - a0) * (i + 1) / segments;
        triangle({ cx, cy },
                 { cx + std::cos(t0) * radius, cy + std::sin(t0) * radius },
                 { cx + std::cos(t1) * radius, cy + std::sin(t1) * radius }, c);
    }
}

void Renderer2D::arc(float cx, float cy, float radius, float thickness, float a0, float a1, Color c, int segments)
{
    segments = std::max(1, segments);
    float ri = radius - thickness * 0.5f, ro = radius + thickness * 0.5f;
    for (int i = 0; i < segments; ++i)
    {
        float t0 = a0 + (a1 - a0) * i / segments;
        float t1 = a0 + (a1 - a0) * (i + 1) / segments;
        glm::vec2 d0(std::cos(t0), std::sin(t0)), d1(std::cos(t1), std::sin(t1));
        glm::vec2 ctr(cx, cy);
        quad(ctr + d0 * ri, ctr + d0 * ro, ctr + d1 * ro, ctr + d1 * ri, c);
    }
}

void Renderer2D::line(glm::vec2 a, glm::vec2 b, float thickness, Color c)
{
    glm::vec2 d = b - a;
    float len = glm::length(d);
    if (len < 0.0001f) return;
    glm::vec2 n = glm::vec2(-d.y, d.x) / len * (thickness * 0.5f);
    quad(a + n, b + n, b - n, a - n, c);
}

void Renderer2D::polygon(const std::vector<glm::vec2>& pts, Color c)
{
    for (size_t i = 1; i + 1 < pts.size(); ++i)
        triangle(pts[0], pts[i], pts[i + 1], c);
}
