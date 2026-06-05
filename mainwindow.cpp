#include "mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QProcess>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QDebug>
#include <cmath>

// ── Sample sentences ─────────────────────────────────────────
static const QStringList SENTENCES = {
    "Assembly language is powerful and fast",
    "The quick brown fox jumps over the lazy dog",
    "Programming requires patience and practice",
    "Qt is a cross platform application framework",
    "NASM assembler produces efficient machine code",
    "Typing speed improves with daily practice",
    "Software engineering is a creative discipline",
    "Computers process billions of instructions per second"
};

// ════════════════════════════════════════════════════════════
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , testRunning(false)
{
    setupUI();
    applyStyles();

    displayTimer = new QTimer(this);
    connect(displayTimer, &QTimer::timeout, this, &MainWindow::updateTimer);
}

MainWindow::~MainWindow() {}

// ════════════════════════════════════════════════════════════
//  UI Construction
// ════════════════════════════════════════════════════════════
void MainWindow::setupUI()
{
    setWindowTitle("Typing Speed Test");
    setMinimumSize(700, 600);
    resize(750, 660);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(30, 24, 30, 24);

    // ── Title ────────────────────────────────────────────────
    lblTitle = new QLabel("⌨  Typing Speed Test", this);
    lblTitle->setObjectName("lblTitle");
    lblTitle->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(lblTitle);

    // ── Live timer ───────────────────────────────────────────
    lblLiveTimer = new QLabel("00:00", this);
    lblLiveTimer->setObjectName("lblLiveTimer");
    lblLiveTimer->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(lblLiveTimer);

    // ── Sentence display ─────────────────────────────────────
    lblSentenceHeader = new QLabel("Sentence to type:", this);
    lblSentenceHeader->setObjectName("sectionHeader");
    mainLayout->addWidget(lblSentenceHeader);

    lblSentence = new QLabel("Click  Start Test  to begin.", this);
    lblSentence->setObjectName("lblSentence");
    lblSentence->setWordWrap(true);
    lblSentence->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lblSentence->setMinimumHeight(56);
    mainLayout->addWidget(lblSentence);

    // ── Text input ───────────────────────────────────────────
    lblInputHeader = new QLabel("Your input:", this);
    lblInputHeader->setObjectName("sectionHeader");
    mainLayout->addWidget(lblInputHeader);

    txtInput = new QPlainTextEdit(this);
    txtInput->setObjectName("txtInput");
    txtInput->setPlaceholderText("Start the test first, then type here…");
    txtInput->setEnabled(false);
    txtInput->setMaximumHeight(100);
    mainLayout->addWidget(txtInput);

    // ── Buttons ──────────────────────────────────────────────
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(16);

    btnStart = new QPushButton("▶  Start Test", this);
    btnStart->setObjectName("btnStart");
    btnStart->setMinimumHeight(44);
    btnStart->setCursor(Qt::PointingHandCursor);

    btnSubmit = new QPushButton("✔  Submit", this);
    btnSubmit->setObjectName("btnSubmit");
    btnSubmit->setMinimumHeight(44);
    btnSubmit->setEnabled(false);
    btnSubmit->setCursor(Qt::PointingHandCursor);

    btnLayout->addWidget(btnStart);
    btnLayout->addWidget(btnSubmit);
    mainLayout->addLayout(btnLayout);

    // ── Results group ────────────────────────────────────────
    grpResults = new QGroupBox("Results", this);
    grpResults->setObjectName("grpResults");

    QVBoxLayout *resLayout = new QVBoxLayout(grpResults);
    resLayout->setSpacing(8);

    auto makeResultRow = [&](const QString &label, QLabel *&valueLabel)
    {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *lbl = new QLabel(label, this);
        lbl->setObjectName("resLabel");
        valueLabel = new QLabel("—", this);
        valueLabel->setObjectName("resValue");
        row->addWidget(lbl);
        row->addStretch();
        row->addWidget(valueLabel);
        resLayout->addLayout(row);
    };

    makeResultRow("Correct Characters:",   lblResultCorrect);
    makeResultRow("Incorrect Characters:", lblResultIncorrect);
    makeResultRow("Accuracy:",              lblResultAccuracy);
    makeResultRow("WPM:",                   lblResultWPM);
    makeResultRow("Time Taken:",           lblResultTime);

    mainLayout->addWidget(grpResults);
    mainLayout->addStretch();

    // ── Connect buttons ──────────────────────────────────────
    connect(btnStart,  &QPushButton::clicked, this, &MainWindow::onStartTest);
    connect(btnSubmit, &QPushButton::clicked, this, &MainWindow::onSubmit);
}

