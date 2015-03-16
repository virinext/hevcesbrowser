#include <QApplication>

#include "MainWindow.h"

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  
  MainWindow wgt;
  wgt.show();
  
  return app.exec();
}
