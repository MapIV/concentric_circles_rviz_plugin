#ifndef CONCENTRIC_CIRCLES_DISPLAY_HPP_
#define CONCENTRIC_CIRCLES_DISPLAY_HPP_

#include <QColor>
#include <QObject>
#include <QString>
#include <rviz_common/display.hpp>
#include <rviz_common/display_context.hpp>
#include <rviz_common/frame_manager_iface.hpp>
#include <rviz_common/properties/bool_property.hpp>
#include <rviz_common/properties/color_property.hpp>
#include <rviz_common/properties/float_property.hpp>
#include <rviz_common/properties/int_property.hpp>
#include <rviz_common/properties/parse_color.hpp>
#include <rviz_common/properties/tf_frame_property.hpp>
#include <rviz_rendering/objects/billboard_line.hpp>
#include <rviz_rendering/objects/movable_text.hpp>

#include <OgreQuaternion.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreVector3.h>

#include <memory>
#include <string>
#include <vector>

namespace concentric_circles
{

class ConcentricCirclesDisplay : public rviz_common::Display
{
  Q_OBJECT
public:
  ConcentricCirclesDisplay();
  ~ConcentricCirclesDisplay() override;

  void onInitialize() override;
  void onEnable() override;
  void onDisable() override;
  void update(float wall_dt, float ros_dt) override;

private Q_SLOTS:
  void updateStyle();
  void updateGeometry();
  void updateShowText();
  void updateTextSize();

private:
  rviz_common::properties::TfFrameProperty * frame_property_{nullptr};
  rviz_common::properties::ColorProperty * color_property_{nullptr};
  rviz_common::properties::FloatProperty * line_width_property_{nullptr};
  rviz_common::properties::FloatProperty * max_radius_property_{nullptr};
  rviz_common::properties::FloatProperty * spacing_property_{nullptr};
  rviz_common::properties::IntProperty * resolution_property_{nullptr};
  rviz_common::properties::BoolProperty * show_text_property_{nullptr};
  rviz_common::properties::FloatProperty * text_size_property_{nullptr};

  std::vector<rviz_rendering::BillboardLine *> circles_;
  std::vector<rviz_rendering::MovableText *> text_labels_;
  Ogre::SceneNode * scene_node_{nullptr};

  Ogre::ColourValue color_{0.0f, 0.0f, 0.0f, 1.0f};
  float line_width_{0.02f};
  float max_radius_{200.0f};
  float spacing_{10.0f};
  int resolution_{99};
  bool show_text_{true};
  float text_size_{0.5f};
};

}  // namespace concentric_circles

#endif  // CONCENTRIC_CIRCLES_DISPLAY_HPP_
