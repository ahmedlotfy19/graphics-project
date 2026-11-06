#include "Renderer2D.h"
#include "../Config.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

const float PI = 3.14159265f;

static const char* vertexShaderSource = R"(
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

static const char* fragmentShaderSource = R"(
#version 330 core
in vec4 vColor;
out vec4 FragColor;
void main()
{
    FragColor = vColor;
}
)";

static GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compile error:\n" << infoLog << std::endl;
    }
    return shader;
}

bool Renderer2D::init()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    m_program = glCreateProgram();
    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    glLinkProgram(m_program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    int success;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        std::cerr << "Shader link error\n";
        return false;
    }
    m_projLoc = glGetUniformLocation(m_program, "u_Proj");

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return true;
}

void Renderer2D::shutdown()
{
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
    glDeleteProgram(m_program);
}

void Renderer2D::beginFrame(int screenW, int screenH)
{
    m_scale = std::min(screenW / (float)VIRTUAL_W, screenH / (float)VIRTUAL_H);
    int w = (int)(VIRTUAL_W * m_scale);
    int h = (int)(VIRTUAL_H * m_scale);
    m_vpX = (screenW - w) / 2;
    m_vpY = (screenH - h) / 2;

    glViewport(0, 0, screenW, screenH);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(m_vpX, m_vpY, w, h);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

    glm::mat4 proj = glm::ortho(0.0f, (float)VIRTUAL_W, 0.0f, (float)VIRTUAL_H);
    glUseProgram(m_program);
    glUniformMatrix4fv(m_projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_vertices.size());
    glBindVertexArray(0);

    m_vertices.clear();
}

void Renderer2D::push()
{
    m_stack.push_back(m_state);
}

void Renderer2D::pop()
{
    if (m_stack.empty()) return;
    m_state = m_stack.back();
    m_stack.pop_back();
}

void Renderer2D::translate(float x, float y)
{
    m_state.transform = glm::translate(m_state.transform, glm::vec3(x, y, 0));
}

void Renderer2D::rotate(float radians)
{
    m_state.transform = glm::rotate(m_state.transform, radians, glm::vec3(0, 0, 1));
}

void Renderer2D::scale(float sx, float sy)
{
    m_state.transform = glm::scale(m_state.transform, glm::vec3(sx, sy, 1));
}

void Renderer2D::setDamageTint(float amount) { m_state.damage = glm::clamp(amount, 0.0f, 1.0f); }
void Renderer2D::setGrayscale(bool on)       { m_state.gray = on; }
void Renderer2D::setAlpha(float alpha)       { m_state.alpha = glm::clamp(alpha, 0.0f, 1.0f); }

