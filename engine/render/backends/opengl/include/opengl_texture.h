#pragma once
#include "core/render_types.h"
#include "core/resource_types.h"
#include <cstdint>
#include <glad/gl.h>
#include <memory>

namespace ssme::opengl {

class OpenGLTexture {
public:
  // Owned render-target texture: creates GL texture + FBO (color or depth)
  // OpenGLTexture(uint32_t width, uint32_t height, Format format,
  //               ImageUsage usage);
  explicit OpenGLTexture(const TextureDesc &desc);
  // Borrowed wrapper around the default framebuffer (FBO 0)
  static std::unique_ptr<OpenGLTexture> createDefaultFramebuffer();
  OpenGLTexture(); // default framebuffer wrapper
  ~OpenGLTexture();

  OpenGLTexture(const OpenGLTexture &) = delete;
  OpenGLTexture &operator=(const OpenGLTexture &) = delete;

  GLuint getTexture() const { return m_texture; }
  GLuint getFbo() const { return m_fbo; }
  uint32_t getWidth() const { return m_width; }
  uint32_t getHeight() const { return m_height; }
  Format getFormat() const { return m_format; }
  bool isDefaultFramebuffer() const { return !m_is_owned; }

private:
  GLuint m_texture = 0;
  GLuint m_fbo = 0;
  uint32_t m_width = 0;
  uint32_t m_height = 0;
  Format m_format = Format::UNDEFINED;
  bool m_is_owned = false;
};

} // namespace ssme::opengl
