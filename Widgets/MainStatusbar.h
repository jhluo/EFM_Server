#pragma once

#include <QStatusBar>
#include <Qlabel>
#include <QDateTime>

class MainStatusbar : public QStatusBar
{
    Q_OBJECT

public:
    explicit MainStatusbar(QWidget *parent = 0);
    ~MainStatusbar();

private:
    void createLayout();
    void updateLabels();
    void timerEvent(QTimerEvent *event);

    QLabel *m_pCompanyLabel;
    QLabel *m_pTimeLabel;
};
