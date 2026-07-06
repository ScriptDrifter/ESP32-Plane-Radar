#pragma once

namespace ui {

/** Draw the static sonar/radar grid (black disc, green overlay, labels). */
void radarDisplayDraw();

/** Redraw aircraft only (blits cached grid; no full-screen clear). */
void radarDisplayRefreshAircraft();

/** Invalidate cached label metrics so they are recalculated on next draw (e.g. after range presets change). */
void radarDisplayResetMetrics();

}  // namespace ui
