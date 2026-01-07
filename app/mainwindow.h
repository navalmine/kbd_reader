#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QString>

extern "C" {
#include "stats.h"
#include "scancode_map.h"
}

class QLabel;
class QPlainTextEdit;
class QTimer;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

private slots:
  void onTick();

private:
  void openDeviceIfNeeded();
  void readDevice();
  void applyChar(char ch);
  void updateCounters(unsigned long added);
  void rotateDayIfNeeded();
  QString currentDay() const;

  QPlainTextEdit *m_textEdit;
  QLabel *m_totalLabel;
  QLabel *m_dayLabel;
  QLabel *m_statusLabel;
  QTimer *m_timer;

  QString m_buffer;
  int m_fd;
  stats_t m_stats;
  QString m_day;
  QString m_devicePath;
  QString m_statsPath;
  bool m_statsReady;
  scancode_state_t m_scancodeState;
};

#endif
