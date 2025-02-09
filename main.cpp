#include "mainwindow.h"


#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Ustawienie ikonki aplikacji
    a.setWindowIcon(QIcon(":/resources/juliaset.png"));

    MainWindow w;
    // Ustawienie nazwy okna
    w.setWindowTitle("Zbiór Julii - Wątki w C++");
    w.show();
    return a.exec();
}
