#ifndef CHARTDIALOG_H
#define CHARTDIALOG_H

#include <QDialog>

class ChartTabWidget;
class AClient;

class ChartDialog : public QDialog
{
    Q_OBJECT

public:
    ChartDialog(QWidget *pParent=0);
    ~ChartDialog();

private:
    void createActions();

    ChartTabWidget *m_pTabWidget;

public slots:
    void onChartToggled(const bool enabled, AClient* pClient);

};

#endif // CHARTDIALOG_H
