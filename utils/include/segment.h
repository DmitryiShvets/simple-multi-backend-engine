#pragma once

#include "vector2.h"
#include <optional>

template <typename Vector2> struct LineSegment {
  LineSegment(const Vector2 &a, const Vector2 &b) : a(a), b(b) {}

  Vector2 a, b;

  /**
   * @return A copy of the line segment, offset by the given vector.
   */
  LineSegment operator+(const Vector2 &toAdd) const {
    return {Vec2Utils::add(a, toAdd), Vec2Utils::add(b, toAdd)};
  }

  /**
   * @return A copy of the line segment, offset by the given vector.
   */
  LineSegment operator-(const Vector2 &toRemove) const {
    return {Vec2Utils::subtract(a, toRemove), Vec2Utils::subtract(b, toRemove)};
  }

  /**
   * @return The line segment's normal vector.
   */
  Vector2 normal() const {
    auto dir = direction();

    // return the direction vector
    // rotated by 90 degrees counter-clockwise
    return {-dir.y, dir.x};
  }

  /**
   * @return The line segment's direction vector.
   */
  Vector2 direction(bool normalized = true) const {
    auto vec = Vec2Utils::subtract(b, a);

    return normalized ? Vec2Utils::normalized(vec) : vec;
  }

  static std::optional<Vector2>
  intersection(const LineSegment &a, const LineSegment &b, bool infiniteLines) {
    // calculate un-normalized direction vectors
    auto r = a.direction(false);
    auto s = b.direction(false);

    auto originDist = Vec2Utils::subtract(b.a, a.a);

    auto uNumerator = Vec2Utils::cross(originDist, r);
    auto denominator = Vec2Utils::cross(r, s);

    if (std::abs(denominator) < 0.0001f) {
      // The lines are parallel
      return std::nullopt;
    }

    // solve the intersection positions
    auto u = uNumerator / denominator;
    auto t = Vec2Utils::cross(originDist, s) / denominator;

    if (!infiniteLines && (t < 0 || t > 1 || u < 0 || u > 1)) {
      // the intersection lies outside of the line segments
      return std::nullopt;
    }

    // calculate the intersection point
    // a.a + r * t;
    return Vec2Utils::add(a.a, Vec2Utils::multiply(r, t));
  }
};
