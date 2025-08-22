#ifndef CONCENTRIC_CIRCLES_DISPLAY_HPP_
#define CONCENTRIC_CIRCLES_DISPLAY_HPP_

#include <memory>
#include <string>

#include <QColor>
#include <QObject>
#include <QString>

#include <rviz_common/display.hpp>
#include <rviz_common/display_context.hpp>
#include <rviz_common/frame_manager_iface.hpp>               // FrameManagerIface を使うため
#include <rviz_common/properties/color_property.hpp>
#include <rviz_common/properties/float_property.hpp>
#include <rviz_common/properties/int_property.hpp>
#include <rviz_common/properties/tf_frame_property.hpp>
#include <rviz_common/properties/parse_color.hpp>

#include <rviz_rendering/objects/billboard_line.hpp>

#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>

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

private:
  // Properties (Qt parent-child ownership: this -> properties)
  rviz_common::properties::TfFrameProperty * frame_property_{nullptr};
  rviz_common::properties::ColorProperty * color_property_{nullptr};
  rviz_common::properties::FloatProperty * line_width_property_{nullptr};
  rviz_common::properties::FloatProperty * max_radius_property_{nullptr};
  rviz_common::properties::FloatProperty * spacing_property_{nullptr};
  rviz_common::properties::IntProperty * resolution_property_{nullptr};

  // Rendering objects
  std::vector<rviz_rendering::BillboardLine *> circles_;
  Ogre::SceneNode * scene_node_{nullptr};

  // cached style/geometry
  Ogre::ColourValue color_{0.0f, 0.0f, 0.0f, 1.0f};
  float line_width_{0.02f};
  float max_radius_{5.0f};
  float spacing_{1.0f};
  int resolution_{64};
};

} // namespace concentric_circles

#endif // CONCENTRIC_CIRCLES_DISPLAY_HPP_
