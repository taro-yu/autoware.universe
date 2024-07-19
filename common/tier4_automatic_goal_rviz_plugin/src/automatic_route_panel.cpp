#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QComboBox>
#include <QDir>
#include <fstream>

#include "automatic_route_panel.hpp"



namespace rviz_plugins
{
AutowareAutomaticRoutePanel::AutowareAutomaticRoutePanel(QWidget * parent)
: rviz_common::Panel(parent)
{
  // qt_timer_ = new QTimer(this);
  // connect(
  //   qt_timer_, &QTimer::timeout, this, &AutowareAutomaticGoalPanel::updateAutoExecutionTimerTick);

  // auto * h_layout = new QHBoxLayout(this);
  auto * v_layout = new QVBoxLayout(this);
  v_layout->addWidget(makeRouteListGroup());
  // h_layout->addWidget(makeRouteListGroup());
  // v_layout->addWidget(makeEngagementGroup());
  // v_layout->addWidget(makeRoutingGroup());
  // h_layout->addLayout(v_layout);
  setLayout(v_layout);
}

void AutowareAutomaticRoutePanel::onInitialize()
{
  raw_node_ = this->getDisplayContext()->getRosNodeAbstraction().lock()->get_raw_node();
  pub_marker_ = raw_node_->create_publisher<MarkerArray>("~/automatic_goal/markers", 0);
//   sub_append_goal_ = raw_node_->create_subscription<PoseStamped>(
//     "~/automatic_goal/goal", 5,
//     std::bind(&AutowareAutomaticGoalPanel::onAppendGoal, this, std::placeholders::_1));
//   initCommunication(raw_node_.get());
}


// Layouts
QGroupBox * AutowareAutomaticRoutePanel::makeRouteListGroup()
{
    auto * group = new QGroupBox("RouteList", this);
    auto * grid = new QGridLayout(group);

    // Load Route list From Directory
    load_folder_button_ = new QPushButton("Load Routes From Directory", group);
    connect(load_folder_button_, SIGNAL(clicked()), SLOT(loadRouteFiles()));
    grid->addWidget(load_folder_button_, 0, 0);

    // Route List
    routes_list_widget_ptr_ = new QListWidget(group);
    routes_list_widget_ptr_->setStyleSheet("border:1px solid black;");
    grid->addWidget(routes_list_widget_ptr_, 1, 0);

    // セットルートボタン
    set_route_button_ = new QPushButton("Set Route");
    connect(set_route_button_, SIGNAL(clicked()), SLOT(setRoute()));
    grid->addWidget(set_route_button_, 2, 0);

    // クリアルートボタン
    clear_route_button_ = new QPushButton("Clear Route");
    connect(clear_route_button_, SIGNAL(clicked()), SLOT(clearRoute()));
    grid->addWidget(clear_route_button_, 3, 0);

    // // ルート選択コンボボックス
    // route_selector = new QComboBox;
    // layout->addWidget(route_selector);

  group->setLayout(grid);
  return group;
  }


  void AutowareAutomaticRoutePanel::setRoute()
  {
    QListWidgetItem *selectedItem = routes_list_widget_ptr_->currentItem();
    if (!selectedItem) {
        return;  
    }

    int selectedIndex = routes_list_widget_ptr_->currentRow();

 
    if (selectedIndex < 0 || selectedIndex >= (int)route_paths_.size()) {
        return;
    }

    std::string full_path = route_paths_[selectedIndex] + "/" + route_names_[selectedIndex];
    

    // コマンドを実行してルートを設定
    std::string command = "ros2 service call /api/routing/set_route msgs/srv/SetRoute \"$(cat " + full_path + ")\"";
    int result = system(command.c_str());

    if (result != 0) {
        std::cerr << "Failed to call the set route service. Error code: " << result << std::endl;
    } else {
        std::cout << "Successfully called the set route service." << std::endl;
    }
  }

  void AutowareAutomaticRoutePanel::clearRoute()
  {
    // コマンドを実行してルートをクリア
    std::string command = "ros2 service call /api/routing/clear_route msgs/srv/ClearRoute";
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Failed to call the clear route service. Error code: " << result << std::endl;
    } else {
        std::cout << "Successfully called the clear route service." << std::endl;
    }
  }

  void AutowareAutomaticRoutePanel::loadRouteFiles()
  {

    QString folder_path = QFileDialog::getExistingDirectory(
      this, "Please select a directory", "/home/user", QFileDialog::ShowDirsOnly);
    if (folder_path.isEmpty())
    {
        return;
    }

    QDir directory(folder_path);
    QStringList route_files = directory.entryList(QStringList() << "*.route", QDir::Files);

    routes_list_widget_ptr_->clear();
    route_names_.clear();
    route_paths_.clear();

    // 各ファイルについて処理
    for (const QString &route_file : route_files)
    {
        // ファイルの絶対パスを取得
        QString absolute_path = directory.absoluteFilePath(route_file);

        // ファイル名と絶対パスを保存
        route_names_.push_back(route_file.toStdString());
        route_paths_.push_back(absolute_path.toStdString());

        // RVizにはファイル名のみを表示
        routes_list_widget_ptr_->addItem(route_file);
    }
}

// private:
//   QComboBox *route_selector;
} // namespace rviz_plugins

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(rviz_plugins::AutowareAutomaticRoutePanel, rviz_common::Panel)