// ════════════════════════════════════════════════════════════
//  Stylesheet
// ════════════════════════════════════════════════════════════
void MainWindow::applyStyles()
{
    setStyleSheet(R"(
        QMainWindow, QWidget {
            background-color: #1e1e2e;
            color: #cdd6f4;
            font-family: 'Segoe UI', sans-serif;
            font-size: 14px;
        }

        #lblTitle {
            font-size: 26px;
            font-weight: bold;
            color: #cba6f7;
            padding: 8px 0;
        }

        #lblLiveTimer {
            font-size: 32px;
            font-weight: bold;
            color: #89dceb;
            letter-spacing: 4px;
        }

        #sectionHeader {
            font-size: 12px;
            font-weight: bold;
            color: #a6adc8;
            text-transform: uppercase;
            letter-spacing: 1px;
        }

        #lblSentence {
            background-color: #313244;
            border: 1px solid #45475a;
            border-radius: 8px;
            padding: 12px 16px;
            font-size: 16px;
            color: #f5c2e7;
            font-weight: 500;
        }

        #txtInput {
            background-color: #313244;
            border: 2px solid #45475a;
            border-radius: 8px;
            padding: 10px;
            font-size: 15px;
            color: #cdd6f4;
            selection-background-color: #7f849c;
        }

        #txtInput:focus {
            border: 2px solid #cba6f7;
        }

        #txtInput:disabled {
            background-color: #181825;
            color: #585b70;
        }

        #btnStart {
            background-color: #a6e3a1;
            color: #1e1e2e;
            font-size: 15px;
            font-weight: bold;
            border: none;
            border-radius: 8px;
            padding: 8px 20px;
        }

        #btnStart:hover {
            background-color: #94e2d5;
        }

        #btnStart:pressed {
            background-color: #74c7ec;
        }

        #btnSubmit {
            background-color: #89b4fa;
            color: #1e1e2e;
            font-size: 15px;
            font-weight: bold;
            border: none;
            border-radius: 8px;
            padding: 8px 20px;
        }

        #btnSubmit:hover {
            background-color: #b4befe;
        }

        #btnSubmit:pressed {
            background-color: #cba6f7;
        }

        #btnSubmit:disabled, #btnStart:disabled {
            background-color: #313244;
            color: #585b70;
        }

        #grpResults {
            background-color: #181825;
            border: 1px solid #45475a;
            border-radius: 10px;
            font-size: 13px;
            color: #a6adc8;
            padding-top: 4px;
        }

        #grpResults::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            color: #cba6f7;
            font-weight: bold;
            font-size: 13px;
        }

        #resLabel {
            color: #a6adc8;
            font-size: 14px;
        }

        #resValue {
            color: #cdd6f4;
            font-size: 15px;
            font-weight: bold;
        }
    )");
}

// ════════════════════════════════════════════════════════════
//  Slot: Start Test
// ════════════════════════════════════════════════════════════
void MainWindow::onStartTest()
{
    // Pick a random sentence
    int idx = QDateTime::currentMSecsSinceEpoch() % SENTENCES.size();
    currentSentence = SENTENCES[idx];

    lblSentence->setText(currentSentence);
    txtInput->setPlainText("");
    txtInput->setEnabled(true);
    txtInput->setFocus();

    // Reset result labels
    lblResultCorrect->setText("—");
    lblResultIncorrect->setText("—");
    lblResultAccuracy->setText("—");
    lblResultWPM->setText("—");
    lblResultTime->setText("—");

    // Start timers
    elapsedTimer.start();
    displayTimer->start(200);
    testRunning = true;

    btnStart->setEnabled(false);
    btnSubmit->setEnabled(true);
    lblLiveTimer->setText("00:00");
}

// ════════════════════════════════════════════════════════════
//  Slot: Submit
// ════════════════════════════════════════════════════════════
void MainWindow::onSubmit()
{
    if (!testRunning) return;

    // Stop timing
    qint64 elapsed_ms = elapsedTimer.elapsed();
    displayTimer->stop();
    testRunning = false;
    double elapsedSec = elapsed_ms / 1000.0;

    QString typed = txtInput->toPlainText().trimmed();
    if (typed.isEmpty()) {
        QMessageBox::warning(this, "Empty Input",
                             "Please type something before submitting.");
        displayTimer->start(200);
        testRunning = true;
        return;
    }

    txtInput->setEnabled(false);
    btnSubmit->setEnabled(false);
    btnStart->setEnabled(true);

    // Update live timer one final time
    int secs = static_cast<int>(elapsedSec);
    lblLiveTimer->setText(QString("%1:%2")
                              .arg(secs / 60, 2, 10, QChar('0'))
                              .arg(secs % 60, 2, 10, QChar('0')));

    // ── Call Assembly executable ─────────────────────────────
    int correctChars = 0, incorrectChars = 0, totalChars = 0, wordCount = 0;

    bool asmOk = runAssembly(currentSentence, typed,
                             correctChars, incorrectChars,
                             totalChars, wordCount);

    if (!asmOk) {
        // Detailed errors are handled via popups inside runAssembly!
        return;
    }

    showResults(correctChars, incorrectChars, totalChars, wordCount, elapsedSec);
}

