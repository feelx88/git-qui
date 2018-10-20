#include <iostream>

#include <QApplication>

#include "mainwindow.h"

#include "gitinterface.h"
#include "gitcommit.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  MainWindow w;
  w.show();

  return app.exec();
}
