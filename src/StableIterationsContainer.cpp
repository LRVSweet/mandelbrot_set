#include "StableIterationsContainer.hpp"

#include <stdexcept>
#include <string>

StableIterationsContainer::StableIterationsContainer(unsigned int width, unsigned int height)
    : _width(width), _height(height), _data(width * height, 0u) {}

unsigned int& StableIterationsContainer::at(unsigned int row, unsigned int col) {
    if (_isOutOfRange(row, col))
        throw std::out_of_range(_outOfRangeMessage(row, col));
    return _data[row * _width + col];
}

unsigned int StableIterationsContainer::at(unsigned int row, unsigned int col) const {
    if (_isOutOfRange(row, col))
        throw std::out_of_range(_outOfRangeMessage(row, col));
    return _data[row * _width + col];
}

bool StableIterationsContainer::_isOutOfRange(unsigned int row, unsigned int col) const {
    return row >= _height || col >= _width;
}

std::string StableIterationsContainer::_outOfRangeMessage(unsigned int row, unsigned int col) const {
    return "StableIterationsContainer::at(" + std::to_string(row) + ", " +
           std::to_string(col) + ") out of range for dimensions " +
           std::to_string(_width) + "x" + std::to_string(_height);
}

unsigned int StableIterationsContainer::getWidth() const {
    return _width;
}

unsigned int StableIterationsContainer::getHeight() const {
    return _height;
}
