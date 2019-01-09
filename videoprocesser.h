#ifndef VIDEOPROCESSER_H
#define VIDEOPROCESSER_H

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

class VideoProcesser
{
public:
    VideoProcesser();
    
    void process(const cv::Mat &);

	void getWaterLine(cv::Vec4i &);
    
private:
	int m_bufferImageSize;
	std::vector<cv::Mat> m_bufferImages;
	int m_bufferImagesPlace;
	void appendBufferImage(const cv::Mat &);
	void getIntersection(cv::Mat &);

	int m_bufferLinesSize;
	std::vector<std::vector<cv::Vec4i>> m_bufferWaterLines;
	int m_bufferLinesPlace;
	void appendBufferLine(const std::vector<cv::Vec4i> &);
};

#endif // VIDEOPROCESSER_H
