#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <complex>
#include <QMap>
#include "ui_mainwindow.h"

// Parę przykładowych inputów:

// szerokość, wysokość, liczba iteracji, część rzeczywista, część urojona, lewy brzeg prostokąta, prawy brzeg prostokąta, dolny brzeg prostokąta, górny brzeg prostokąta

// 700 700 100 -0.7 0.27105 -1.5 1.5 -1.5 1.5
// 700 700 100 -0.1184 -0.756 -1.5 1.5 -1.5 1.5
// 700 700 80 -0.752 -0.0144 -2.0 2.0 -2.0 2.0
// 700 700 150 -1.3136 -0.0144 -2.0 2.0 -1.5 1.5
// 700 700 150 0.2704 0.0072 -2.0 2.0 -2.0 2.0
// 700 700 100 -0.0104 0.0288 -1.5 1.5 -1.5 1.5

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Parametry zbioru Julii

    // Zmienne pod liczbę linii poziomych, pionowych oraz maksymalną liczbę iteracji do sprawdzenia
    int width, height, N;

    // c_re - część rzeczywista parametru c; c_im - część urojona parametru c;
    // x_min - lewy brzeg prostokąta; x_max - prawy brzeg prostokąta; y_min - dolny brzeg prostokąta; y_max - górny brzeg prostokąta
    double c_re, c_im, x_min, x_max, y_min, y_max;

    int num_threads; // Liczba wątków

    QImage *im; // Zmienna pod przechowywanie obrazu zbioru Julii
    QPoint m1; // Zmienna pod współrzędne myszki

    // Maksymalne rozmiary okna
    std::pair<int, int> resolution;

    // Uchwyt pod konsole w ui
    QPlainTextEdit *console;

    // Zmienne pod numery id czcionek Roboto
    int thinRobotoId, regularRobotoId, boldRobotoId;

    // Obiekt QPoint przechowujący ostatnią pozycję kursora
    QPoint lastMousePosition;

    // Zmienna boolowska odpowiedzialna za możliwość przesuwania obrazka
    bool isDragging = false;

    // Enum pod grubość czcionek
    enum Thickness{
        Thin,
        Regular,
        Bold
    };

    // Definicje map kolorów
    std::map<QString, std::vector<QColor>> colormaps = {
        {"colormap1", {
                          QColor(68, 1, 84), QColor(72, 35, 116), QColor(64, 67, 135),
                          QColor(52, 94, 141), QColor(41, 120, 142), QColor(32, 144, 140),
                          QColor(34, 167, 132), QColor(68, 190, 112), QColor(121, 209, 81),
                          QColor(189, 223, 38), QColor(253, 231, 37)
                      }},
        {"colormap2", {
                          QColor(0, 0, 0), QColor(255, 200, 0), QColor(255, 255, 255),
                          QColor(0, 0, 255), QColor(0, 0, 128)
                      }},
        {"colormap3", {
                      QColor(0, 32, 76), QColor(0, 42, 102), QColor(0, 52, 110),
                      QColor(50, 72, 109), QColor(100, 92, 100), QColor(150, 112, 85),
                      QColor(200, 150, 64), QColor(250, 200, 38), QColor(255, 255, 0)
                  }},
        {"colormap4", {
                       QColor(5, 5, 35), QColor(25, 25, 112), QColor(75, 0, 130),
                       QColor(138, 43, 226), QColor(70, 130, 180), QColor(32, 178, 170),
                       QColor(173, 216, 230), QColor(255, 182, 193), QColor(255, 239, 213),
                       QColor(255, 250, 250)
                   }},
        {"colormap5", {
                           QColor(0, 0, 128), QColor(0, 191, 255), QColor(70, 130, 180),
                           QColor(173, 216, 230), QColor(255, 255, 255),
                           QColor(255, 165, 0), QColor(255, 69, 0), QColor(220, 20, 60),
                           QColor(139, 0, 0)
                       }},
        {"colormap6", {
                          QColor(0, 0, 0), QColor(0, 255, 255), QColor(255, 20, 147),
                          QColor(75, 0, 130), QColor(148, 0, 211),
                          QColor(255, 105, 180), QColor(255, 255, 0), QColor(0, 255, 127)
                      }}
    };

    // Wskaźnik na aktywną mapę kolorów
    std::vector<QColor>* activeColormap = &colormaps["colormap1"];

    // Metoda rysująca piksel
    void drawPixel(int x, int y, int r, int g, int b, QImage *im);

    // Czyszczenie obrazu
    void clearImage(QImage *im);

    // Metoda ustawiająca poziomy i pionowy rozmiar ekranu
    std::pair<int, int> getDesktopResolution();

    // Metoda dodająca czcionki do aplikacji
    void addFonts();

    // Metoda zmieniająca wielkość widgeta przy zmianie rozdziałki ekranu komputera
    void resizeWidget(const QString& widgetName, int monitorWidth, int monitorHeight, double widthRatio, double heightRatio, int posX, int posY);

    // Metoda zmieniająca wielkość layoutu przy zmianie rozdziałki ekranu komputera
    void resizeLayout(const QString& layoutName, int monitorWidth, int monitorHeight, double widthRatio, double heightRatio, int posX, int posY);

    // Metoda centrująca widget w stosunku do ramki obrazu
    void centerWidgetInFrame(const QString& widgetName, const QString& frameName, double widthRatio, double heightRatio);

    // Metoda edytująca GUI
    void editGui();

    // Metoda pobierająca daną czcionkę
    QFont getFont(int fontSize, enum Thickness);

    // Metoda ustawiająca domyślne parametry dla zbioru Julii
    void setDefaultParameters();

    // Metoda przywracająca domyślne parametry programu
    void resetToDefaultParameters();

    // Metoda resetująca spinboxy
    void resetSpinboxes();

    // Ustawienie maksymalnej ilości wątków w programie (metoda powiązana również z GUI aplikacji)
    void setMaxThreadsValue();

    // Uruchomienie testu silnej skalowalności
    void runStrongScalingTest();

    // Sprawdzanie, czy dany punkt należy do zbioru Julii
    int check(std::complex<long double> z, std::complex<long double> c, int N);

    // Obliczanie obrazu zbioru
    void computeJulia(int start, int end, std::complex<long double> c, int N, QImage *im);

    // Metody powiązane z paletami barw
    QColor iterToColor(int n, int N);
    QColor iterToColor2(int n, int N);
    QColor iterToColor3(int n, int N);
    QColor iterToColor4(int n, int N);

    // Deklaracja wskaźnika do funkcji oraz jego przypisanie do jednej z metod
    QColor (MainWindow::*activeFunction)(int, int) = &MainWindow::iterToColor;

    // Funkcja do wywołania wskaźnika do funkcji członkowskiej
    QColor callActiveFunction(int n, int N);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);

