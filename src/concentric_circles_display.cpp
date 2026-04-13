#include "concentric_circles_rviz_plugin/concentric_circles_display.hpp"

#include <rclcpp/rclcpp.hpp>
#include <rviz_common/display_context.hpp>
#include <rviz_common/frame_manager_iface.hpp>
#include <rviz_common/properties/parse_color.hpp>
#include <rviz_rendering/objects/billboard_line.hpp>

#include <OgreQuaternion.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreVector3.h>
#include <rcl/time.h>

namespace concentric_circles
{

ConcentricCirclesDisplay::ConcentricCirclesDisplay() : rviz_common::Display()
{
}

ConcentricCirclesDisplay::~ConcentricCirclesDisplay()
{
  for (auto circle : circles_) {
    delete circle;
  }
  circles_.clear();

  for (auto node : text_nodes_) {
    node->detachAllObjects();
    scene_node_->removeAndDestroyChild(node);
  }
  text_nodes_.clear();

  for (auto label : text_labels_) {
    delete label;
  }
  text_labels_.clear();

  if (scene_node_) {
    scene_node_->getParentSceneNode()->removeChild(scene_node_);
    scene_node_ = nullptr;
  }
}

void ConcentricCirclesDisplay::onInitialize()
{
  scene_node_ = scene_manager_->getRootSceneNode()->createChildSceneNode();

  color_property_ = new rviz_common::properties::ColorProperty(
    "Color", QColor(200, 200, 200), "Color of the concentric circles", this, SLOT(updateStyle()),
    this);

  line_width_property_ = new rviz_common::properties::FloatProperty(
    "Line Width", 0.02f, "Width of the circle lines (meters)", this, SLOT(updateStyle()), this);
  line_width_property_->setMin(0.0f);

  max_radius_property_ = new rviz_common::properties::FloatProperty(
    "Max Radius", 200.0f, "Maximum radius of concentric circles (meters)", this,
    SLOT(updateGeometry()), this);
  max_radius_property_->setMin(0.0f);

  spacing_property_ = new rviz_common::properties::FloatProperty(
    "Spacing", 10.0f, "Spacing between concentric circles (meters)", this, SLOT(updateGeometry()),
    this);
  spacing_property_->setMin(0.0f);

  resolution_property_ = new rviz_common::properties::IntProperty(
    "Resolution", 99, "Number of points per circle (higher = smoother)", this,
    SLOT(updateGeometry()), this);
  resolution_property_->setMin(3);
  resolution_property_->setMax(99);

  plane_property_ = new rviz_common::properties::EnumProperty(
    "Plane", "XY", "Plane on which to draw the concentric circles", this,
    SLOT(updateGeometry()), this);
  plane_property_->addOption("XY", 0);
  plane_property_->addOption("XZ", 1);
  plane_property_->addOption("YZ", 2);

  frame_property_ = new rviz_common::properties::TfFrameProperty(
    "Reference Frame", "base_link", "Reference frame for circles", this, nullptr, false,
    SLOT(updateGeometry()), this);
  frame_property_->setFrameManager(context_->getFrameManager());

  show_text_property_ = new rviz_common::properties::BoolProperty(
    "Show Text", true, "Show radius text labels", this, SLOT(updateShowText()), this);

  text_size_property_ = new rviz_common::properties::FloatProperty(
    "Text Size", 5.0f, "Character height of the radius labels in meters", this,
    SLOT(updateTextSize()), this);
  text_size_property_->setMin(0.0f);

  text_angle_property_ = new rviz_common::properties::IntProperty(
    "Text Angle", 0, "Angle in degrees (0-359) where text labels are placed on each circle", this,
    SLOT(updateGeometry()), this);
  text_angle_property_->setMin(0);
  text_angle_property_->setMax(359);

  updateStyle();
  updateGeometry();
  updateTextSize();
}

void ConcentricCirclesDisplay::onEnable()
{
  if (scene_node_) {
    scene_node_->setVisible(true);
  }
  updateGeometry();
}

void ConcentricCirclesDisplay::onDisable()
{
  if (scene_node_) {
    scene_node_->setVisible(false);
  }
}

void ConcentricCirclesDisplay::updateStyle()
{
  color_ = rviz_common::properties::qtToOgre(color_property_->getColor());
  line_width_ = line_width_property_->getFloat();

  for (auto circle : circles_) {
    circle->setLineWidth(line_width_);
  }

  for (auto label : text_labels_) {
    label->setColor(color_);
  }

  updateGeometry();

  if (context_) {
    context_->queueRender();
  }
}

void ConcentricCirclesDisplay::updateShowText()
{
  show_text_ = show_text_property_->getBool();
  for (auto label : text_labels_) {
    label->setVisible(show_text_);
  }
  if (context_) {
    context_->queueRender();
  }
}

void ConcentricCirclesDisplay::updateGeometry()
{
  max_radius_ = max_radius_property_->getFloat();
  spacing_ = spacing_property_->getFloat();
  resolution_ = resolution_property_->getInt();

  for (auto circle : circles_) {
    delete circle;
  }
  circles_.clear();

  for (auto node : text_nodes_) {
    node->detachAllObjects();
    scene_node_->removeAndDestroyChild(node);
  }
  text_nodes_.clear();

  for (auto label : text_labels_) {
    delete label;
  }
  text_labels_.clear();

  if (spacing_ <= 0.0f || max_radius_ <= 0.0f) {
    if (context_) {
      context_->queueRender();
    }
    return;
  }

  int num_circles = static_cast<int>(std::floor(max_radius_ / spacing_));
  const int plane = plane_property_->getOptionInt();

  for (int i = 1; i <= num_circles; ++i) {
    rviz_rendering::BillboardLine * circle_line =
      new rviz_rendering::BillboardLine(scene_manager_, scene_node_);

    circle_line->setLineWidth(line_width_);

    const float r = spacing_ * static_cast<float>(i);

    for (int s = 0; s <= resolution_; ++s) {
      const float theta =
        (2.0f * Ogre::Math::PI * static_cast<float>(s)) / static_cast<float>(resolution_);
      const float a = r * std::cos(theta);
      const float b = r * std::sin(theta);

      Ogre::Vector3 point;
      if (plane == 0) {        // XY
        point = Ogre::Vector3(a, b, 0.0f);
      } else if (plane == 1) { // XZ
        point = Ogre::Vector3(a, 0.0f, b);
      } else {                 // YZ
        point = Ogre::Vector3(0.0f, a, b);
      }
      circle_line->addPoint(point, color_);
    }

    circles_.push_back(circle_line);

    std::stringstream ss;
    ss << std::fixed << std::setprecision(0) << r;

    Ogre::SceneNode * text_node = scene_node_->createChildSceneNode();

    const float angle_rad =
      Ogre::Degree(static_cast<float>(text_angle_property_->getInt())).valueRadians();
    const float ta = r * std::cos(angle_rad);
    const float tb = r * std::sin(angle_rad);

    Ogre::Vector3 text_pos;
    Ogre::Quaternion text_rot = Ogre::Quaternion::IDENTITY;
    if (plane == 0) {        // XY
      text_pos = Ogre::Vector3(ta, tb, 0.0f);
      text_rot = Ogre::Quaternion(Ogre::Degree(-90), Ogre::Vector3::UNIT_X);
    } else if (plane == 1) { // XZ
      text_pos = Ogre::Vector3(ta, 0.0f, tb);
    } else {                 // YZ
      text_pos = Ogre::Vector3(0.0f, ta, tb);
      text_rot = Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3::UNIT_Y);
    }
    text_node->setPosition(text_pos);
    text_node->setOrientation(text_rot);

    rviz_rendering::MovableText * text_label =
      new rviz_rendering::MovableText(ss.str());
    text_node->attachObject(text_label);

    text_label->setTextAlignment(
      rviz_rendering::MovableText::HorizontalAlignment::H_LEFT,
      rviz_rendering::MovableText::VerticalAlignment::V_CENTER);
    text_label->setCharacterHeight(text_size_);
    text_label->setColor(color_);
    text_label->setVisible(show_text_);

    text_labels_.push_back(text_label);
    text_nodes_.push_back(text_node);
  }

