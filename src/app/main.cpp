#include "app.hpp"
#include "log/log.hpp"
#include "ui/main_window.hpp"
#include <QApplication>

int main(int argc, char *argv[]) {
  grustnify::Log::Init();
  app::App a(argc, argv);
  ui::MainWindow w;
  w.show();
  return a.exec();
}
