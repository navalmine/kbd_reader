#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName("codex_demo");
  QCoreApplication::setApplicationName("kbd_sim_ui");
  MainWindow w;
  w.resize(640, 360);
  w.show();
  return app.exec();
}
