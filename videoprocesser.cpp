#include "videoprocesser.h"

VideoProcesser::VideoProcesser()
{
	m_bufferLinesPlace = 0;
	m_bufferImagesPlace = 0;

	m_bufferImageSize = 2;
	m_bufferLinesSize = 20;
}

int getUnderLineWhitePointNum(const cv::Mat &img, const cv::Vec4i &line)
{
	int count = 0;

	int splt = 10;
	cv::Point vec(line[2]-line[0],line[3]-line[1]);

	for(int i=0;i<splt;i++)
	{
		cv::Point pt0( line[0]+i*vec.x/(float)splt , line[1]+i*vec.y/(float)splt );
		pt0.y += 4;

		if(pt0.y >= img.rows)
			continue;

		if(img.at<uchar>(pt0.y,pt0.x) == 255)
			count++;
	}

	return count;
}

void VideoProcesser::process(const cv::Mat & src)
{
	cv::Mat img;
    if(src.channels() == 3)
        cv::cvtColor(src,img,cv::COLOR_BGR2GRAY);
    else
        src.copyTo(img);

	//自适应阈值
	cv::adaptiveThreshold(img,img,255,0,1,305,30);

	//加入缓存
	appendBufferImage(img);
	cv::Mat intersection;
	getIntersection(intersection);
	intersection = 255 - intersection;

	//剔除变动部分
	img -= intersection;

	//test
//	cv::imshow("adaptiveThresh",img);

	//检测直线
	cv::Mat cannyImg;
	cv::Canny(img,cannyImg,100,100,3);
	std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(cannyImg,lines,1,CV_PI/180,100,70,70);

    //筛选直线
    std::vector<cv::Vec4i> lines_slct;
    for(int i=0;i<lines.size();i++)
    {
    	//根据水平程度筛选
        if(abs(lines[i][0]-lines[i][2]) < 7*abs(lines[i][1]-lines[i][3]))
        	continue;
        //根据水平方向的长度筛选
        else if(abs(lines[i][0] - lines[i][2]) < 2*img.cols/3)
        	continue;
        else if(getUnderLineWhitePointNum(img,lines[i]) < 6)
        	continue;
        else
            lines_slct.push_back(lines[i]);
    }

    //test
/*    cv::Mat testImg;
    img.copyTo(testImg);
    for(int i=0;i<lines_slct.size();i++ )
    {
        cv::line(testImg,cv::Point(lines_slct[i][0],lines_slct[i][1]),
        				 cv::Point(lines_slct[i][2],lines_slct[i][3]),
        				 cv::Scalar(255));

        int testNum = getUnderLineWhitePointNum(img,lines_slct[i]);
        cv::putText(testImg,std::to_string(testNum),
        			cv::Point(lines_slct[i][0],lines_slct[i][1]),
        			1,2,cv::Scalar(127),2);
    }
	cv::imshow("result",testImg);
*/
    //结果加入缓存
    appendBufferLine(lines_slct);

}

void VideoProcesser::appendBufferImage(const cv::Mat &newImage)
{
	if(m_bufferImages.size() < m_bufferImageSize)
		m_bufferImages.push_back(newImage);
	else
	{
		newImage.copyTo(m_bufferImages[m_bufferImagesPlace]);

		m_bufferImagesPlace++;
		m_bufferImagesPlace %= m_bufferImageSize;
	}
}

void VideoProcesser::getIntersection(cv::Mat & image)
{
	m_bufferImages[0].copyTo(image);
	for(int i=1;i<m_bufferImages.size();i++)
		image = (image == m_bufferImages[i]);
}

void VideoProcesser::appendBufferLine(const std::vector<cv::Vec4i> &newLines)
{
	if(m_bufferWaterLines.size() < m_bufferLinesSize)
		m_bufferWaterLines.push_back(newLines);
	else
	{
		m_bufferWaterLines[m_bufferLinesPlace].assign(newLines.begin(),newLines.end());

		m_bufferLinesPlace++;
		m_bufferLinesPlace %= m_bufferLinesSize;
	}
}

void fitLine(const std::vector<cv::Vec4i> &lines, cv::Vec4i &resultLine);

void VideoProcesser::getWaterLine(cv::Vec4i &resultLine)
{
	std::vector<cv::Vec4i> sumLines;
	for(int i=0;i<m_bufferWaterLines.size();i++)
		for(int j=0;j<m_bufferWaterLines[i].size();j++)
			sumLines.push_back(m_bufferWaterLines[i][j]);
	
	fitLine(sumLines,resultLine);
}

void fitNum(const std::vector<int> &nums, int &resultNum);

void fitLine(const std::vector<cv::Vec4i> &lines, cv::Vec4i &resultLine)
{
	std::vector<int> left,right;
	for(int i=0;i<lines.size();i++)
	{
		left.push_back(lines[i][1]);
		right.push_back(lines[i][3]);
	}

	if(left.size() == 0)
		return;

	int leftFit,rightFit;
	fitNum(left,leftFit);
	fitNum(right,rightFit);

	resultLine[1] = leftFit;
	resultLine[3] = rightFit;
}

void fitNum(const std::vector<int> &nums, int &resultNum)
{
	int minDistValue = 0;
	int minDistPlace = 0;

	for(int i=0;i<nums.size();i++)
	{
		int dist = 0;
		for(int j=0;j<nums.size();j++)
		{
			dist += (nums[i]-nums[j])*(nums[i]-nums[j]);

			if(i==0)
				minDistValue = dist;
			else if(dist < minDistValue)
			{
				minDistValue = dist;
				minDistPlace = i;
			}
		}
	}

	resultNum = (nums[minDistPlace]);
}
