#pragma once

#include "segment.h"
#include <vector>
#include <iterator>
#include <cassert>

//https://www.npmjs.com/package/adaptive-bezier-curve?activeTab=readme
//https://github.com/tyt2y3/vaserenderer
//https://github.com/CrushedPixel/Polyline2D?tab=readme-ov-file

class Polyline2D {
public:
	enum class JointStyle {
		MITER,
		BEVEL,
		ROUND
	};

	enum class EndCapStyle {
		BUTT,
		SQUARE,
		ROUND,
		JOINT
	};

	/**
	 * Creates a vector of vertices describing a solid path through the input points.
	 * @param points The points of the path.
	 * @param thickness The path's thickness.
	 * @param jointStyle The path's joint style.
	 * @param endCapStyle The path's end cap style.
	 * @param allowOverlap Whether to allow overlapping vertices.
	 * 					   This yields better results when dealing with paths
	 * 					   whose points have a distance smaller than the thickness,
	 * 					   but may introduce overlapping vertices,
	 * 					   which is undesirable when rendering transparent paths.
	 * @return The vertices describing the path.
	 * @tparam Vector2 The vector type to use for the vertices.
	 *              Must have public non-const float fields "x" and "y".
	 *              Must have a two-args constructor taking x and y values.
	 *              See crushedpixel::Vector2 for a type that satisfies these requirements.
	 * @tparam InputCollection The collection type of the input points.
	 *                         Must contain elements of type Vector2.
	 *                         Must expose size() and operator[] functions.
	 */
	template<typename Vector2, typename InputCollection>
	static std::vector<Vector2> create(const InputCollection &points, float thickness,
	                                JointStyle jointStyle = JointStyle::MITER,
	                                EndCapStyle endCapStyle = EndCapStyle::BUTT,
	                                bool allowOverlap = false) {
		std::vector<Vector2> vertices;
		create(vertices, points, thickness, jointStyle, endCapStyle, allowOverlap);
		return vertices;
	}

	template<typename Vector2>
	static std::vector<Vector2> create(const std::vector<Vector2> &points, float thickness,
	                                JointStyle jointStyle = JointStyle::MITER,
	                                EndCapStyle endCapStyle = EndCapStyle::BUTT,
	                                bool allowOverlap = false) {
		std::vector<Vector2> vertices;
		create<Vector2, std::vector<Vector2>>(vertices, points, thickness, jointStyle, endCapStyle, allowOverlap);
		return vertices;
	}

	template<typename Vector2, typename InputCollection>
	static size_t create(std::vector<Vector2> &vertices, const InputCollection &points, float thickness,
	                     JointStyle jointStyle = JointStyle::MITER,
	                     EndCapStyle endCapStyle = EndCapStyle::BUTT,
	                     bool allowOverlap = false) {
		auto numVerticesBefore = vertices.size();

		create<Vector2, InputCollection>(std::back_inserter(vertices), points, thickness,
		                              jointStyle, endCapStyle, allowOverlap);

		return vertices.size() - numVerticesBefore;
	}

