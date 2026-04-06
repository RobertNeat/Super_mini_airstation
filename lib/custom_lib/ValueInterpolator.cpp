#include "ValueInterpolator.h"

ValueInterpolator::ValueInterpolator(float initial, uint16_t steps)
    : _start(initial), _current(initial), _target(initial)
    , _steps(steps), _step(steps)  // starts "done"
{}

void ValueInterpolator::setTarget(float target) {
    if (target == _target) return;
    _start   = _current;
    _target  = target;
    _step    = 0;
}

void ValueInterpolator::tick() {
    if (_step >= _steps) return;
    _step++;
    // Linear interpolation: t goes 0→1 over _steps ticks
    float t = (float)_step / (float)_steps;
    _current = _start + t * (_target - _start);
}
