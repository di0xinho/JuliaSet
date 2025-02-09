#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <thread>
#include <QPainter>
#include <QMouseEvent>
#include <complex>
#include <chrono>
#include <QFontDatabase>
#include <QMessageBox>
#include <QElapsedTimer>

// Konstruktor
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Maksymalne wymiary okna aplikacji
    resolution = getDesktopResolution();

    // Ustawienie maksymalnych wymiarów okna aplikacji
    setFixedSize(resolution.first, resolution.second);
    showMaximized();

    // Wstępna edycja GUI (chodzi głównie o czcionki)
    editGui();

    // Przypisanie wartości dla wysokości i szerokości dla naszej ramki, w której wyświetlany będzie zbiór Julii
    width = ui->screen->width(); // szerokość ramki
    height = ui->screen->height(); // wysokość ramki

    // Nadajemy obrazkowi wymiary
    im = new QImage(width, height, QImage::Format_RGB32);

    // Zapisujemy domyślne wartości spinboxów zaraz po ich utworzeniu
    saveDefaultSpinBoxValues();

    // Ustawienie domyślnych parametrów programu
    setDefaultParameters();

    // Na początku działania programu czyścimy obraz - wszystkie piksele obrazu stają się czarne
    clearImage(im);

    update();
}

// Destruktor
MainWindow::~MainWindow()
{
    delete ui;
}

// Metoda odpowiadająca za zmianę rozmiarów widgetów (z myślą o tym, że program może być odpalany na ekranach o różnych rozdziałkach)
// Do zapewnienia responsywności aplikacji
void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    // Pobieramy nowe wymiary okna
    int newWidth = this->width;
    int newHeight = this->height;

    // Rozdzielczość monitora
    int monitorWidth = resolution.first;
    int monitorHeight = resolution.second;

    // Ustawienie widgetów
    resizeWidget("screen", monitorWidth, monitorHeight, 0.7, 0.9, 10, 10);
    centerWidgetInFrame("progressBar", "screen", 0.2, 0.1);
    centerWidgetInFrame("loadingScreen", "screen", 0.2, 0.1);
    resizeLayout("rightSideLayout", monitorWidth, monitorHeight, 0.25, 0.9, 10 + static_cast<int>(monitorWidth * 0.7) + 20, 10);
}

void MainWindow::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    // Uzyskujemy rozmiar ramki
    QSize frameSize = ui->screen->size();

    // Uzyskujemy rozmiar obrazka
    QSize imageSize = im->size();

    // Jeśli obrazek jest większy niż ramka, obcinamy go
    if (imageSize.width() > frameSize.width() || imageSize.height() > frameSize.height()) {
        // Obliczamy obszar do rysowania, by dopasować obrazek do ramki
        QRect targetRect = QRect(ui->screen->pos(), frameSize);
        p.drawImage(targetRect, *im, QRect(0, 0, frameSize.width(), frameSize.height()));
    }
    else {
        // Jeśli obrazek jest mniejszy niż ramka, przeskalowujemy go, aby wypełnił ramkę
        QImage scaledImage = im->scaled(frameSize, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
        p.drawImage(ui->screen->pos(), scaledImage);
    }

    update();
}

// Metoda rysująca piksel przyjmująca jako parametr jego współrzędne, wartości RGB oraz obraz na którym się pojawi
void MainWindow::drawPixel(int x, int y, int r, int g, int b, QImage *im)
{
    // Pozwalamy rysować piksele, ale tylko w ramach określonego obszaru - dla maksymalnych rozmiarów obrazu
    if(x < im->width() && y < im->height() && x >= 0 && y >= 0)
    {
        uchar *pix = im->scanLine(y);
        pix[4 * x] = b;
        pix[4 * x + 1] = g;
        pix[4 * x + 2] = r;
        pix[4 * x + 3] = 255;
    }
    update();
}

