#pragma once

#include <cstddef>
#include <cstdint>

namespace ui::radar {

/**
 * Range presets (label on ring 3 = ¾ of outer radius).
 *
 * Recommended for ADS-B on a 1.28″ display:
 *   5 km  — pattern / very local (airfield vicinity)
 *  10 km  — default; neighborhood spotting
 *  15 km  — wider local area
 *  25 km  — metro / regional picture
 *
 * Outer radius (for aircraft math) is ring-3 distance ÷ 0.75.
 */
struct RangePreset {
  /** Distance shown on ring 3 (¾ of outer radius), always stored in km. */
  float ring3_km;
  float outer_km;
};

constexpr float kRing3ToOuterKm = 4.0f / 3.0f;
constexpr float kKmPerMile = 1.609344f;
constexpr size_t kRangePresetCount = 5;

/** Runtime accessor — presets are user-configurable via the portal. */
const RangePreset& rangePreset(uint8_t index);

/** Load saved range and distance units from flash. Call once after boot. */
void rangeInit();
/** Cycle preset and save to flash. */
void rangeNext();
const RangePreset& rangeCurrent();
uint8_t rangeIndex();
/** ADSB fetch radius (km): scaled to screen edge so beyond-ring dots have data. */
float fetchRadiusKm();

bool useMiles();
bool showRunways();
/** WiFi portal checkbox: "T" = miles, otherwise km. */
void saveMilesFromPortal(const char* checkbox_value);
void saveRunwaysFromPortal(const char* checkbox_value);
void formatRing3Label(char* buf, size_t len, float ring3_km, bool use_miles);
void formatCurrentRing3Label(char* buf, size_t len);
/** Save 5 range presets from portal strings (input_is_miles: true if values are in mi). */
void saveRangePresetsFromPortal(const char* r0, const char* r1, const char* r2,
                                 const char* r3, const char* r4, bool input_is_miles);
/** Reset distance units to km (e.g. with WiFi credential wipe). */
void unitsReset();

/** Which cardinal direction appears at the top of the radar display. */
enum class DisplayTop : uint8_t { kNorth = 0, kSouth = 1, kEast = 2, kWest = 3 };
DisplayTop displayTop();
/** Portal text field value: single letter N/S/E/W (case-insensitive). */
void saveDisplayTopFromPortal(const char* value);

/** View cone: wedge drawn from radar center showing what's visible from the window. */
bool viewConeEnabled();
float viewConeLeftLat();
float viewConeLeftLon();
float viewConeRightLat();
float viewConeRightLon();
void saveViewConeFromPortal(const char* enabled,
                             const char* left_lat, const char* left_lon,
                             const char* right_lat, const char* right_lon);

}  // namespace ui::radar
