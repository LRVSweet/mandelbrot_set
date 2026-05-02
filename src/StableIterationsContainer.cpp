#include "StableIterationsContainer.hpp"

StableIterationsContainer::StableIterationsContainer(unsigned int width, unsigned int /*height*/)
    : _width(width), _height(0u), _data() {}

unsigned int& StableIterationsContainer::at(unsigned int /*row*/, unsigned int /*col*/) {
    static unsigned int dummy = 0u;
    return dummy;
}

unsigned int StableIterationsContainer::at(unsigned int /*row*/, unsigned int /*col*/) const {
    return 0u;
}

unsigned int StableIterationsContainer::getWidth() const {
    return _width;
}

unsigned int StableIterationsContainer::getHeight() const {
    return 0u;
}