// Metoda czyszcząca obraz
void MainWindow::clearImage(QImage *im)
{
    uchar *pix = im->bits();

    for(int i = 0; i < im->height(); i++)
    {
        for(int j = 0; j < im->width(); j++)
        {
            pix[4 * im->width() * i + 4 * j] = 0;
            pix[4 * im->width() * i + 4 * j + 1] = 0;
            pix[4 * im->width() * i + 4 * j + 2] = 0;
        }
    }
}

// Metoda zwracająca poziomy i pionowy rozmiar pulpitu niezależnie od systemu operacyjnego
std::pair<int, int> MainWindow::getDesktopResolution(){

    // Tworzymy uchwyt pod ekran główny
    QScreen *screen = QGuiApplication::primaryScreen();

    // Dobieramy się do wymiarów ekranu
    QRect  screenGeometry = screen->geometry();

     // Przypisujemy szerokość i wysokość ekranu do zmiennych
    int W = screenGeometry.width();
    int H = screenGeometry.height();

    return std::make_pair(W, H);
}

// Dodanie czcionek do aplikacji
void MainWindow::addFonts(){
    // https://fonts.google.com/selection?query=Roboto - link do czcionki

    // Dodanie czcionek Roboto z zasobów
    thinRobotoId = QFontDatabase::addApplicationFont(":/resources/Fonts/Roboto-Thin.ttf");
    regularRobotoId = QFontDatabase::addApplicationFont(":/resources/Fonts/Roboto-Regular.ttf");
    boldRobotoId = QFontDatabase::addApplicationFont(":/resources/Fonts/Roboto-Bold.ttf");

    // Sprawdzenie, czy czcionki zostały załadowane poprawnie
    if (thinRobotoId == -1 || regularRobotoId == -1 || boldRobotoId == -1) {
        qDebug() <<"Nie udało się załadować jednej lub więcej czcionek.";
        console->appendPlainText("Nie udało się załadować jednej lub więcej czcionek.");
    }
}

