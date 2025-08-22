# RViz2 Concentric Circles Display Plugin

This package provides a display plugin for RViz2, the official visualization tool for ROS 2. It allows you to render concentric circles in the 3D view, which can be useful as a visual aid for gauging distances from a specified coordinate frame.

-----

## Features

You can dynamically configure the following parameters through the RViz2 graphical user interface:

  * **Reference Frame:** The TF frame that will serve as the center of the concentric circles.
  * **Color:** The color of the circles.
  * **Line Width:** The width of the circle lines, in meters.
  * **Max Radius:** The radius of the outermost circle to be drawn, in meters.
  * **Spacing:** The distance between each circle, in meters.
  * **Resolution:** The number of points used to draw each circle. A higher value results in a smoother circle.

-----

## Prerequisites

  * ROS 2
  * RViz2

-----

## How to Build

1.  Place this `concentric_circles_rviz_plugin` package inside the `src` directory of your ROS 2 workspace.

2.  From the root of your workspace, run the following commands to build the package:

    ```bash
    # Install dependencies
    rosdep install --from-paths src --ignore-src -r -y

    # Build the package
    colcon build --packages-select concentric_circles_rviz_plugin
    ```

-----

## How to Use

1.  Source your workspace's setup file:

    ```bash
    source install/setup.bash
    ```

2.  Launch RViz2:

    ```bash
    rviz2
    ```

3.  In the "Displays" panel on the left, click the **[Add]** button.

4.  In the "Create visualization" window, navigate through the `By display type` tab to find the `concentric_circles` group. Select `ConcentricCirclesDisplay` and click **[OK]**.

5.  Expand the newly added display's properties in the "Displays" panel to configure its parameters (Reference Frame, Max Radius, etc.).

-----

## License

This package is released under the **Apache License 2.0**.
