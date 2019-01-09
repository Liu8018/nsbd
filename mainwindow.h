#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "videoprocesser.h"
#include <opencv2/videoio.hpp>

#include <QMainWindow>
#include <QTimer>
#include<QFileDialog>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    
    void initParas();
    
    VideoProcesser processer;
    
    cv::VideoCapture capture;
    cv::Mat frame;
    cv::Mat cover;
    cv::Mat frame_show;
    QImage qimg_frame;
    QTimer *timer = new QTimer(this);
    
    int ruler_x;
    int ruler_y;
    double refrenceValue;
    int unitPix;
    
    int splt;
    cv::Vec4i waterLine;
    double waterLevelValue;
    
    void calculate();
    void drawRuler();
    void showMat();
    
private slots:
    void getframe();
    
    void on_action_open_triggered();
    void on_action_run_triggered();
    void on_action_stop_triggered();
    void on_verticalSlider_rule_sliderMoved(int position);
    void on_doubleSpinBox_waterLevelValue_valueChanged(double arg1);
    void on_horizontalSlider_rule_valueChanged(int value);
    void on_horizontalSlider_unitPix_valueChanged(int value);
};

#endif // MAINWINDOW_H
