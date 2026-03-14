#pragma once

#include <cmath>

/**
 * A two-dimensional float vector.
 * It exposes the x and y fields
 * as required by the Polyline2D functions.
 */
struct Vector2 {
  Vector2() : Vector2(0, 0) {}

  Vector2(float x, float y) : x(x), y(y) {}

  virtual ~Vector2() = default;

  float x, y;
};

namespace Vec2Utils {

template <typename Vector2>
static bool equal(const Vector2 &a, const Vector2 &b) {
  return a.x == b.x && a.y == b.y;
}

template <typename Vector2>
static Vector2 multiply(const Vector2 &a, const Vector2 &b) {
  return {a.x * b.x, a.y * b.y};
}

template <typename Vector2>
static Vector2 multiply(const Vector2 &vec, float factor) {
  return {vec.x * factor, vec.y * factor};
}

template <typename Vector2>
static Vector2 divide(const Vector2 &vec, float factor) {
  return {vec.x / factor, vec.y / factor};
}

template <typename Vector2>
static Vector2 add(const Vector2 &a, const Vector2 &b) {
  return {a.x + b.x, a.y + b.y};
}

template <typename Vector2>
static Vector2 subtract(const Vector2 &a, const Vector2 &b) {
  return {a.x - b.x, a.y - b.y};
}

template <typename Vector2> static float magnitude(const Vector2 &vec) {
  return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

template <typename Vector2>
static Vector2 withLength(const Vector2 &vec, float len) {
  auto mag = magnitude(vec);
  auto factor = mag / len;
  return divide(vec, factor);
}

template <typename Vector2> static Vector2 normalized(const Vector2 &vec) {
  return withLength(vec, 1);
}

/**
 * Calculates the dot product of two vectors.
 */
template <typename Vector2>
static float dot(const Vector2 &a, const Vector2 &b) {
  return a.x * b.x + a.y * b.y;
}

/**
 * Calculates the cross product of two vectors.
 */
template <typename Vector2>
static float cross(const Vector2 &a, const Vector2 &b) {
  return a.x * b.y - a.y * b.x;
}

/**
 * Calculates the angle between two vectors.
 */
template <typename Vector2>
static float angle(const Vector2 &a, const Vector2 &b) {
  return std::acos(dot(a, b) / (magnitude(a) * magnitude(b)));
}

} // namespace Vec2Utils