Color Renderer2D::applyFilters(Color c) const
{
    if (m_state.damage > 0)
    {
        glm::vec3 red(0.95f, 0.05f, 0.05f);
        glm::vec3 mixed = glm::mix(glm::vec3(c), red, m_state.damage * 0.8f);
        c = Color(mixed, c.a);
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

void Renderer2D::addVertex(float x, float y, Color c)
{
    glm::vec4 p = m_state.transform * glm::vec4(x, y, 0, 1);
    c = applyFilters(c);
    m_vertices.push_back({ p.x, p.y, c.r, c.g, c.b, c.a });
    if (m_vertices.size() >= 60000 && m_vertices.size() % 3 == 0) flush();
}

void Renderer2D::triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, Color col)
{
    addVertex(a.x, a.y, col);
    addVertex(b.x, b.y, col);
    addVertex(c.x, c.y, col);
}

void Renderer2D::rect(float x, float y, float w, float h, Color c)
{
    rectGradient(x, y, w, h, c, c);
}

void Renderer2D::rectGradient(float x, float y, float w, float h, Color bottom, Color top)
{
    addVertex(x, y, bottom);
    addVertex(x + w, y, bottom);
    addVertex(x + w, y + h, top);
    addVertex(x, y, bottom);
    addVertex(x + w, y + h, top);
    addVertex(x, y + h, top);
}

void Renderer2D::rectOutline(float x, float y, float w, float h, float t, Color c)
{
    rect(x, y, w, t, c);
    rect(x, y + h - t, w, t, c);
    rect(x, y, t, h, c);
    rect(x + w - t, y, t, h, c);
}

void Renderer2D::roundedRect(float x, float y, float w, float h, float radius, Color c)
{
    radius = std::min(radius, std::min(w, h) / 2);
    rect(x + radius, y, w - 2 * radius, h, c);
    rect(x, y + radius, radius, h - 2 * radius, c);
    rect(x + w - radius, y + radius, radius, h - 2 * radius, c);
    pie(x + radius, y + radius, radius, PI, 1.5f * PI, c, 6);
    pie(x + w - radius, y + radius, radius, 1.5f * PI, 2 * PI, c, 6);
    pie(x + w - radius, y + h - radius, radius, 0, 0.5f * PI, c, 6);
    pie(x + radius, y + h - radius, radius, 0.5f * PI, PI, c, 6);
}

void Renderer2D::circle(float cx, float cy, float radius, Color c, int segments)
{
    ellipse(cx, cy, radius, radius, c, segments);
}

void Renderer2D::ellipse(float cx, float cy, float rx, float ry, Color c, int segments)
{
    for (int i = 0; i < segments; i++)
    {
        float a0 = 2 * PI * i / segments;
        float a1 = 2 * PI * (i + 1) / segments;
        triangle({ cx, cy },
                 { cx + cos(a0) * rx, cy + sin(a0) * ry },
                 { cx + cos(a1) * rx, cy + sin(a1) * ry }, c);
    }
}

void Renderer2D::pie(float cx, float cy, float radius, float a0, float a1, Color c, int segments)
{
    for (int i = 0; i < segments; i++)
    {
        float t0 = a0 + (a1 - a0) * i / segments;
        float t1 = a0 + (a1 - a0) * (i + 1) / segments;
        triangle({ cx, cy },
                 { cx + cos(t0) * radius, cy + sin(t0) * radius },
                 { cx + cos(t1) * radius, cy + sin(t1) * radius }, c);
    }
}

void Renderer2D::arc(float cx, float cy, float radius, float thickness, float a0, float a1, Color c, int segments)
{
    float inner = radius - thickness / 2;
    float outer = radius + thickness / 2;
    for (int i = 0; i < segments; i++)
    {
        float t0 = a0 + (a1 - a0) * i / segments;
        float t1 = a0 + (a1 - a0) * (i + 1) / segments;
        glm::vec2 p0(cx + cos(t0) * inner, cy + sin(t0) * inner);
        glm::vec2 p1(cx + cos(t0) * outer, cy + sin(t0) * outer);
        glm::vec2 p2(cx + cos(t1) * outer, cy + sin(t1) * outer);
        glm::vec2 p3(cx + cos(t1) * inner, cy + sin(t1) * inner);
        triangle(p0, p1, p2, c);
        triangle(p0, p2, p3, c);
    }
}

void Renderer2D::ring(float cx, float cy, float innerR, float outerR, Color c, int segments)
{
    arc(cx, cy, (innerR + outerR) / 2, outerR - innerR, 0, 2 * PI, c, segments);
}

void Renderer2D::line(glm::vec2 a, glm::vec2 b, float thickness, Color c)
{
    glm::vec2 dir = b - a;
    float len = glm::length(dir);
    if (len == 0) return;
    glm::vec2 side = glm::vec2(-dir.y, dir.x) / len * (thickness / 2);
    triangle(a + side, b + side, b - side, c);
    triangle(a + side, b - side, a - side, c);
}

void Renderer2D::polygon(const std::vector<glm::vec2>& points, Color c)
{
    for (size_t i = 1; i + 1 < points.size(); i++)
        triangle(points[0], points[i], points[i + 1], c);
}

void Renderer2D::fullscreen(Color c)
{
    rect(0, 0, (float)VIRTUAL_W, (float)VIRTUAL_H, c);
}