// Edytuje wybrane elementy GUI, które nie podlegają edycji w Qt Designerze
void MainWindow::editGui(){

    // Ustalamy maksymalną liczbę wątków (domyślne ustawienie)
    setMaxThreadsValue();

    // Dodanie czcionek
    addFonts();

    // Ukrycie progressbara i komunikatu informującego o wykonywanych obliczeniach
    ui->progressBar->setVisible(false);
    ui->loadingScreen->setVisible(false);

    // Roboto, wielkość 18 i 11
    QFont regularRoboto_18 = getFont(18, Regular);
    QFont regularRoboto_11 = getFont(11, Regular);

    // Kolekcja guzików interfejsu aplikacji
    QList<QPushButton*> buttons = centralWidget()->findChildren<QPushButton*>();

    // Dla każdego guzika zmieniamy czcionkę tekstu
    for (QPushButton* button : buttons) {
        button->setFont(regularRoboto_18);
    }

    // Kolekcja labelów
    QList<QLabel*> labels = centralWidget()->findChildren<QLabel*>();

    // Dla każdego labela zmieniamy czcionkę tekstu
    for(QLabel* label : labels){
        label->setFont(regularRoboto_11);
    }

    // Znajdujemy widget konsoli
    console = ui->plainTextEdit;

    // Konfigurujemy widget konsoli
    console->setReadOnly(true); // Ustawiamy go na "tylko do odczytu"
    QFont font;
    font.setFamily("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(12);
    console->setFont(font);

    // Dodajemy na początku informację o jej uruchomieniu tekst
    console->appendPlainText("Konsola została uruchomiona...");

    // Ustawienie wartości minimalnych spinboxów zależne od szerokości i wysokości ramki
    ui->spinBox_4->setMinimum(ui->screen->width());
    ui->spinBox_3->setMinimum(ui->screen->height());
}

// Metoda zmieniająca wielkość widgeta przy zmianie rozdziałki ekranu komputera
void MainWindow::resizeWidget(const QString& widgetName, int monitorWidth, int monitorHeight, double widthRatio, double heightRatio, int posX, int posY)
{
    if (QWidget* widget = this->findChild<QWidget*>(widgetName)) {
        int widgetWidth = static_cast<int>(monitorWidth * widthRatio);
        int widgetHeight = static_cast<int>(monitorHeight * heightRatio);
        widget->setGeometry(posX, posY, widgetWidth, widgetHeight);
    }
}

// Metoda zmieniająca wielkość layoutu przy zmianie rozdziałki ekranu komputera
void MainWindow::resizeLayout(const QString& layoutName, int monitorWidth, int monitorHeight, double widthRatio, double heightRatio, int posX, int posY)
{
    if (QLayout* layout = this->findChild<QLayout*>(layoutName)) {
        int layoutWidth = static_cast<int>(monitorWidth * widthRatio);
        int layoutHeight = static_cast<int>(monitorHeight * heightRatio);

        if (QWidget* layoutWidget = layout->parentWidget()) {
            layoutWidget->setGeometry(posX, posY, layoutWidth, layoutHeight);
        }
    }
}

// Metoda centrująca widget w stosunku do ramki obrazu
void MainWindow::centerWidgetInFrame(const QString& widgetName, const QString& frameName, double widthRatio, double heightRatio)
{
    if (QWidget* widget = this->findChild<QWidget*>(widgetName)) {
        if (QFrame* frame = this->findChild<QFrame*>(frameName)) {
            int frameWidth = frame->width();
            int frameHeight = frame->height();
            int widgetWidth = static_cast<int>(frameWidth * widthRatio);
            int widgetHeight = static_cast<int>(frameHeight * heightRatio);
            int posX = frame->x() + (frameWidth - widgetWidth) / 2;
            int posY = frame->y() + (frameHeight - widgetHeight) / 2;
            widget->setGeometry(posX, posY, widgetWidth, widgetHeight);
        }
    }
}

// Ustala maksymalną liczbę wątków, na których może być wykonywany program
void MainWindow::setMaxThreadsValue(){
    // Wprowadzenie blokady od strony hardware'u jeśli chodzi o maksymalną ilość wątków na którym wykonywany będzie program
    int max_threads = std::thread::hardware_concurrency();
    ui->spinBox->setMaximum(max_threads);
    ui->spinBox->setValue(max_threads);
}

// Ustawia domyślne parametry dla zbioru Julii
void MainWindow::setDefaultParameters(){

    // Oto domyślne parametry dla jakich uruchamia się program
    num_threads = std::thread::hardware_concurrency();
    N = 100; // liczba iteracji
    c_re = -0.7; // część rzeczywista parametru C
    c_im = 0.27105; // część urojona parametru C
    x_min = -1.5; // lewy brzeg prostokąta
    x_max = 1.5; // prawy brzeg prostokąta
    y_min = -1.5; // dolny brzeg prostokąta
    y_max = 1.5; //górny brzeg prostokąta
}

// Resetuje ustawienie wszystkich parametrów do parametrów domyślnych
void MainWindow::resetToDefaultParameters(){
    setMaxThreadsValue();
    setDefaultParameters();
    restoreDefaultSpinBoxValues();
}

// Metoda zapisująca defaultowe wartości dla spinboxów
void MainWindow::saveDefaultSpinBoxValues() {
    // Zapisanie wartości domyślnych dla QSpinBox
    QList<QSpinBox*> spinBoxes = this->findChildren<QSpinBox*>();
    for (QSpinBox* spinBox : spinBoxes) {
        defaultSpinBoxValues[spinBox] = spinBox->value();
    }

    // Zapisanie wartości domyślnych dla QDoubleSpinBox
    QList<QDoubleSpinBox*> doubleSpinBoxes = this->findChildren<QDoubleSpinBox*>();
    for (QDoubleSpinBox* doubleSpinBox : doubleSpinBoxes) {
        defaultDoubleSpinBoxValues[doubleSpinBox] = doubleSpinBox->value();
    }

    // Zapisanie domyślnej wartości indeksu comboboxa
    defaultComboBoxIndex = ui->comboBox->currentIndex();
}

// Metoda przywracająca defaultowe wartości dla spinboxów
void MainWindow::restoreDefaultSpinBoxValues() {

    // Przywrócenie wartości domyślnych dla QSpinBox
    for (QSpinBox* spinBox : defaultSpinBoxValues.keys()) {
        spinBox->setValue(defaultSpinBoxValues[spinBox]);
    }

    // Przywrócenie wartości domyślnych dla QDoubleSpinBox
    for (QDoubleSpinBox* doubleSpinBox : defaultDoubleSpinBoxValues.keys()) {
        doubleSpinBox->setValue(defaultDoubleSpinBoxValues[doubleSpinBox]);
    }

    // Ustawienie największej liczby wątków
    setMaxThreadsValue();

    // Ustawienie defaultowej opcji w comboboxie
    ui->comboBox->setCurrentIndex(defaultComboBoxIndex);
}

// Metoda pobierająca odpowiednią czcionkę
QFont MainWindow::getFont(int fontSize, Thickness thickness){

    QStringList selectedFamilies;

    switch (thickness) {
    case Thin:
        selectedFamilies = QFontDatabase::applicationFontFamilies(thinRobotoId);
        break;
    case Regular:
        selectedFamilies = QFontDatabase::applicationFontFamilies(regularRobotoId);
        break;
    case Bold:
        selectedFamilies = QFontDatabase::applicationFontFamilies(boldRobotoId);
        break;
    default:
        break;
    }

    // Pobranie rodziny czcionki dla regular Roboto
    if (!selectedFamilies.isEmpty()) {
        QString robotoFamily = selectedFamilies.at(0);

        // Ustawienie czcionki Roboto w elemencie GUI
        QFont monospace(robotoFamily);
        monospace.setPointSize(fontSize); // Ustawienie rozmiaru czcionki

        return monospace;
    } else {
        qDebug() << "Nie znaleziono szukanej rodziny czcionek.";
        console->appendPlainText("Nie znaleziono szukanej rodziny czcionek.");
        return QFont("Arial");
    }
}

// Metoda wykorzystująca skalę szarości
QColor MainWindow::iterToColor(int n, int N) {
    int intensity = static_cast<int>(255.0 * n / N); // Skala od 0 (czarny) do 255 (biały)
    return QColor(intensity, intensity, intensity); // Skala szarości
}

// Metoda wykorzystująca skalę kolorów HSV
QColor MainWindow::iterToColor2(int n, int N) {
    if (n >= N) {
        return QColor(Qt::black); // Punkt należy do zbioru (czarny kolor)
    }

    double t = static_cast<double>(n) / N; // Normalizacja iteracji do zakresu 0.0 - 1.0
    int hue = static_cast<int>(360 * t);  // Mapowanie do zakresu Hue (0-360 stopni)
    return QColor::fromHsv(hue, 255, 255); // Pełna saturacja i jasność
}

// Metoda wykorzystująca gładki gradient kolorów
QColor MainWindow::iterToColor3(int n, int N) {
    if (n >= N) {
        return QColor(Qt::black); // Punkt należy do zbioru (kolor czarny)
    }

    double t = static_cast<double>(n) / N; // Normalizacja liczby iteracji do zakresu 0.0 - 1.0

    // Generowanie gładkiego gradientu: czerwony, zielony, niebieski
    int red = static_cast<int>(9 * (1 - t) * t * t * t * 255);
    int green = static_cast<int>(15 * (1 - t) * (1 - t) * t * t * 255);
    int blue = static_cast<int>(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);

    return QColor(red, green, blue); // Zwracamy interpolowany kolor
}

// Metoda wykorzystująca interpolowaną mapę kolorów
QColor MainWindow::iterToColor4(int n, int N) {

    // Sprawdzamy, czy mapa kolorów ma co najmniej dwa kolory
    if (activeColormap->size() < 2) {
        return (*activeColormap)[0]; // Zwracamy pierwszy kolor, jeśli mapa jest zbyt mała
    }

    // Normalizujemy indeks n do zakresu [0, 1]
    float t = static_cast<float>(n) / static_cast<float>(N - 1);

    // Obliczamy, na jakiej pozycji w mapie kolorów jesteśmy
    float index = t * (activeColormap->size() - 1);
    int idx1 = static_cast<int>(index); // Indeks początkowy
    int idx2 = std::min(idx1 + 1, static_cast<int>(activeColormap->size()) - 1); // Indeks końcowy

    // Obliczamy wagę interpolacji (czyli procent, jak daleko jesteśmy pomiędzy dwoma kolorami)
    float weight = index - idx1;

    // Pobieramy kolory z mapy
    QColor color1 = (*activeColormap)[idx1];
    QColor color2 = (*activeColormap)[idx2];

    // Interpolujemy każdy kanał (R, G, B)
    int r = static_cast<int>(color1.red() + weight * (color2.red() - color1.red()));
    int g = static_cast<int>(color1.green() + weight * (color2.green() - color1.green()));
    int b = static_cast<int>(color1.blue() + weight * (color2.blue() - color1.blue()));

    // Zwracamy interpolowany kolor
    return QColor(r, g, b);
}

// Funkcja do wywołania wskaźnika do funkcji członkowskiej
QColor MainWindow::callActiveFunction(int n, int N) {
    return (this->*activeFunction)(n, N);
}

// Funkcja sprawdzająca, czy dany punkt należy do zbioru Julii
int MainWindow::check(std::complex<long double> z, std::complex<long double> c, int N){
    for (int n = 0; n <= N; ++n) {
        if (abs(z) >= 2.0) {
            return n; // Punkt wychodzi poza zbiór Julii
        }
        z = z * z + c;
    }
    return N; // Punkt należy do zbioru Julii
}

// Metoda odpowiedzialna za rysowanie obrazu zbioru Julii
void MainWindow::computeJulia(int start, int end, std::complex<long double> c, int N, QImage *im) {

    // Pobieramy wskaźnik do danych pikseli obrazu
    uchar* pixels = im->bits();
    int imageWidth = im->width();

    for (int y = start; y < end; ++y) {
        for (int x = 0; x < imageWidth; ++x) {
            // Mapowanie współrzędnych pikseli na układ zespolony
            long double real = ((x_max - x_min) * x / (imageWidth - 1) + x_min);
            long double imag = ((y_max - y_min) * (height - 1 - y) / (height - 1) + y_min);
            std::complex<long double> z(real, imag);

            // Sprawdzamy, czy punkt należy do zbioru Julii
            int n = check(z, c, N);
            QColor color = callActiveFunction(n, N); // Zmienna pod kolor piksela

            // Kolorujemy piksel danym kolorem
            drawPixel(x, y, color.red(), color.green(), color.blue(), im);
        }
    }

    update();
}

// Zdarzenie po kliknięciu myszki
void MainWindow::mousePressEvent(QMouseEvent *e)
{
    // Pobieramy współrzędne kursora
    int x = e->pos().x();
    int y = e->pos().y();

    // Pozwalamy przeciągać obrazek wtedy kiedy kursor znajduje się w polu obrazka
    if(x < im->width() && y < im->height() && x >= 0 && y >= 0)
    {
        // Jeśli kliknięty został lewy przycisk myszy, to można przesuwać obrazek oraz przypisywana
        // jest do zmiennej ostatnia pozycja kursora
        if (e->button() == Qt::LeftButton) {
            lastMousePosition = e->pos();
            isDragging = true;
        }
    }
}

// Zdarzenie w czasie przesuwania myszki
void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    // Pobieramy współrzędne kursora
    int x = e->pos().x();
    int y = e->pos().y();

    // Pozwalamy przesuwać obrazek wtedy kiedy kursor znajduje się w polu obrazka
    if(x < im->width() && y < im->height() && x >= 0 && y >= 0)
    {
        // Sprawdzamy, czy przesuwanie jest aktywne
        if (isDragging) {
            // Obliczamy różnicę między aktualną a poprzednią pozycją kursora myszki
            QPoint delta = e->pos() - lastMousePosition;

            // Aktualizujemy pozycję kursora
            lastMousePosition = e->pos();

            // Przeliczamy przesunięcie kursora na przesunięcie w układzie współrzędnych
            double dx = delta.x() * (x_max - x_min) / width;
            double dy = delta.y() * (y_max - y_min) / height;

            // Przesuwamy zakres współrzędnych kolejno w poziomie i w pionie
            x_min -= dx;
            x_max -= dx;
            y_min += dy;
            y_max += dy;

            // Generujemy obraz na nowo
            on_generateImagePushButton_clicked();
        }
    }
}

