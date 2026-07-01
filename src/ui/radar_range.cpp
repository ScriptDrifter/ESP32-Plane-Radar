#include "ui/radar_range.h"

#include "ui/radar_theme.h"

#include <Preferences.h>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace ui::radar {

namespace {

constexpr char kPrefsNamespace[] = "planeradar";
constexpr char kPrefsRangeKey[] = "rangeIdx";
constexpr char kPrefsMilesKey[] = "useMiles";
constexpr char kPrefsRunwaysKey[] = "showRwys";
constexpr char kPrefsDisplayTopKey[] = "displayTop";
constexpr char kPrefsVcEnabledKey[]  = "vcEnabled";
constexpr char kPrefsVcLeftLatKey[]  = "vcLeftLat";
constexpr char kPrefsVcLeftLonKey[]  = "vcLeftLon";
constexpr char kPrefsVcRightLatKey[] = "vcRightLat";
constexpr char kPrefsVcRightLonKey[] = "vcRightLon";
constexpr uint8_t kDefaultRangeIndex = 1;  // 10 km ring
constexpr float kKmPerMile = 1.609344f;

Preferences s_prefs;
uint8_t s_range_index = kDefaultRangeIndex;
bool s_use_miles = false;
bool s_show_runways = true;
DisplayTop s_display_top = DisplayTop::kNorth;
bool  s_vc_enabled   = false;
float s_vc_left_lat  = 0.0f;
float s_vc_left_lon  = 0.0f;
float s_vc_right_lat = 0.0f;
float s_vc_right_lon = 0.0f;

void saveRangeIndex() {
  if (!s_prefs.begin(kPrefsNamespace, false)) {
    return;
  }
  s_prefs.putUChar(kPrefsRangeKey, s_range_index);
  s_prefs.end();
}

void saveUseMiles() {
  if (!s_prefs.begin(kPrefsNamespace, false)) {
    return;
  }
  s_prefs.putBool(kPrefsMilesKey, s_use_miles);
  s_prefs.end();
}

void saveShowRunways() {
  if (!s_prefs.begin(kPrefsNamespace, false)) {
    return;
  }
  s_prefs.putBool(kPrefsRunwaysKey, s_show_runways);
  s_prefs.end();
}

bool portalCheckboxChecked(const char* value) {
  if (value == nullptr || value[0] == '\0') {
    return false;
  }
  // WiFiManager checkbox submits its value= attribute ("T", or "F" if we prefilled F).
  if ((value[0] == 'T' || value[0] == 't' || value[0] == 'F' || value[0] == 'f') &&
      value[1] == '\0') {
    return true;
  }
  return strcmp(value, "on") == 0;
}

}  // namespace

void rangeInit() {
  if (!s_prefs.begin(kPrefsNamespace, true)) {
    return;
  }
  const uint8_t saved = s_prefs.getUChar(kPrefsRangeKey, kDefaultRangeIndex);
  s_range_index =
      (saved < kRangePresetCount) ? saved : kDefaultRangeIndex;
  s_use_miles = s_prefs.getBool(kPrefsMilesKey, false);
  s_show_runways = s_prefs.getBool(kPrefsRunwaysKey, true);
  const uint8_t saved_top = s_prefs.getUChar(kPrefsDisplayTopKey, 0);
  s_display_top = (saved_top < 4) ? static_cast<DisplayTop>(saved_top)
                                  : DisplayTop::kNorth;
  s_vc_enabled   = s_prefs.getBool(kPrefsVcEnabledKey, false);
  s_vc_left_lat  = s_prefs.getFloat(kPrefsVcLeftLatKey,  0.0f);
  s_vc_left_lon  = s_prefs.getFloat(kPrefsVcLeftLonKey,  0.0f);
  s_vc_right_lat = s_prefs.getFloat(kPrefsVcRightLatKey, 0.0f);
  s_vc_right_lon = s_prefs.getFloat(kPrefsVcRightLonKey, 0.0f);
  s_prefs.end();
}

void rangeNext() {
  s_range_index = static_cast<uint8_t>((s_range_index + 1) % kRangePresetCount);
  saveRangeIndex();
}

const RangePreset& rangeCurrent() { return kRangePresets[s_range_index]; }

uint8_t rangeIndex() { return s_range_index; }

float fetchRadiusKm() {
  const float outer_km = rangeCurrent().outer_km;
  const float screen_r_px =
      static_cast<float>(kCenterX - kBeyondRingScreenMarginPx);
  return outer_km * (screen_r_px / static_cast<float>(kGridOuterRadius));
}

bool useMiles() { return s_use_miles; }

bool showRunways() { return s_show_runways; }