	template<typename Vector2, typename InputCollection, typename OutputIterator>
	static OutputIterator create(OutputIterator vertices, const InputCollection &points, float thickness,
	                             JointStyle jointStyle = JointStyle::MITER,
	                             EndCapStyle endCapStyle = EndCapStyle::BUTT,
	                             bool allowOverlap = false) {
		// operate on half the thickness to make our lives easier
		thickness /= 2;

		// create poly segments from the points
		std::vector<PolySegment<Vector2>> segments;
		for (size_t i = 0; i + 1 < points.size(); i++) {
			auto &point1 = points[i];
			auto &point2 = points[i + 1];

			// to avoid division-by-zero errors,
			// only create a line segment for non-identical points
			if (!Vec2Utils::equal(point1, point2)) {
				segments.emplace_back(LineSegment<Vector2>(point1, point2), thickness);
			}
		}

		if (endCapStyle == EndCapStyle::JOINT) {
			// create a connecting segment from the last to the first point

			auto &point1 = points[points.size() - 1];
			auto &point2 = points[0];

			// to avoid division-by-zero errors,
			// only create a line segment for non-identical points
			if (!Vec2Utils::equal(point1, point2)) {
				segments.emplace_back(LineSegment<Vector2>(point1, point2), thickness);
			}
		}

		if (segments.empty()) {
			// handle the case of insufficient input points
			return vertices;
		}

		Vector2 nextStart1{0, 0};
		Vector2 nextStart2{0, 0};
		Vector2 start1{0, 0};
		Vector2 start2{0, 0};
		Vector2 end1{0, 0};
		Vector2 end2{0, 0};

		// calculate the path's global start and end points
		auto &firstSegment = segments[0];
		auto &lastSegment = segments[segments.size() - 1];

		auto pathStart1 = firstSegment.edge1.a;
		auto pathStart2 = firstSegment.edge2.a;
		auto pathEnd1 = lastSegment.edge1.b;
		auto pathEnd2 = lastSegment.edge2.b;

		// handle different end cap styles
		if (endCapStyle == EndCapStyle::SQUARE) {
			// extend the start/end points by half the thickness
			pathStart1 = Vec2Utils::subtract(pathStart1, Vec2Utils::multiply(firstSegment.edge1.direction(), thickness));
			pathStart2 = Vec2Utils::subtract(pathStart2, Vec2Utils::multiply(firstSegment.edge2.direction(), thickness));
			pathEnd1 = Vec2Utils::add(pathEnd1, Vec2Utils::multiply(lastSegment.edge1.direction(), thickness));
			pathEnd2 = Vec2Utils::add(pathEnd2, Vec2Utils::multiply(lastSegment.edge2.direction(), thickness));

		} else if (endCapStyle == EndCapStyle::ROUND) {
			// draw half circle end caps
			createTriangleFan(vertices, firstSegment.center.a, firstSegment.center.a,
			                  firstSegment.edge1.a, firstSegment.edge2.a, false);
			createTriangleFan(vertices, lastSegment.center.b, lastSegment.center.b,
			                  lastSegment.edge1.b, lastSegment.edge2.b, true);

		} else if (endCapStyle == EndCapStyle::JOINT) {
			// join the last (connecting) segment and the first segment
			createJoint(vertices, lastSegment, firstSegment, jointStyle,
			            pathEnd1, pathEnd2, pathStart1, pathStart2, allowOverlap);
		}

		// generate mesh data for path segments
		for (size_t i = 0; i < segments.size(); i++) {
			auto &segment = segments[i];

			// calculate start
			if (i == 0) {
				// this is the first segment
				start1 = pathStart1;
				start2 = pathStart2;
			}

			if (i + 1 == segments.size()) {
				// this is the last segment
				end1 = pathEnd1;
				end2 = pathEnd2;

			} else {
				createJoint(vertices, segment, segments[i + 1], jointStyle,
				            end1, end2, nextStart1, nextStart2, allowOverlap);
			}

			// emit vertices
			*vertices++ = start1;
			*vertices++ = start2;
			*vertices++ = end1;

			*vertices++ = end1;
			*vertices++ = start2;
			*vertices++ = end2;

			start1 = nextStart1;
			start2 = nextStart2;
		}

		return vertices;
	}

private:
	static constexpr float pi = 3.14159265358979323846f;

	/**
	 * The threshold for mitered joints.
	 * If the joint's angle is smaller than this angle,
	 * the joint will be drawn beveled instead.
	 */
	static constexpr float miterMinAngle = 0.349066; // ~20 degrees

	/**
	 * The minimum angle of a round joint's triangles.
	 */
	static constexpr float roundMinAngle = 0.174533; // ~10 degrees

	template<typename Vector2>
	struct PolySegment {
		PolySegment(const LineSegment<Vector2> &center, float thickness) :
				center(center),
				// calculate the segment's outer edges by offsetting
				// the central line by the normal vector
				// multiplied with the thickness

				// center + center.normal() * thickness
				edge1(center + Vec2Utils::multiply(center.normal(), thickness)),
				edge2(center - Vec2Utils::multiply(center.normal(), thickness)) {}

		LineSegment<Vector2> center, edge1, edge2;
	};

