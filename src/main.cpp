#include <iostream>

#include <QApplication>

#include "mainwindow.hpp"

#include "gitinterface.hpp"
#include "gitcommit.hpp"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  MainWindow w;
  w.show();

  return app.exec();
}