private slots:

    void on_generateImagePushButton_clicked();

    void on_clearImagePushButton_clicked();

    void on_spinBox_valueChanged(int arg1);

    void on_spinBox_4_valueChanged(int arg1);

    void on_spinBox_3_valueChanged(int arg1);

    void on_spinBox_2_valueChanged(int arg1);

    void on_doubleSpinBox_3_valueChanged(double arg1);

    void on_doubleSpinBox_6_valueChanged(double arg1);

    void on_doubleSpinBox_4_valueChanged(double arg1);

    void on_doubleSpinBox_5_valueChanged(double arg1);

    void on_doubleSpinBox_2_valueChanged(double arg1);

    void on_doubleSpinBox_valueChanged(double arg1);

    void on_resetValuesPushButton_clicked();

    void on_strongScalingTestPushButton_clicked();

    void on_comboBox_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;

    QMap<QSpinBox*, int> defaultSpinBoxValues; // Przechowuje wartości domyślne dla QSpinBox
    QMap<QDoubleSpinBox*, double> defaultDoubleSpinBoxValues; // Dla QDoubleSpinBox
    int defaultComboBoxIndex; // Indeks defaultowej opcji w QComboBoxie

    void saveDefaultSpinBoxValues(); // Funkcja zapisująca domyślne wartości spinboxów
    void restoreDefaultSpinBoxValues(); // Funkcja przywracająca domyślne wartości spinboxów

};
#endif // MAINWINDOW_H