// Zdarzenie po puszczeniu myszki
void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    // Pobieramy współrzędne kursora
    int x = e->pos().x();
    int y = e->pos().y();

    // Pozwalamy puścić przesuwany obrazek wtedy kiedy kursor znajduje się w polu obrazka
    if(x < im->width() && y < im->height() && x >= 0 && y >= 0)
    {
        // Jeśli puścimy lewy przycisk myszy, to przestaniemy przesuwać obrazek
        if (e->button() == Qt::LeftButton) {
            isDragging = false;
        }
    }
}

// Zdarzenie po scrollowaniu myszką
void MainWindow::wheelEvent(QWheelEvent *e)
{
    // Pobieramy pozycję kursora
    QPointF cursorPos = e->position();

    // Pozwalamy przybliżać obrazek wtedy kiedy kursor znajduje się w polu obrazka
    if(cursorPos.x() < im->width() && cursorPos.y() < im->height() && cursorPos.x() >= 0 && cursorPos.y() >= 0)
    {
        // Przybliżamy (zoom in) bądź oddalamy (zoom out) - ustawiamy zoomFactor
        double zoomFactor = (e->angleDelta().y() > 0) ? 0.95 : 1.05;

        // Ustalamy pozycję kursora w układzie współrzędnych dla osi X i Y
        double xCursor = x_min + (cursorPos.x() / width) * (x_max - x_min);
        double yCursor = y_max - (cursorPos.y() / height) * (y_max - y_min);

        // Obliczamy nowy zakres dla osi X i Y
        double xRange = (x_max - x_min) * zoomFactor;
        double yRange = (y_max - y_min) * zoomFactor;

        // Centrujemy zakres współrzędnych na pozycji kursora
        x_min = xCursor - xRange / 2.0;
        x_max = xCursor + xRange / 2.0;
        y_min = yCursor - yRange / 2.0;
        y_max = yCursor + yRange / 2.0;

        // Generujemy obraz na nowo
        on_generateImagePushButton_clicked();
    }
}