  if (context_) {
    context_->queueRender();
  }
}

void ConcentricCirclesDisplay::updateTextSize()
{
  text_size_ = text_size_property_->getFloat();

  for (auto label : text_labels_) {
    label->setCharacterHeight(text_size_);
  }

  if (context_) {
    context_->queueRender();
  }
}

void ConcentricCirclesDisplay::update(float /*wall_dt*/, float /*ros_dt*/)
{
  if (!frame_property_) return;

  std::string frame = frame_property_->getFrameStd();

  Ogre::Vector3 position;
  Ogre::Quaternion orientation;

  rclcpp::Time t(0, 0, RCL_ROS_TIME);

  auto fm_iface = context_->getFrameManager();
  bool ok = fm_iface->getTransform(frame, t, position, orientation);

  if (!ok) {
    setStatus(
      rviz_common::properties::StatusProperty::Warn, "Transform",
      "Transform to fixed frame not available for selected Reference Frame");
    if (scene_node_) scene_node_->setPosition(Ogre::Vector3::ZERO);
    return;
  }

  setStatus(rviz_common::properties::StatusProperty::Ok, "Transform", "OK");
  if (scene_node_) {
    scene_node_->setPosition(position);
    scene_node_->setOrientation(orientation);
  }
}

}  // namespace concentric_circles

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(concentric_circles::ConcentricCirclesDisplay, rviz_common::Display)