	template<typename Vector2, typename OutputIterator>
	static OutputIterator createJoint(OutputIterator vertices,
	                                  const PolySegment<Vector2> &segment1, const PolySegment<Vector2> &segment2,
	                                  JointStyle jointStyle, Vector2 &end1, Vector2 &end2,
	                                  Vector2 &nextStart1, Vector2 &nextStart2,
	                                  bool allowOverlap) {
		// calculate the angle between the two line segments
		auto dir1 = segment1.center.direction();
		auto dir2 = segment2.center.direction();

		auto angle = Vec2Utils::angle(dir1, dir2);

		// wrap the angle around the 180° mark if it exceeds 90°
		// for minimum angle detection
		auto wrappedAngle = angle;
		if (wrappedAngle > pi / 2) {
			wrappedAngle = pi - wrappedAngle;
		}

		if (jointStyle == JointStyle::MITER && wrappedAngle < miterMinAngle) {
			// the minimum angle for mitered joints wasn't exceeded.
			// to avoid the intersection point being extremely far out,
			// thus producing an enormous joint like a rasta on 4/20,
			// we render the joint beveled instead.
			jointStyle = JointStyle::BEVEL;
		}

		if (jointStyle == JointStyle::MITER) {
			// calculate each edge's intersection point
			// with the next segment's central line
			auto sec1 = LineSegment<Vector2>::intersection(segment1.edge1, segment2.edge1, true);
			auto sec2 = LineSegment<Vector2>::intersection(segment1.edge2, segment2.edge2, true);

			end1 = sec1 ? *sec1 : segment1.edge1.b;
			end2 = sec2 ? *sec2 : segment1.edge2.b;

			nextStart1 = end1;
			nextStart2 = end2;

		} else {
			// joint style is either BEVEL or ROUND

			// find out which are the inner edges for this joint
			auto x1 = dir1.x;
			auto x2 = dir2.x;
			auto y1 = dir1.y;
			auto y2 = dir2.y;

			auto clockwise = x1 * y2 - x2 * y1 < 0;

			const LineSegment<Vector2> *inner1, *inner2, *outer1, *outer2;

			// as the normal vector is rotated counter-clockwise,
			// the first edge lies to the left
			// from the central line's perspective,
			// and the second one to the right.
			if (clockwise) {
				outer1 = &segment1.edge1;
				outer2 = &segment2.edge1;
				inner1 = &segment1.edge2;
				inner2 = &segment2.edge2;
			} else {
				outer1 = &segment1.edge2;
				outer2 = &segment2.edge2;
				inner1 = &segment1.edge1;
				inner2 = &segment2.edge1;
			}

			// calculate the intersection point of the inner edges
			auto innerSecOpt = LineSegment<Vector2>::intersection(*inner1, *inner2, allowOverlap);

			auto innerSec = innerSecOpt
			                ? *innerSecOpt
			                // for parallel lines, simply connect them directly
			                : inner1->b;

			// if there's no inner intersection, flip
			// the next start position for near-180° turns
			Vector2 innerStart;
			if (innerSecOpt) {
				innerStart = innerSec;
			} else if (angle > pi / 2) {
				innerStart = outer1->b;
			} else {
				innerStart = inner1->b;
			}

			if (clockwise) {
				end1 = outer1->b;
				end2 = innerSec;

				nextStart1 = outer2->a;
				nextStart2 = innerStart;

			} else {
				end1 = innerSec;
				end2 = outer1->b;

				nextStart1 = innerStart;
				nextStart2 = outer2->a;
			}

			// connect the intersection points according to the joint style

			if (jointStyle == JointStyle::BEVEL) {
				// simply connect the intersection points
				*vertices++ = outer1->b;
				*vertices++ = outer2->a;
				*vertices++ = innerSec;

			} else if (jointStyle == JointStyle::ROUND) {
				// draw a circle between the ends of the outer edges,
				// centered at the actual point
				// with half the line thickness as the radius
				createTriangleFan(vertices, innerSec, segment1.center.b, outer1->b, outer2->a, clockwise);
			} else {
				assert(false);
			}
		}

		return vertices;
	}

	/**
	 * Creates a partial circle between two points.
	 * The points must be equally far away from the origin.
	 * @param vertices The vector to add vertices to.
	 * @param connectTo The position to connect the triangles to.
	 * @param origin The circle's origin.
	 * @param start The circle's starting point.
	 * @param end The circle's ending point.
	 * @param clockwise Whether the circle's rotation is clockwise.
	 */
	template<typename Vector2, typename OutputIterator>
	static OutputIterator createTriangleFan(OutputIterator vertices, Vector2 connectTo, Vector2 origin,
	                                        Vector2 start, Vector2 end, bool clockwise) {

		auto point1 = Vec2Utils::subtract(start, origin);
		auto point2 = Vec2Utils::subtract(end, origin);

		// calculate the angle between the two points
		auto angle1 = atan2(point1.y, point1.x);
		auto angle2 = atan2(point2.y, point2.x);

		// ensure the outer angle is calculated
		if (clockwise) {
			if (angle2 > angle1) {
				angle2 = angle2 - 2 * pi;
			}
		} else {
			if (angle1 > angle2) {
				angle1 = angle1 - 2 * pi;
			}
		}

		auto jointAngle = angle2 - angle1;

		// calculate the amount of triangles to use for the joint
		auto numTriangles = std::max(1, (int) std::floor(std::abs(jointAngle) / roundMinAngle));

		// calculate the angle of each triangle
		auto triAngle = jointAngle / numTriangles;

		Vector2 startPoint = start;
		Vector2 endPoint;
		for (int t = 0; t < numTriangles; t++) {
			if (t + 1 == numTriangles) {
				// it's the last triangle - ensure it perfectly
				// connects to the next line
				endPoint = end;
			} else {
				auto rot = (t + 1) * triAngle;

				// rotate the original point around the origin
				endPoint.x = std::cos(rot) * point1.x - std::sin(rot) * point1.y;
				endPoint.y = std::sin(rot) * point1.x + std::cos(rot) * point1.y;

				// re-add the rotation origin to the target point
				endPoint = Vec2Utils::add(endPoint, origin);
			}

			// emit the triangle
			*vertices++ = startPoint;
			*vertices++ = endPoint;
			*vertices++ = connectTo;

			startPoint = endPoint;
		}

		return vertices;
	}
};