void saveMilesFromPortal(const char* checkbox_value) {
  s_use_miles = portalCheckboxChecked(checkbox_value);
  saveUseMiles();
  Serial.printf("Distance units: %s\n", s_use_miles ? "miles" : "km");
}

void saveRunwaysFromPortal(const char* checkbox_value) {
  s_show_runways = portalCheckboxChecked(checkbox_value);
  saveShowRunways();
  Serial.printf("Runway overlay: %s\n", s_show_runways ? "on" : "off");
}

void formatRing3Label(char* buf, size_t len, float ring3_km, bool use_miles) {
  if (use_miles) {
    const int mi = static_cast<int>(lroundf(ring3_km / kKmPerMile));
    snprintf(buf, len, "%dmi", mi);
  } else {
    const int km = static_cast<int>(lroundf(ring3_km));
    snprintf(buf, len, "%dkm", km);
  }
}

void formatCurrentRing3Label(char* buf, size_t len) {
  formatRing3Label(buf, len, rangeCurrent().ring3_km, s_use_miles);
}

void unitsReset() {
  s_use_miles = false;
  s_show_runways = true;
  s_display_top = DisplayTop::kNorth;
  s_vc_enabled = false;
  s_vc_left_lat = s_vc_left_lon = s_vc_right_lat = s_vc_right_lon = 0.0f;
  if (s_prefs.begin(kPrefsNamespace, false)) {
    s_prefs.remove(kPrefsMilesKey);
    s_prefs.remove(kPrefsRunwaysKey);
    s_prefs.remove(kPrefsDisplayTopKey);
    s_prefs.remove(kPrefsVcEnabledKey);
    s_prefs.remove(kPrefsVcLeftLatKey);
    s_prefs.remove(kPrefsVcLeftLonKey);
    s_prefs.remove(kPrefsVcRightLatKey);
    s_prefs.remove(kPrefsVcRightLonKey);
    s_prefs.end();
  }
}

DisplayTop displayTop() { return s_display_top; }

void saveDisplayTopFromPortal(const char* value) {
  if (!value || value[0] == '\0') return;
  DisplayTop dt;
  switch (value[0] | 0x20) {  // tolower
    case 'n': dt = DisplayTop::kNorth; break;
    case 's': dt = DisplayTop::kSouth; break;
    case 'e': dt = DisplayTop::kEast;  break;
    case 'w': dt = DisplayTop::kWest;  break;
    default:  return;
  }
  s_display_top = dt;
  if (s_prefs.begin(kPrefsNamespace, false)) {
    s_prefs.putUChar(kPrefsDisplayTopKey, static_cast<uint8_t>(dt));
    s_prefs.end();
  }
  const char* names[] = {"North", "South", "East", "West"};
  Serial.printf("Radar top: %s\n", names[static_cast<int>(dt)]);
}

bool viewConeEnabled()   { return s_vc_enabled; }
float viewConeLeftLat()  { return s_vc_left_lat; }
float viewConeLeftLon()  { return s_vc_left_lon; }
float viewConeRightLat() { return s_vc_right_lat; }
float viewConeRightLon() { return s_vc_right_lon; }

void saveViewConeFromPortal(const char* enabled,
                             const char* left_lat, const char* left_lon,
                             const char* right_lat, const char* right_lon) {
  s_vc_enabled   = portalCheckboxChecked(enabled);
  s_vc_left_lat  = left_lat  ? strtof(left_lat,  nullptr) : 0.0f;
  s_vc_left_lon  = left_lon  ? strtof(left_lon,  nullptr) : 0.0f;
  s_vc_right_lat = right_lat ? strtof(right_lat, nullptr) : 0.0f;
  s_vc_right_lon = right_lon ? strtof(right_lon, nullptr) : 0.0f;
  if (s_prefs.begin(kPrefsNamespace, false)) {
    s_prefs.putBool(kPrefsVcEnabledKey,   s_vc_enabled);
    s_prefs.putFloat(kPrefsVcLeftLatKey,  s_vc_left_lat);
    s_prefs.putFloat(kPrefsVcLeftLonKey,  s_vc_left_lon);
    s_prefs.putFloat(kPrefsVcRightLatKey, s_vc_right_lat);
    s_prefs.putFloat(kPrefsVcRightLonKey, s_vc_right_lon);
    s_prefs.end();
  }
  Serial.printf("View cone: %s  L(%.4f,%.4f)  R(%.4f,%.4f)\n",
                s_vc_enabled ? "on" : "off",
                s_vc_left_lat, s_vc_left_lon,
                s_vc_right_lat, s_vc_right_lon);
}

}  // namespace ui::radar
