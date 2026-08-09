#include "opengl_texture.h"
#include "core/render_types.h"
#include "opengl_buffer_objects.h"
#include "utils/image_loader.h"
#include <GL/gl.h>
#include <stdexcept>

namespace ssme::opengl {

static bool hasFlag(ImageUsage usage, ImageUsage flag) {
  return (static_cast<uint32_t>(usage) & static_cast<uint32_t>(flag)) != 0;
}

static GLenum toInternalFormat(Format format) {
  switch (format) {
  case Format::R8G8B8A8_SRGB:
      return GL_SRGB8_ALPHA8;
  case Format::R8G8B8A8_UNORM:
    return GL_RGBA8;
  case Format::R32_SFLOAT:
    return GL_R32F;
  case Format::R32G32_SFLOAT:
    return GL_RG32F;
  case Format::R32G32B32_SFLOAT:
    return GL_RGB32F;
  case Format::R32G32B32A32_SFLOAT:
    return GL_RGBA32F;
  default:
    throw std::runtime_error("Unsupported Format in OpenGLTexture");
  }
}

static GLenum toInternalWrap(Wrap wrap) {
  switch (wrap) {
  case Wrap::REPEAT:
    return GL_REPEAT;
  case Wrap::MIRRORED:
    return GL_MIRRORED_REPEAT;
  case Wrap::CLAMP:
    return GL_CLAMP_TO_EDGE;
  case Wrap::BORDER:
    return GL_CLAMP_TO_BORDER;
  default:
    throw std::runtime_error("Unsupported Wrap in OpenGLTexture");
  }
}

static void toUploadFormat(Format format, GLenum &fmt, GLenum &type) {
  switch (format) {
  case Format::R8G8B8A8_SRGB:
  case Format::R8G8B8A8_UNORM:
    fmt = GL_RGBA;
    type = GL_UNSIGNED_BYTE;
    break;
  case Format::R32_SFLOAT:
    fmt = GL_RED;
    type = GL_FLOAT;
    break;
  case Format::R32G32_SFLOAT:
    fmt = GL_RG;
    type = GL_FLOAT;
    break;
  case Format::R32G32B32_SFLOAT:
    fmt = GL_RGB;
    type = GL_FLOAT;
    break;
  case Format::R32G32B32A32_SFLOAT:
    fmt = GL_RGBA;
    type = GL_FLOAT;
    break;
  default:
    throw std::runtime_error("Unsupported Format in OpenGLTexture");
  }
}

OpenGLTexture::OpenGLTexture() : m_is_owned(false) {}

std::unique_ptr<OpenGLTexture> OpenGLTexture::createDefaultFramebuffer() {
  return std::make_unique<OpenGLTexture>();
}

OpenGLTexture::OpenGLTexture(const TextureDesc &desc)
    : m_format(desc.format), m_is_owned(true) {
  bool is_color_attachment = hasFlag(desc.usage, ImageUsage::COLOR_ATTACHMENT);
  bool is_depth_attachment = hasFlag(desc.usage, ImageUsage::DEPTH_STENCIL);
  bool has_data = !desc.source_path.empty() || !desc.raw_data.empty();

  // --- Resolve size + pixel data ---
  const void *data = nullptr;
  int img_w = 0, img_h = 0, img_ch = 0;
  unsigned char *file_pixels = nullptr;
  if (!desc.source_path.empty()) {
    file_pixels = loadImage(desc.source_path.c_str(), &img_w, &img_h, &img_ch);
    if (!file_pixels)
      throw std::runtime_error("Failed to load texture: " + desc.source_path);
    m_width = img_w;
    m_height = img_h;
    m_format = Format::R8G8B8A8_UNORM; // loader forces RGBA8
    data = file_pixels;
  } else {
    m_width = desc.width;
    m_height = desc.height;
    if (!desc.raw_data.empty())
      data = desc.raw_data.data();
  }

  // --- Storage + upload ---
  GLenum internal_format =
      is_depth_attachment ? GL_DEPTH_COMPONENT32F : toInternalFormat(m_format);
  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  GLenum img_fmt = is_depth_attachment ? GL_DEPTH_COMPONENT : GL_RGBA;
  GLenum img_type = is_depth_attachment ? GL_FLOAT : GL_UNSIGNED_BYTE;
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, (GLsizei)m_width,
               (GLsizei)m_height, 0, img_fmt, img_type, data);

  if (has_data) {
    GLenum upload_format, upload_type;
    toUploadFormat(m_format, upload_format, upload_type);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, (GLsizei)m_width,
                 (GLsizei)m_height, 0, upload_format, upload_type, data);
  }
  if (desc.generate_mips && has_data)
    glGenerateMipmap(GL_TEXTURE_2D);
  GLenum min_filter =
      desc.min_filter == Filter::LINEAR ? GL_LINEAR : GL_NEAREST;
  GLenum mag_filter =
      desc.mag_filter == Filter::LINEAR ? GL_LINEAR : GL_NEAREST;
  GLenum wrap_S = toInternalWrap(desc.wrap_s);
  GLenum wrap_T = toInternalWrap(desc.wrap_t);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_S);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_T);
  glBindTexture(GL_TEXTURE_2D, 0);
  if (file_pixels)
    freeImage(file_pixels);

  // --- FBO only for render targets ---
  if (is_color_attachment || is_depth_attachment) {
    GLenum attachment =
        is_depth_attachment ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0;
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, m_texture,
                           0);
    bool complete =
        glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (!complete)
      throw std::runtime_error("OpenGLTexture: framebuffer incomplete");
  }
}

OpenGLTexture::~OpenGLTexture() {
  if (!m_is_owned)
    return;
  if (m_fbo)
    glDeleteFramebuffers(1, &m_fbo);
  if (m_texture)
    glDeleteTextures(1, &m_texture);
}

} // namespace ssme::opengl
