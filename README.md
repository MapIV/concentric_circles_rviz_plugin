# concentric_circles_rviz_plugin

> **TL;DR** — An RViz2 *display plugin* that draws evenly spaced **distance range-rings** (concentric circles) around a chosen coordinate frame, so operators can eyeball how far away objects are. Add it in RViz with **Add → concentric_circles → ConcentricCirclesDisplay**.

---

## Overview

In the `map4_smartpole` system, roadside "smartpoles" perceive vehicles and pedestrians, and a human operator watches the results in an RViz2 "portal" (RViz2 is the standard 3D visualizer for ROS 2). When you look at a flat top-down scene of detected objects, it is hard to judge distance by eye — is that car 30 m away, or 80 m?

This plugin solves that. It draws a set of concentric circles (like a radar screen or an archery target) centered on a reference point — typically a pole's `base_link` — with one ring every *N* meters and an optional number label on each ring. An operator can then glance at the scene and immediately read "this object is just past the 40 m ring."

It is a small, self-contained, general-purpose RViz2 plugin originally from [MapIV/concentric_circles_rviz_plugin](https://github.com/MapIV/concentric_circles_rviz_plugin). It publishes and subscribes to nothing — it is purely a visualization aid rendered locally inside RViz.

## Provides

A single RViz2 display plugin class:

| | |
|---|---|
| **C++ class** | `concentric_circles::ConcentricCirclesDisplay` |
| **Base class** | `rviz_common::Display` |
| **Plugin name** | `concentric_circles/ConcentricCirclesDisplay` (shown as **ConcentricCirclesDisplay** under the **concentric_circles** group in RViz's *Add* dialog) |

The class is registered for pluginlib discovery via `PLUGINLIB_EXPORT_CLASS(...)` in the source and the `plugin_description.xml` manifest, so RViz finds it automatically once the package is built and sourced.

**To add it in RViz:** open the *Displays* panel → click **Add** → in the *By display type* tab, expand **concentric_circles** → select **ConcentricCirclesDisplay** → **OK**.

## How it works

On each frame, the plugin:

1. Looks up the TF transform from RViz's fixed frame to the configured **Reference Frame** (e.g. `base_link`). TF is the ROS system that tracks where coordinate frames are relative to each other. If the transform is unavailable, the display shows a warning status and draws nothing at a valid location.
2. Computes how many rings fit — `floor(Max Radius / Spacing)` — and draws a circle at each multiple of **Spacing** out to **Max Radius**. For example, `Spacing = 10`, `Max Radius = 200` gives 20 rings at 10, 20, … 200 m.
3. Each ring is a closed line loop built from **Resolution** points (more points = smoother circle), rendered with the chosen **Color** and **Line Width**.
4. If **Show Text** is on, a label showing the ring's radius in meters (e.g. `40`) is placed on each ring, sized by **Text Size**.

The whole set of circles is attached to a scene node that is repositioned every frame to follow the reference frame, so the rings stay centered on the pole even as the view or fixed frame changes.

## Build

From the workspace root:

```bash
bash build.sh --packages-select concentric_circles_rviz_plugin
```

Then source the workspace so RViz can find the plugin:

```bash
source install/setup.bash
```

## Usage

There are two ways this plugin gets used:

- **Interactively** — start `rviz2`, then add the display via **Add → concentric_circles → ConcentricCirclesDisplay** and tune its properties in the *Displays* panel.
- **Via the portal (typical)** — the smartpole operator portal's RViz config already includes this display, configured per pole. See `rviz/default.rviz` in the repo, where it appears as:

  ```yaml
  - Class: concentric_circles/ConcentricCirclesDisplay
    Color: 0; 200; 200
    Line Width: 0.1
    Max Radius: 200
    Name: 同心円          # "concentric circles"
    Reference Frame: base_link
    Resolution: 99
    Show Text: true
    Spacing: 10
    Text Size: 3
  ```

  This centers 10 m-spaced rings out to 200 m on the selected pole's `base_link`.

## Configuration

All properties are editable live in the RViz *Displays* panel. Defaults below are the plugin's built-in defaults (as set in code); the portal's `default.rviz` overrides some of them.

| Property | Type | Default | Description |
|---|---|---|---|
| **Reference Frame** | TF frame | `base_link` | The coordinate frame the circles are centered on. |
| **Color** | color | `200; 200; 200` (light gray) | Color of the ring lines and the radius labels. |
| **Line Width** | float (m) | `0.02` | Thickness of each ring line, in meters. Min `0`. |
| **Max Radius** | float (m) | `200.0` | Radius of the outermost ring, in meters. Min `0`. |
| **Spacing** | float (m) | `10.0` | Distance between consecutive rings, in meters. Min `0`. |
| **Resolution** | int | `99` | Number of points used to draw each circle (higher = smoother). Range `3`–`99`. |
| **Show Text** | bool | `true` | Whether to show the radius number label on each ring. |
| **Text Size** | float (m) | `5.0` | Character height of the radius labels, in meters. Min `0`. |

## Notes

- **No rings appear if** `Spacing` or `Max Radius` is `0`, or if the **Reference Frame** has no available transform to the fixed frame (a *Transform* warning is shown in that case).
- Labels are drawn as always-facing-camera billboard text, so they stay readable from any viewing angle.
- The plugin is purely visual: it has no topics, parameters, or nodes, and adds no runtime load beyond local rendering.
- License: Apache License 2.0.