// Metoda obsługująca test silnej skalowalności
void MainWindow::runStrongScalingTest() {
    int defaultThreads = std::thread::hardware_concurrency(); // Domyślna liczba wątków
    QVector<int> threadCounts;

    // Tworzymy listę potęg dwójki mniejszych lub równych domyślnej liczbie wątków
    while(defaultThreads > 0){
        // Przejście do poprzedniej potęgi dwójki
        threadCounts.append(defaultThreads);
        while( ( (--defaultThreads) & (defaultThreads - 1) ) != 0 );
    }

    qDebug() << "Rozpoczynam test silnej skalowalności:";
    qDebug() << "Liczba wątków: " << threadCounts;
    console->appendPlainText("Rozpoczynam test silnej skalowalności:");
    console->appendPlainText("Liczba wątków: ");

    // Zmienna typu QString pod liczbę wątków, aby móc przedstawić je wyświetlić w widgecie konsoli
    QString values = "";

    for(int k = 0; k < threadCounts.count(); ++k){
        values += QString::number(threadCounts.value(k));
        if(k != threadCounts.count() - 1){
            values += ", ";
        }
    }

    // Wypisujemy ilości wątków biorących udział w teście do konsoli
    console->appendPlainText(values);

    // Wyniki testu: Liczba wątków -> Czas wykonania
    QMap<int, qint64> results;

    // Zmienna iteracyjna do śledzenia ilości przebiegów pętli for
    int iterator = 0;

    // Odsłaniamy pasek ładowania i ustawiamy jego wartość początkową
    ui->progressBar->setVisible(true);
    ui->progressBar->setValue(0);

    for (int numThreads : threadCounts) {
        num_threads = numThreads; // Ustawiamy liczbę wątków

        QElapsedTimer timer;
        timer.start(); // Startujemy pomiar czasu

        // Generujemy obraz z aktualną liczbą wątków
        std::complex<long double> c(c_re, c_im);
        std::vector<std::thread> threads;
        int block_size = height / num_threads;

        for (int i = 0; i < num_threads; ++i) {
            int start = i * block_size;
            int end = (i == num_threads - 1) ? height : start + block_size;
            threads.emplace_back(&MainWindow::computeJulia, this, start, end, c, N, im);
        }

        for (auto &t : threads) {
            t.join();
        }

        // Zapisujemy czas wykonania
        qint64 elapsed = timer.elapsed(); // Czas podany w milisekundach
        results[numThreads] = elapsed;
        qDebug() << "Liczba wątków:" << numThreads << ", Czas wykonania:" << elapsed << "ms";
        console->appendPlainText("Liczba wątków: " + QString::number(numThreads) + ", Czas wykonania: " + QString::number(elapsed) + "ms");

        // Inkrementacja iteratora oraz aktualizacja wartości na progressbarze
        iterator++;
        ui->progressBar->setValue(static_cast<int>((static_cast<double>(iterator) / threadCounts.size()) * 100));
    }

    // Wyświetlamy wyniki
    qDebug() << "Test został zakończony. Wyniki:";
    for (auto it = results.begin(); it != results.end(); ++it) {
        qDebug() << "Liczba wątków:" << it.key() << " -> Czas:" << it.value() << "ms";
    }

    // Ukrywamy znowu progressbar
    ui->progressBar->setVisible(false);
}

