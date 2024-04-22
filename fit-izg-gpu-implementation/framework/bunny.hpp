/*!
 * @file
 * @brief This file contains model of Standford bunny in static arrays.
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#pragma once

#include <cstdint>

#include <student/fwd.hpp>

/**
 * @brief This structure represents points that contains only position and
 * normal.
 */
struct BunnyVertex {
  float position[3];  ///< position of points
  float normal[3];    ///< normal of points
};

/// This variable contains vertices of Standford bunny.
extern struct BunnyVertex const bunnyVertices[1048];

using VertexIndex = uint32_t;///< type of index

/// This variable contains Standford bunny indices.
extern const VertexIndex bunnyIndices[2092][3];

