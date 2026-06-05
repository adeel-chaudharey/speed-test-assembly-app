#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTimer>
#include <QElapsedTimer>
#include <QTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFrame>
#include <QProcess>
#include <QFileInfo>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartTest();
    void onSubmit();
    void updateTimer();

private:
    // ── UI widgets ──────────────────────────────────────────
    QLabel          *lblTitle;
    QLabel          *lblSentenceHeader;
    QLabel          *lblSentence;
    QLabel          *lblInputHeader;
    QPlainTextEdit  *txtInput;
    QPushButton     *btnStart;
    QPushButton     *btnSubmit;
    QGroupBox       *grpResults;
    QLabel          *lblResultCorrect;
    QLabel          *lblResultIncorrect;
    QLabel          *lblResultAccuracy;
    QLabel          *lblResultWPM;
    QLabel          *lblResultTime;
    QLabel          *lblLiveTimer;

    // ── Timer ────────────────────────────────────────────────
    QTimer          *displayTimer;
    QElapsedTimer    elapsedTimer;
    bool             testRunning;

    // ── Helpers ──────────────────────────────────────────────
    void setupUI();
    void applyStyles();
    void showResults(int correctChars, int incorrectChars,
                     int totalChars, int wordCount, double elapsedSec);
    bool runAssembly(const QString &original,
                     const QString &typed,
                     int &correctOut, int &incorrectOut,
                     int &totalOut,   int &wordOut);
    QString locateAssemblyExe() const;
    QString currentSentence;
};

#endif // MAINWINDOW_H
