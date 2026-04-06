#pragma once
#include <stdint.h>

// ─── ValueInterpolator ────────────────────────────────────────────────────────
// Smoothly transitions a float value from its current position toward a target
// over a fixed number of steps.  Call tick() once per display frame;
// call setTarget() whenever a new sensor reading arrives.
//
// steps : how many tick() calls to reach the target (e.g. 20 fps × 5 s = 100)
class ValueInterpolator {
public:
    explicit ValueInterpolator(float initial = 0.0f, uint16_t steps = 40);

    // Set a new destination.  Interpolation restarts from current position.
    void  setTarget(float target);

    // Advance one step toward target.  Call every display tick.
    void  tick();

    float value()  const { return _current; }
    float target() const { return _target;  }
    bool  done()   const { return _step >= _steps; }

private:
    float    _start;
    float    _current;
    float    _target;
    uint16_t _steps;
    uint16_t _step;
};