// Zdarzenie po którym następuje generowanie obrazu zbioru Julii
void MainWindow::on_generateImagePushButton_clicked() {

    // Ustawienie parametru C
    std::complex<long double> c(c_re, c_im);

    // Kolekcja wątków do wykonywania zadań
    std::vector<std::thread> threads;

    // Zmienna pod liczbę bloków;
    // Każdy wątek będzie kolorował swój blok pikseli
    int block_size = width / num_threads;

    // Rezerwujemy odpowiednią ilość elementów (wątków) pod wektor do ich trzymania
    threads.reserve(num_threads);

    // Wyświetlamy powiadomienie o wykonywanych obliczeniach
    ui->loadingScreen->setVisible(true);

    // Upewniamy się, że interfejs użytkownika jest aktualizowany
    QCoreApplication::processEvents();

    // Start pomiaru czasu
    auto start_time = std::chrono::high_resolution_clock::now();

    // Iterujemy po wszystkich wątkach
    for (int i = 0; i < num_threads; ++i) {
        // Każdy wątek otrzymuje swój blok, dlatego liczymy początek i koniec bloku dla którego obliczenia dokonuje dany wątek
        int start = i * block_size;
        int end = (i == num_threads - 1) ? width : start + block_size;
        threads.emplace_back(&MainWindow::computeJulia, this, start, end, c, N, im);
    }

    for (auto& t : threads) {
        t.join();
    }

    // Koniec pomiaru czasu
    auto end_time = std::chrono::high_resolution_clock::now();

    // Oblicz czas wykonania
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    qDebug() << "Wątki wykonały pracę w czasie:" << duration << "ms";
    console->appendPlainText("Wątki wykonały pracę w czasie: " + QString::number(duration) + "ms");

    // Ukrywamy informacje o obliczeniach
    ui->loadingScreen->setVisible(false);

    update();
}

