#include "main_window.hpp"
#include "app/app.hpp"
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <qnamespace.h>

#include "log/log.hpp"

namespace ui {
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setFixedSize(400, 400);

  auto *central = new QWidget(this);
  setCentralWidget(central);
  // central->setStyleSheet("background: white;");

  auto *layout = new QVBoxLayout(central);
  layout->setAlignment(Qt::AlignCenter);
  layout->setSpacing(20);

  auto *label = new QLabel("path", central);

  field_path_ = new QLineEdit(central);
  field_path_->setPlaceholderText("path/to/file");
  field_path_->setFixedWidth(250);
  field_path_->setReadOnly(true);

  // field_path_->setGeometry(100, 87, 200, 35);

  // Load Button
  button_load_ = new QPushButton("load", central);
  button_load_->setFixedWidth(200);
  // button_load_->setGeometry(150, 244, 100, 30);
  connect(button_load_, &QPushButton::clicked, this,
          &MainWindow::on_button_load_clicked);

  // grustnify Button
  button_grustnify_ = new QPushButton("grustnify", central);
  // button_grustnify_->setGeometry(100, 283, 200, 30);
  button_grustnify_->setFixedWidth(200);
  connect(button_grustnify_, &QPushButton::clicked, this,
          &MainWindow::on_button_grustnify_clicked);

  layout->addWidget(label, 0, Qt::AlignCenter);
  layout->addWidget(field_path_, 0, Qt::AlignCenter);
  layout->addWidget(button_load_, 0, Qt::AlignCenter);
  layout->addWidget(button_grustnify_, 0, Qt::AlignCenter);
}

MainWindow::~MainWindow() {}

void MainWindow::on_button_load_clicked() {
  TE_TRACE("load button clicked");
  // QMessageBox::information(this, "load", "TODO: loading file");
  const QString filter = "Audio Files (*.mp3 *.wav);;AllFiles(*)";
  const QString file_path = QFileDialog::getOpenFileName(
      this, "Select Auido File", QDir::homePath(), filter);
  if (file_path.isEmpty())
    return;
  field_path_->setText(file_path);

  auto *app = static_cast<app::App *>(qApp);
  app->load_audio_file(file_path);
}

void MainWindow::on_button_grustnify_clicked() {
  TE_TRACE("grustnify button clicked");

  auto *app = static_cast<app::App *>(qApp);
  app->process_audio_file();
}

} // namespace ui
