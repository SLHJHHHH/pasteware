#pragma once
#include <windows.h>
#include <gl/GL.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

#include "ImGui/imgui.h"
#include "Soil/stb_image_aug.h"

namespace render {
    struct Glyph {
        int unicode{};
        float advance{1.0f};
        ImVec2 uv0{};
        ImVec2 uv1{};
    };

    class FontAtlas {
    public:
        GLuint texture;
        int atlas_width{0};
        int atlas_height{0};
        std::unordered_map<int, Glyph> glyphs;


        static double parse_val(const std::string& str, const std::string& key) {
            const auto pos = str.find("\"" + key + "\":");
            if (pos == std::string::npos) return 0.0;
            const auto start = pos + key.length() + 3;
            return std::strtod(&str[start], nullptr);
        }

        static GLuint create_opengl_texture(const unsigned char* pixels, int width, int height) {
            GLuint tex = 0;
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            return tex;
        }

        bool load_from_memory(const unsigned char* png_data, size_t png_size, std::string_view json_view) {
            int channels = 0;
            unsigned char* pixels = stbi_load_from_memory(png_data, static_cast<int>(png_size), &atlas_width, &atlas_height, &channels, 4);
            if (!pixels) return false;

            std::vector<unsigned char> rgba_buffer(atlas_width * atlas_height * 4, 0);

            for (int y = 0; y < atlas_height; ++y) {
                for (int x = 0; x < atlas_width; ++x) {
                    const int src_idx = (y * atlas_width + x) * 4;
                    const unsigned char r = pixels[src_idx + 0];
                    const unsigned char g = pixels[src_idx + 1];
                    const unsigned char b = pixels[src_idx + 2];
                    const int med = (std::max)((std::min)(r, g), (std::min)((std::max)(r, g), b));
                    const float dist = (static_cast<float>(med) - 128.0f) / 6.0f;
                    const float alpha_f = std::clamp(dist + 0.5f, 0.0f, 1.0f);
                    const unsigned char alpha = static_cast<unsigned char>(alpha_f * 255.0f);
                    const int dst_idx = (y * atlas_width + x) * 4;
                    rgba_buffer[dst_idx + 0] = 255;
                    rgba_buffer[dst_idx + 1] = 255;
                    rgba_buffer[dst_idx + 2] = 255;
                    rgba_buffer[dst_idx + 3] = alpha;
                }
            }

            stbi_image_free(pixels);

            texture = create_opengl_texture(rgba_buffer.data(), atlas_width, atlas_height);
            if (!texture) return false;

            const std::string json(json_view);
            const float inv_w = 1.0f / static_cast<float>(atlas_width);
            const float inv_h = 1.0f / static_cast<float>(atlas_height);

            std::size_t offset = 0;
            while ((offset = json.find("{\"unicode\":", offset)) != std::string::npos) {
                const auto end_obj = json.find("}", offset);
                if (end_obj == std::string::npos) break;
                const std::string obj = json.substr(offset, end_obj - offset + 1);

                Glyph g{};
                g.unicode = static_cast<int>(parse_val(obj, "unicode"));
                g.advance = static_cast<float>(parse_val(obj, "advance"));
                if (g.advance == 0.0f) g.advance = 1.0f;

                const auto ab_pos = json.find("\"atlasBounds\":{", offset);
                if (ab_pos != std::string::npos && ab_pos < json.find("{\"unicode\":", offset + 1)) {
                    const auto ab_end = json.find("}", ab_pos);
                    const std::string ab = json.substr(ab_pos, ab_end - ab_pos + 1);

                    const float left = static_cast<float>(parse_val(ab, "left"));
                    const float bottom = static_cast<float>(parse_val(ab, "bottom"));
                    const float right = static_cast<float>(parse_val(ab, "right"));
                    const float top = static_cast<float>(parse_val(ab, "top"));

                    // UV для имгуи опенгл
                    g.uv0 = ImVec2(left * inv_w, 1.0f - (top * inv_h));
                    g.uv1 = ImVec2(right * inv_w, 1.0f - (bottom * inv_h));
                }

                glyphs[g.unicode] = g;
                offset = end_obj + 1;
            }

            return true;
        }

        void shutdown() {
            if (texture) {
                glDeleteTextures(1, &texture);
                texture = 0;
            }
            glyphs.clear();
        }

        void draw_glyph(ImDrawList* draw_list, int unicode, const ImVec2& pos, float size, ImU32 color) {
            const auto it = glyphs.find(unicode);
            if (it == glyphs.end() || !texture) return;

            const auto& g = it->second;
            draw_list->AddImage(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(texture)),
                pos, ImVec2(pos.x + size, pos.y + size), g.uv0, g.uv1, color);

        }
        void draw_icon(ImDrawList* draw_list, char icon_char, const ImVec2& pos, float size, ImU32 color)
        {
            draw_glyph(draw_list, static_cast<int>(icon_char), pos, size, color);
        }
    };
    extern FontAtlas font_icons;
}