// ════════════════════════════════════════════════════════════
//  Slot: Timer tick
// ════════════════════════════════════════════════════════════
void MainWindow::updateTimer()
{
    if (!testRunning) return;
    qint64 ms = elapsedTimer.elapsed();
    int secs  = static_cast<int>(ms / 1000);
    lblLiveTimer->setText(QString("%1:%2")
                              .arg(secs / 60, 2, 10, QChar('0'))
                              .arg(secs % 60, 2, 10, QChar('0')));
}

// ════════════════════════════════════════════════════════════
//  Show results in the GUI
// ════════════════════════════════════════════════════════════
void MainWindow::showResults(int correctChars, int incorrectChars,
                             int totalChars, int wordCount,
                             double elapsedSec)
{
    double accuracy = 0.0;
    if (totalChars > 0)
        accuracy = (static_cast<double>(correctChars) * 100.0) / totalChars;

    double minutes = elapsedSec / 60.0;
    double wpm = (minutes > 0.0) ? (wordCount / minutes) : 0.0;

    lblResultCorrect->setText(QString::number(correctChars));
    lblResultIncorrect->setText(QString::number(incorrectChars));
    lblResultAccuracy->setText(QString("%1%").arg(accuracy, 0, 'f', 1));
    lblResultWPM->setText(QString("%1").arg(static_cast<int>(std::round(wpm))));
    lblResultTime->setText(QString("%1 sec").arg(elapsedSec, 0, 'f', 1));
}

// ════════════════════════════════════════════════════════════
//  Run Assembly executable via temp files (UPDATED DEBUG VERSION)
// ════════════════════════════════════════════════════════════
bool MainWindow::runAssembly(const QString &original,
                             const QString &typed,
                             int &correctOut, int &incorrectOut,
                             int &totalOut,   int &wordOut)
{
    QString asmExe = locateAssemblyExe();
    if (asmExe.isEmpty()) {
        QMessageBox::critical(this, "Debug Info", "File Status: NOT FOUND anywhere!");
        return false;
    }

    // Write input file
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString inFile  = tempDir + "/typing_in.txt";
    QString outFile = tempDir + "/typing_out.txt";

    {
        QFile f(inFile);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
        QTextStream ts(&f);
        ts << original << "\n" << typed << "\n";
    }

    // Run assembly executable
    QProcess proc;
    proc.setProgram(asmExe);
    proc.setArguments({ inFile, outFile });
    proc.start();

    if (!proc.waitForFinished(5000)) {
        proc.kill();
        QMessageBox::critical(this, "Debug Info", "Assembly Status: HUNG / TIMEOUT\n(Code stuck in infinite loop)");
        return false;
    }

    if (proc.exitCode() != 0) {
        QMessageBox::critical(this, "Debug Info",
                              QString("Assembly Status: CRASHED!\n\nExit Code: %1\nProcess Error: %2")
                                  .arg(proc.exitCode())
                                  .arg(proc.errorString()));
        return false;
    }

    // Read output file
    QFile fOut(outFile);
    if (!fOut.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream ts(&fOut);
    correctOut   = ts.readLine().trimmed().toInt();
    incorrectOut = ts.readLine().trimmed().toInt();
    totalOut     = ts.readLine().trimmed().toInt();
    wordOut      = ts.readLine().trimmed().toInt();

    return true;
}

// Locate typing_analyzer.exe next to the app or in CWD
QString MainWindow::locateAssemblyExe() const
{
    QStringList candidates = {
        QApplication::applicationDirPath() + "/typing_analyzer.exe",
        QDir::currentPath() + "/typing_analyzer.exe",
        QApplication::applicationDirPath() + "/asm/typing_analyzer.exe"
    };
    for (const QString &p : candidates) {
        if (QFileInfo::exists(p)) return p;
    }
    return {};
}