/*******************************************************************************
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
* (c) 2015~2016 SubCarrier Systems Corporation (SCSC)                               *
* Contract No.:                                                                *
* Contractor Name:                                                             *
* Contractor Address:                                                          *
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
* File Name:           ChartDialog.h                                           *
* Dependencies:        None                                                    *
* Operating System(s): 64-bit Windows 7, 64-bit Linux (K)Ubuntu 15             *
* Compiler(s):         Qt C++ 5.5.0                                            *
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
* Revision History:                                                            *
* Author           Date      Comments                                          *
* ---------------  --------  ------------------------------------------------- *
* Fabio Estupinan  11/19/15  Initial release.                                  *
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
*******************************************************************************/

#pragma once

#include <QTabWidget>
#include <QVector>
#include <QChartView>
#include <QtCharts>

class ClientChart;
class AClient;

class ChartTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    // Constructor contains normal init() items as well
    explicit ChartTabWidget(QWidget *pParent=0);
    ~ChartTabWidget();

    void addClientChart(AClient *pClient);
    void removeClientChart(const int id);

    bool hasClient(const int id);

private:
    QVector<ClientChart*> m_pChartList;


signals:


};