// Zdarzenie wywołane po naciśnięciu przycisku do czyszczenia obrazu
void MainWindow::on_clearImagePushButton_clicked()
{   // W przypadku, gdy obraz nie jest pusty - usuwamy
    if(im->bits()){
        clearImage(im);
        qDebug() << "Wyczyszczono obraz";
        console->appendPlainText("Wyczyszczono obraz");
    }else{
        // W przypadku braku pikseli obrazu
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowTitle("Czyszczenie obrazka");
        msgBox.setText("Aby wyczyścić obrazek, najpierw musisz go wygenerować.");

        // Zmieniamy rozmiar okna dialogowego
        msgBox.setFixedSize(300, 150);  // Ustawiamy stały rozmiar okna dialogowego

        msgBox.exec();
    }
}

// Zdarzenie wywołanie po naciśnięciu przycisku do zresetowania wszystkich wartości
void MainWindow::on_resetValuesPushButton_clicked()
{
    resetToDefaultParameters();
}

// Zmiana liczby wątków
void MainWindow::on_spinBox_valueChanged(int arg1)
{
    num_threads = ui->spinBox->value();
}

// Zmiana szerokości
void MainWindow::on_spinBox_4_valueChanged(int arg1)
{
    width = ui->spinBox_4->value();
}

// Zmiana wysokości
void MainWindow::on_spinBox_3_valueChanged(int arg1)
{
    height = ui->spinBox_3->value();
}

