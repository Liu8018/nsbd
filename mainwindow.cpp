#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    //将timer与getframe连接
    connect(timer,SIGNAL(timeout()),this,SLOT(getframe()));
}

MainWindow::~MainWindow()
{
    delete ui;
    
    delete timer;
}

void MainWindow::on_action_open_triggered()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,tr("Open Video"),".",tr("Video File(*.avi *.asf);;All Files(*)"));
    if(fileName.isEmpty()) return;
    capture.open(fileName.toLocal8Bit().data());
    
    initParas();
    
    timer->setInterval(1000/capture.get(CV_CAP_PROP_FPS));//按照每秒x帧计算，每过(1000/x)ms执行一次getframe
    timer->start();
}

void MainWindow::initParas()
{
    capture >> frame;
    
    VideoProcesser processer2;
    processer = processer2;
    
    waterLevelValue = 0;
    waterLine = {0,0,0,0};
    
    splt = 4;
    
    ruler_x = frame.cols * ui->horizontalSlider_rule->value() / (float)ui->horizontalSlider_rule->maximum();
    ruler_y = frame.rows * ui->verticalSlider_rule->value() / (float)ui->verticalSlider_rule->maximum();
    refrenceValue = ui->doubleSpinBox_waterLevelValue->value();
    unitPix = ui->horizontalSlider_unitPix->value();
}

void MainWindow::getframe()
{
    capture >> frame;

    //如果取不到数据，终止计时器
    if(frame.empty())
    {
        timer->stop();
        return;
    }
    
    //保证读取到的帧为三通道
    if(frame.channels() == 1)
        cv::cvtColor(frame,frame,cv::COLOR_GRAY2BGR);
    
    //分割ROI区域
    cv::Mat ROI = frame(cv::Range(0,frame.rows),cv::Range(frame.cols/splt,(splt-1)*frame.cols/splt));
    
    //检测水位线
    processer.process(ROI);
    
    waterLine[0] = frame.cols/splt;
    waterLine[2] = waterLine[0] + ROI.cols;
    processer.getWaterLine(waterLine);
    
    //计算水位
    calculate();

    //cover初始化
    cover.create(frame.size(),CV_8UC3);
    cover = cv::Scalar(0,0,0);
    
    //绘制水位线到cover上
    if(waterLine[1]*waterLine[3] != 0)
        cv::line(cover,cv::Point(waterLine[0],waterLine[1]),cv::Point(waterLine[2],waterLine[3]),cv::Scalar(255,0,0),8);
    
    //绘制ruler到cover上
    drawRuler();

    //显示
    showMat();
    
    ui->label->setText(QString::fromStdString(std::to_string(waterLevelValue)));
}

void MainWindow::showMat()
{
    if(ui->checkBox_showCover->checkState() == Qt::Checked)
        frame_show = frame + cover;
    else
        frame.copyTo(frame_show);
    
    cv::resize(frame_show,frame_show,cv::Size(ui->label_showFrame->width()-2,ui->label_showFrame->height()-2));//改变图像大小使适应窗口

    cv::cvtColor(frame_show,frame_show,CV_BGR2RGB);//转为RGB格式
    qimg_frame = QImage((const uchar*)frame_show.data,
                  frame_show.cols,frame_show.rows,
                  frame_show.cols*frame_show.channels(), //bytesPerLine,即每行的数据量,此处用处是使数据对齐
                  QImage::Format_RGB888); //简单地转换一下为QImage对象

    ui->label_showFrame->setPixmap(QPixmap::fromImage(qimg_frame));
}

void MainWindow::drawRuler()
{
    int x = frame.cols * ui->horizontalSlider_rule->value() / ui->horizontalSlider_rule->maximum();
    int y = frame.rows * ui->verticalSlider_rule->value() / ui->verticalSlider_rule->maximum();
    
    cv::line(cover, cv::Point(x,0), cv::Point(x,cover.rows-1), cv::Scalar(0,0,255),2);
    cv::line(cover, cv::Point(0,y), cv::Point(cover.cols-1,y), cv::Scalar(0,0,255),2);
    
    cv::putText(cover, std::to_string(refrenceValue).substr(0,4),cv::Point(x,y),1,1,cv::Scalar(0,255,255),2);
    
    for(int tmp_y = y, i=0; tmp_y>0; tmp_y -= unitPix, i++)
    {
        cv::line(cover, cv::Point(x-100,tmp_y), cv::Point(x+100,tmp_y),cv::Scalar(0,255,0),2);
        cv::putText(cover, std::to_string(refrenceValue+i).substr(0,4),cv::Point(x,tmp_y),1,1,cv::Scalar(0,255,255),2);
    }
    for(int tmp_y = y+unitPix, i=0; tmp_y < cover.rows; tmp_y += unitPix, i++)
    {
        cv::line(cover, cv::Point(x-100,tmp_y), cv::Point(x+100,tmp_y),cv::Scalar(0,255,0),2);
        cv::putText(cover, std::to_string(refrenceValue-i-1).substr(0,4),cv::Point(x,tmp_y),1,1,cv::Scalar(0,255,255),2);
    }
}

void MainWindow::calculate()
{
    int x1 = waterLine[0], y1 = waterLine[1], x2 = waterLine[2], y2 = waterLine[3];
    int waterLevel_y = (y2-y1)*(ruler_x-x1)/(x2-x1) + y1;
    
    waterLevelValue = refrenceValue - (waterLevel_y - ruler_y)/(float)unitPix;
}

void MainWindow::on_action_run_triggered()
{
    if(!timer->isActive())
        timer->start();
}

void MainWindow::on_action_stop_triggered()
{
    if(timer->isActive())
        timer->stop();
}

void MainWindow::on_verticalSlider_rule_sliderMoved(int position)
{
    //移动旁边的spinBox
    int x = ui->doubleSpinBox_waterLevelValue->x();
    int y = (ui->verticalSlider_rule->height()-30) * position / (float)ui->verticalSlider_rule->maximum();
            
    ui->doubleSpinBox_waterLevelValue->move(x, y);
    
    //设置ruler_y
    ruler_y = frame.rows * position / (float)ui->verticalSlider_rule->maximum();
}

void MainWindow::on_doubleSpinBox_waterLevelValue_valueChanged(double arg1)
{
    refrenceValue = arg1;
}

void MainWindow::on_horizontalSlider_rule_valueChanged(int value)
{
    ruler_x = frame.cols * value / (float)ui->horizontalSlider_rule->maximum();
}

void MainWindow::on_horizontalSlider_unitPix_valueChanged(int value)
{
    unitPix = value;
}
