#ifndef MAINWINDOWWIDGET_H
#define MAINWINDOWWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>

class TheServer;
class ChartDialog;
class ClientTableWidget;

class MainWindowWidget : public QWidget
{
    Q_OBJECT

public:
    MainWindowWidget(TheServer *pApp, QWidget *parent = 0);
    ~MainWindowWidget();

private:
    void createLayout();
    void populateWidgets();

    TheServer *m_pServer;

    ClientTableWidget *m_pClientTable;
    QTextEdit *m_pLogEdit;
    ChartDialog *m_pChartDialog;

    QCheckBox *m_pDatabaseCheckBox;
    QCheckBox *m_pLogCheckBox;
    QCheckBox *m_pRawCheckBox;
    QPushButton *m_pAddSerialButton;
    QPushButton *m_pLogButton;
    QPushButton *m_pChartButton;
    QPushButton *m_pWebButton;

private slots:
    void onDatabaseChecked(const bool checked);
    void onLoggingChecked(const bool checked);
    void onRawLoggingChecked(const bool checked);
    void onAddSerialPushed();
    void onLogButtonPushed();
    void onChartButtonPushed();
    void onWebButtonPushed();


};

#endif // MAINWINDOWWIDGET_H
