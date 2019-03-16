#include <QApplication>

#include "MainWindow.h"
#include <QDebug>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  MainWindow wgt;
  wgt.show();

  if(argc >= 2) {
      wgt.openFile(argv[1]);
  }

  return app.exec();
}
