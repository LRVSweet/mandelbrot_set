#pragma once

#include <vector>

/**
 * @brief Container for stable Mandelbrot iteration counts across a pixel grid.
 *
 * Backing store is a flat std::vector<unsigned int> in row-major order.
 * Element (row, col) maps to index row * width + col.
 */
class StableIterationsContainer {
public:
    /**
     * @brief Constructs a zero-initialized container of the given dimensions.
     *
     * @param width   Number of pixels along the horizontal axis.
     * @param height  Number of pixels along the vertical axis.
     */
    StableIterationsContainer(unsigned int width, unsigned int height);

    /**
     * @brief Returns a mutable reference to the iteration count at (row, col).
     * @throws std::out_of_range if row >= height or col >= width.
     */
    unsigned int& at(unsigned int row, unsigned int col);

    /**
     * @brief Returns the iteration count at (row, col).
     * @throws std::out_of_range if row >= height or col >= width.
     */
    unsigned int at(unsigned int row, unsigned int col) const;

    /** @brief Returns the number of pixels along the horizontal axis. */
    unsigned int getWidth() const;

    /** @brief Returns the number of pixels along the vertical axis. */
    unsigned int getHeight() const;

private:
    unsigned int _width;
    unsigned int _height;
    std::vector<unsigned int> _data;
};
