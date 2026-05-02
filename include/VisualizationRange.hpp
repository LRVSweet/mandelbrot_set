#pragma once

#include <complex>

/**
 * @brief Defines the region of the complex plane to visualize.
 *
 * Row 0 maps to topLeft's imaginary component (maximum); row index increases
 * downward as the imaginary value decreases. Column 0 maps to topLeft's real
 * component (minimum); column index increases rightward as the real value
 * increases. Pixel aspect ratio must be 1:1 — callers are responsible for
 * deriving topLeft and bottomRight from a center point and zoom level to
 * enforce this invariant.
 */
struct VisualizationRange {
    unsigned int horizontalPoints; ///< Number of pixels along the horizontal axis.
    unsigned int verticalPoints;   ///< Number of pixels along the vertical axis.
    std::complex<double> topLeft;     ///< Top-left corner: maximum imaginary, minimum real.
    std::complex<double> bottomRight; ///< Bottom-right corner: minimum imaginary, maximum real.
};
