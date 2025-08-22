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
    "Max Radius", 5.0f, "Maximum radius of concentric circles (meters)", this,
    SLOT(updateGeometry()), this);
  max_radius_property_->setMin(0.0f);

  spacing_property_ = new rviz_common::properties::FloatProperty(
    "Spacing", 1.0f, "Spacing between concentric circles (meters)", this, SLOT(updateGeometry()),
    this);
  spacing_property_->setMin(0.0f);

  resolution_property_ = new rviz_common::properties::IntProperty(
    "Resolution", 64, "Number of points per circle (higher = smoother)", this,
    SLOT(updateGeometry()), this);
  resolution_property_->setMin(3);

  frame_property_ = new rviz_common::properties::TfFrameProperty(
  "Reference Frame", "base_link", "Reference frame for circles", this,
  nullptr, false, SLOT(updateGeometry()), this);
  frame_property_->setFrameManager(context_->getFrameManager());

  updateStyle();
  updateGeometry();
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

  // ベクター内の全ての circle オブジェクトに対して処理を行う
  if (!circles_.empty()) {
    for (auto circle : circles_) {
      // 各 circle オブジェクトの色と線の太さを更新
      // （注：色を変更するにはaddPointで指定し直す必要があるため、ここでは線の太さのみ更新）
      circle->setLineWidth(line_width_);
    }
  }

  // updateGeometryを呼ぶことで、色も反映された状態で再描画させるのが確実です
  updateGeometry();

  if (context_) {
    context_->queueRender();
  }
}

void ConcentricCirclesDisplay::updateGeometry()
{
  // プロパティパネルから現在の値を取得
  max_radius_ = max_radius_property_->getFloat();
  spacing_ = spacing_property_->getFloat();
  resolution_ = resolution_property_->getInt();

  // --- クラッシュ対策 ---
  // BillboardLineの点数上限(16384)をResolutionが超えていないかチェック
  if (resolution_ > 99) {
    // RVizのステータスパネルにエラーメッセージを表示
    setStatus(
      rviz_common::properties::StatusProperty::Error, "Resolution",
      QString("Value is too high (limit: 99)."));

    // 現在表示されている円を全て削除して、処理を中断
    for (auto circle : circles_) {
      delete circle;
    }
    circles_.clear();
    return;  // 描画処理を行わずにこの関数を抜ける
  }
  // 値が正常範囲であれば、ステータスをOKに戻す
  setStatus(rviz_common::properties::StatusProperty::Ok, "Resolution", "OK");

  // --- 再描画のための準備 ---
  // 以前の描画で作成したBillboardLineオブジェクトをすべて削除
  for (auto circle : circles_) {
    delete circle;
  }
  circles_.clear();

  // --- 描画条件のチェック ---
  // 半径や間隔が0以下なら何も描画しない
  if (spacing_ <= 0.0f || max_radius_ <= 0.0f) {
    if (context_) {
      context_->queueRender();
    }
    return;
  }

  // --- 円の描画処理 ---
  // 表示する円の数を計算
  int num_circles = static_cast<int>(std::floor(max_radius_ / spacing_));

  // 計算された数の円を1つずつ生成・描画
  for (int i = 1; i <= num_circles; ++i) {
    // 円1つにつき、1つのBillboardLineオブジェクトを新規作成
    rviz_rendering::BillboardLine * circle_line =
      new rviz_rendering::BillboardLine(scene_manager_, scene_node_);

    // 現在のスタイル（線の太さ）を設定
    circle_line->setLineWidth(line_width_);

    const float r = spacing_ * static_cast<float>(i);

    // Resolutionの値に基づいて円周上の点を計算し、線に追加
    for (int s = 0; s <= resolution_; ++s) {
      const float theta =
        (2.0f * Ogre::Math::PI * static_cast<float>(s)) / static_cast<float>(resolution_);
      const float x = r * std::cos(theta);
      const float y = r * std::sin(theta);
      // 現在のスタイル（色）で点を追加
      circle_line->addPoint(Ogre::Vector3(x, y, 0.0f), color_);
    }
    // 完成した円オブジェクトを管理用のベクターに追加
    circles_.push_back(circle_line);
  }

  // --- 画面更新 ---
  // 描画内容の変更を画面に反映させるようRVizに通知
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

  // Use rclcpp::Time(0,0,RCL_ROS_TIME) to request "latest" transform like other ROS2 plugins do.
  // (Many RViz2 plugins use rclcpp::Time(0,0,RCL_ROS_TIME) when calling getTransform.)
  rclcpp::Time t(0, 0, RCL_ROS_TIME);

  // FrameManagerIface (what context_->getFrameManager() returns) exposes getTransform(...)
  // so call it directly.
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

// Plugin export macro
#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(concentric_circles::ConcentricCirclesDisplay, rviz_common::Display)