// Zmiana liczby iteracji
void MainWindow::on_spinBox_2_valueChanged(int arg1)
{
    N = ui->spinBox_2->value();
}

// Zmiana x_min
void MainWindow::on_doubleSpinBox_3_valueChanged(double arg1)
{
    x_min = ui->doubleSpinBox_3->value();
}

// Zmiana x_max
void MainWindow::on_doubleSpinBox_6_valueChanged(double arg1)
{
    x_max = ui->doubleSpinBox_6->value();
}

// Zmiana y_min
void MainWindow::on_doubleSpinBox_4_valueChanged(double arg1)
{
    y_min = ui->doubleSpinBox_4->value();
}

// Zmiana y_max
void MainWindow::on_doubleSpinBox_5_valueChanged(double arg1)
{
    y_max = ui->doubleSpinBox_5->value();
}

// Zmiana części rzeczywistej liczby zespolonej
void MainWindow::on_doubleSpinBox_2_valueChanged(double arg1)
{
    c_re = ui->doubleSpinBox_2->value();
}

// Zmiana części urojonej liczby zespolonej
void MainWindow::on_doubleSpinBox_valueChanged(double arg1)
{
    c_im = ui->doubleSpinBox->value();
}

// Wybór odpowiedniej palety kolorów
void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    if (index >= 0 && index <= 2) {
        // Przypisanie odpowiedniej funkcji dla indeksów 0-2
        switch (index) {
        case 0:
            activeFunction = &MainWindow::iterToColor;
            break;
        case 1:
            activeFunction = &MainWindow::iterToColor2;
            break;
        case 2:
            activeFunction = &MainWindow::iterToColor3;
            break;
        }
    }
    else if (index >= 3 && index <= 8) {
        // Dla indeksów 3-8 ustawiamy tę samą funkcję, ale różne mapy kolorów
        activeFunction = &MainWindow::iterToColor4;

        static const std::array<QString, 6> colormapKeys = {
            "colormap1", "colormap2", "colormap3",
            "colormap4", "colormap5", "colormap6"
        };

        int colormapIndex = index - 3;
        activeColormap = &colormaps[colormapKeys[colormapIndex]];
    }
    else {
        // Domyślna akcja dla nieistniejących indeksów
        qDebug() << "Nie ma takiej palety kolorów\n";
        console->appendPlainText("Nie ma takiej palety kolorów\n");
        activeFunction = &MainWindow::iterToColor;
    }
}

// Uruchamianie testów silnej skalowalności
void MainWindow::on_strongScalingTestPushButton_clicked()
{
    runStrongScalingTest();
}



