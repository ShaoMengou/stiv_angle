#include <iostream>
#include <vector>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cmath>

const double PI = 3.1415926535;

int main(int argc, char* argv[]){
    cv::VideoCapture v_in(argv[1]);

    cv::Mat frame_in;
    v_in>>frame_in;

    std::cout<<"go"<<std::endl;

    cv::Rect rect(300,400,300,1);
    cv::Mat tagged = frame_in.clone();

    cv::Rect rect_tag = rect+cv::Size(6,6)+cv::Point(-3,-3);
    cv::rectangle(tagged,rect_tag,cv::Scalar(0.0,0.0,255.0),2);

    cv::namedWindow("tagged area",cv::WINDOW_NORMAL);
    cv::resizeWindow("tagged area",640,480);
    cv::imshow("tagged area",tagged);

    cv::Mat sum = frame_in(rect);

    float avg_rad = 0.0f;
    float sum_c = 0.0f;

    for(;;){
        v_in>>frame_in;
        if (frame_in.empty())break;

        sum.push_back(frame_in(rect));
    }
    cv::cvtColor(sum,sum,cv::COLOR_BGR2GRAY);
    sum = sum(cv::Rect(0,200,sum.cols,sum.rows-200)); //剔除河流未上镜的空白区域
    cv::imshow("total gray",sum);

    std::vector<cv::Mat> parts; // 我们将其分为九宫格
    int step_x = sum.cols/3;
    int step_y = sum.rows/3;

    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            parts.push_back(sum(cv::Rect(j*step_x,i*step_y,step_x,step_y))) ;
        }
    }

    std::vector<cv::Mat>::iterator it = parts.begin();
    for(;it!=parts.end();it++){
        int index = std::distance(parts.begin(),it);
        cv::imshow(std::to_string(index),*it);
        std::cout<<std::endl;
        std::cout<< "In part "<<index<<":"<<std::endl;

        cv::Mat g_x;cv::Sobel(*it, g_x,-1, 1,0,cv::FILTER_SCHARR);// cv::imshow("g_x",g_x);
        cv::Mat g_t;cv::Sobel(*it, g_t,-1, 0,1,cv::FILTER_SCHARR);// cv::imshow("g_t",g_t);

        float g_xx = g_x.dot(g_x);
        float g_xt = g_x.dot(g_t);
        float g_tt = g_t.dot(g_t);

        float c = (std::sqrt(std::pow((g_xx-g_tt),2)+4*g_xt*g_xt))/(g_xx+g_tt); //清晰度

        float rad = std::atan((2*g_xt)/(g_tt-g_xx))/2.0;
        std::cout<<"degree: "<< rad*180/PI<< "("<<rad<<" rad)"<<std::endl;
        std::cout<<"c: "<< c<< std::endl;
        avg_rad+=rad*c;
        sum_c+=c;
    }

    avg_rad/=sum_c;
    std::cout<<std::endl;
    std::cout<<"Weighted avg degree: "<< avg_rad*180/PI<< "("<<avg_rad<<" rad)"<<std::endl;

    cv::waitKey(0);
    return 0;
}
