#include "oscilloscopewindow.h"
#include "ui_oscilloscopewindow.h"
#include <QThread>

OscilloscopeWindow::OscilloscopeWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OscilloscopeWindow)
{
    ui->setupUi(this);

    // add two new graphs and set their look:
    ui->widget->addGraph();
    ui->widget->graph(0)->setPen(QPen(Qt::blue));
//    ui->widget->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
    ui->widget->setBackground(QBrush(Qt::lightGray));
    ui->widget->xAxis2->setVisible(true);
    ui->widget->xAxis2->setTickLabels(false);
    ui->widget->yAxis2->setVisible(true);
    ui->widget->yAxis2->setTickLabels(false);

    // make left and bottom axes always transfer their ranges to right and top axes:
    connect(ui->widget->yAxis, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged), this,[this](const QCPRange &range) {
        ui->widget->yAxis2->setRange(QCPRange(range.lower -range.lower*0.1,range.upper+range.upper*0.1));
    });

    // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
    ui->widget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    max_points_count = 5000;
    ui->widget->xAxis->setRange(0, max_points_count);

    this->clearPoint();

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->start(50);

    connect(timer,&QTimer::timeout,this,[this]() {
        if (curr_points_count != last_points_count) {
            last_points_count = curr_points_count;
            if(point_list.x.count() > max_points_count){
                point_list.x.remove(0, point_list.x.count() - max_points_count);
                point_list.y1.remove(0, point_list.y1.count() - max_points_count);
            }

            ui->widget->graph(0)->setData(point_list.x, point_list.y1);
            ui->widget->graph(0)->rescaleAxes();
//            ui->widget->graph(0)->rescaleValueAxis();
            ui->widget->replot();
        }
    });

//    // 正弦波模拟测试
//    QTimer *timer1 = new QTimer(this);
//    timer1->start(1);
//    timer1->setSingleShot(false);
//    connect(timer1,&QTimer::timeout,this,[this]() {
//        static double amplitude = 0;
//        for (int i = 0; i < 10; i++) {
//            this->appendPoint(qSin(elapsed_Time) + amplitude);
//            amplitude += 0.0001;
//        }
//    });
}

OscilloscopeWindow::~OscilloscopeWindow()
{
    delete ui;
}

void OscilloscopeWindow::appendPoint(double value)
{
    elapsed_Time = start_time.msecsTo(QDateTime::currentDateTime());
    point_list.x.push_back(elapsed_Time / 1000.0);
    point_list.y1.push_back(value);
    curr_points_count++;
}

void OscilloscopeWindow::clearPoint()
{
    curr_points_count = 0;
    start_time = QDateTime::currentDateTime();
    elapsed_Time = 0;
    point_list.x.clear();
    point_list.y1.clear();
    ui->widget->graph(0)->setData(point_list.x, point_list.y1);
    ui->widget->graph(0)->rescaleAxes();
}

void OscilloscopeWindow::setMaxPointCount(int count)
{
    max_points_count = count;
}
