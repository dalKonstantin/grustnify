#include "app.hpp"
#include <QApplication>

#include "log/log.hpp"

App::App(int &argc, char **argv) : QApplication(argc, argv) {};
App::~App() = default;

void App::load_audio_file(const QString &path) {
  TE_INFO("loading file");
  file_path_ = path;
}
void App::process_audio_file() { TE_INFO("processing file"); }
