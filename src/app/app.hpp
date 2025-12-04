#include <QApplication>

namespace app {
class App : public QApplication {
  Q_OBJECT
public:
  App(int &argc, char **argv);
  ~App();

  void load_audio_file(const QString &path);
  void process_audio_file();

private:
  QString file_path_;
};
} // namespace app
