#include "mainwindow.h"

#include <QDate>
#include <QDir>
#include <QLabel>
#include <QPlainTextEdit>
#include <QStandardPaths>
#include <QTimer>
#include <QVBoxLayout>

#include <fcntl.h>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_textEdit(new QPlainTextEdit(this)),
      m_totalLabel(new QLabel(this)),
      m_dayLabel(new QLabel(this)),
      m_statusLabel(new QLabel(this)),
      m_timer(new QTimer(this)),
      m_fd(-1),
      m_stats{},
      m_statsReady(false) {
  setWindowTitle("Kbd Sim Monitor");
  scancode_state_init(&m_scancodeState);

  m_textEdit->setReadOnly(true);

  QWidget *central = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout(central);
  layout->addWidget(m_statusLabel);
  layout->addWidget(m_totalLabel);
  layout->addWidget(m_dayLabel);
  layout->addWidget(m_textEdit);
  setCentralWidget(central);

  m_devicePath = qEnvironmentVariable("DEVICE_PATH", "/dev/kbd");

  QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (!dataDir.isEmpty()) {
    QDir().mkpath(dataDir);
  }
  m_statsPath = dataDir + "/stats.txt";

  m_day = currentDay();
  QByteArray pathBytes = m_statsPath.toLocal8Bit();
  QByteArray dayBytes = m_day.toLocal8Bit();
  if (stats_init(&m_stats, pathBytes.constData(), dayBytes.constData()) == 0) {
    m_statsReady = true;
  }

  updateCounters(0);
  m_statusLabel->setText("device: waiting for /dev/kbd");

  connect(m_timer, &QTimer::timeout, this, &MainWindow::onTick);
  m_timer->start(100);
}

MainWindow::~MainWindow() {
  if (m_statsReady) {
    stats_save(&m_stats);
    stats_free(&m_stats);
  }
  if (m_fd >= 0) {
    ::close(m_fd);
  }
}

void MainWindow::onTick() {
  rotateDayIfNeeded();
  openDeviceIfNeeded();
  readDevice();
}

void MainWindow::openDeviceIfNeeded() {
  if (m_fd >= 0) {
    return;
  }
  m_fd = open(m_devicePath.toLocal8Bit().constData(), O_RDONLY | O_NONBLOCK);
  if (m_fd >= 0) {
    m_statusLabel->setText(QString("device: %1").arg(m_devicePath));
  }
}

void MainWindow::readDevice() {
  if (m_fd < 0) {
    return;
  }

  unsigned char buf[256];
  ssize_t n = 0;
  unsigned long added = 0;
  char out[32];
  while ((n = read(m_fd, buf, sizeof(buf))) > 0) {
    for (ssize_t i = 0; i < n; ++i) {
      unsigned long counted = 0;
      size_t out_len = scancode_process(&m_scancodeState, buf[i], out, sizeof(out), &counted);
      if (out_len == 0) {
        continue;
      }
      for (size_t j = 0; j < out_len; ++j) {
        applyChar(out[j]);
      }
      added += counted;
    }
  }

  if (n == 0) {
    return;
  }

  if (n < 0 && added == 0) {
    return;
  }

  if (added > 0) {
    updateCounters(added);
  }
}

void MainWindow::applyChar(char ch) {
  if (ch == '\b') {
    if (!m_buffer.isEmpty()) {
      m_buffer.chop(1);
    }
  } else if (ch == '\n') {
    m_buffer.append('\n');
  } else {
    m_buffer.append(ch);
  }

  if (m_buffer.size() > 200) {
    m_buffer = m_buffer.right(200);
  }

  m_textEdit->setPlainText(m_buffer);
}

void MainWindow::updateCounters(unsigned long added) {
  if (!m_statsReady) {
    m_totalLabel->setText("total count (since start/save): unavailable");
    m_dayLabel->setText("today count: unavailable");
    return;
  }

  if (added > 0) {
    stats_record(&m_stats, added);
    stats_save(&m_stats);
  }

  m_totalLabel->setText(QString("total count (since start/save): %1").arg(m_stats.total));
  m_dayLabel->setText(QString("today count: %1").arg(m_stats.day_count));
}

void MainWindow::rotateDayIfNeeded() {
  QString today = currentDay();
  if (today == m_day) {
    return;
  }

  if (m_statsReady) {
    stats_save(&m_stats);
    stats_free(&m_stats);
    m_statsReady = false;
  }

  m_day = today;
  QByteArray pathBytes = m_statsPath.toLocal8Bit();
  QByteArray dayBytes = m_day.toLocal8Bit();
  if (stats_init(&m_stats, pathBytes.constData(), dayBytes.constData()) == 0) {
    m_statsReady = true;
  }
  updateCounters(0);
}

QString MainWindow::currentDay() const {
  return QDate::currentDate().toString("yyyy-MM-dd");
}
