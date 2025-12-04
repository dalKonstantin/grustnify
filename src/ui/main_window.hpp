#pragma once

#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>

namespace ui {
class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void on_button_load_clicked();
  void on_button_grustnify_clicked();

private:
  QPushButton *button_load_;
  QPushButton *button_grustnify_;
  QLineEdit *field_path_;
};
} // namespace ui
