#ifndef OSCILLOSCOPEWINDOW_H
#define OSCILLOSCOPEWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QDateTime>

namespace Ui {
class OscilloscopeWindow;
}

class OscilloscopeWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit OscilloscopeWindow(QWidget *parent = nullptr);
    ~OscilloscopeWindow();

    typedef struct{
        QVector<double> x;
        QVector<double> y1;
    }POINT_LIST;

public slots:
    ///
    /// \brief appendPoint
    /// \param value
    ///
    void appendPoint(double value);

    ///
    /// \brief clearPoint
    ///
    void clearPoint();

    ///
    /// \brief 设置最大显示点数
    /// \param count
    ///
    void setMaxPointCount(int count);

private:
    Ui::OscilloscopeWindow *ui;
    POINT_LIST point_list;
    uint32_t curr_points_count;
    uint32_t last_points_count;
    uint32_t max_points_count;
    QDateTime start_time;
    double elapsed_Time;
};

#endif // OSCILLOSCOPEWINDOW_H
