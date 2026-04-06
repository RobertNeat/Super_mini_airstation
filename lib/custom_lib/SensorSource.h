#pragma once
#include <Arduino.h>

// ─── Abstract sensor interface ────────────────────────────────────────────────
// Implement this to swap simulated data for real sensor reads.
class SensorSource {
public:
    virtual ~SensorSource() = default;

    // Returns true when a new reading is available (values changed).
    // Call every loop iteration; the implementation decides how often to sample.
    virtual bool update() = 0;

    virtual float temperature() const = 0;  // °C
    virtual float humidity()    const = 0;  // %
    virtual float co2()         const = 0;  // ppm
    virtual float aqi()         const = 0;  // 0–500
    virtual float tvoc()        const = 0;  // ppb
};

// ─── Sine-wave simulation ─────────────────────────────────────────────────────
// Produces a new batch of values every `intervalMs` milliseconds (default 10 s).
class SimulatedSensor : public SensorSource {
public:
    explicit SimulatedSensor(uint32_t intervalMs = 10000);

    // Returns true only when a fresh batch was computed.
    bool  update()          override;

    float temperature()     const override { return _temp;  }
    float humidity()        const override { return _hum;   }
    float co2()             const override { return _co2;   }
    float aqi()             const override { return _aqi;   }
    float tvoc()            const override { return _tvoc;  }

private:
    struct Channel { float base, amp, freq, phase, minVal, maxVal; };
    static float tick(const Channel &ch, float t);

    Channel  _chTemp, _chHum, _chCO2, _chAQI, _chTVOC;
    float    _temp, _hum, _co2, _aqi, _tvoc;
    uint32_t _intervalMs;
    uint32_t _lastUpdate;
};
