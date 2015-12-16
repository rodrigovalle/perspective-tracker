#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
//#include "cinder/Capture.h"

#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
//#include "opencv2/gpu/gpu.hpp"
#include "opencv2/highgui/highgui.hpp"
//#include "CinderOpenCv.h"

#include <iostream>
#include <Windows.h>
#include <wincon.h>

using namespace ci;
using namespace ci::app;
using namespace std;

const bool FULLSCREEN = false;
const int WINDOW_WIDTH = 960;
const int WINDOW_HEIGHT = 540;

class perspectiveTrackerApp : public App {
  public:
	void setup() override;
    void update() override;
	void draw() override;

    /* This function takes an image and returns information about
       green circles that it finds. */
    void findGreenCircles();

    /* Gives the developer a bunch of trackbars and shit to fine tune
       tracking/thresholding parameters */
    void createDebugInterface();

  private:
    cv::Mat frame;
    cv::Mat hsv_frame;
    vector<cv::Mat> hsv_channels;

    cv::Mat green_stuff_mask;
    cv::Mat green_stuff_open;
    cv::Mat green_stuff_erode;
    cv::Mat green_stuff_dilate;

    //vector<cv::Vec3f> circles;
    vector<vector<cv::Point>> contours;

    cv::VideoCapture webcam;

    // thresholding defaults
    int lh = 31;
    int uh = 153;
    int ls = 55;
    int us = 190;
    int lv = 97;
    int uv = 185;

    int open = 8;
    int erode = 16;
    int dilate = 16;

    string control_win = "debug_controls";
};

void perspectiveTrackerApp::setup()
{
    // get a console window
    AllocConsole();
    freopen("CONOUT$", "w", stdout);

    // find the default webcam and start the stream
    // try to find usb cams
    webcam = cv::VideoCapture(1);
    if (!webcam.isOpened()) {
        // try to find the the laptop cam
        webcam = cv::VideoCapture(0);
    } else if (!webcam.isOpened()) {
        cerr << "couldn't open any webcams :(" << endl;
    }

    if (webcam.isOpened()) {
        cout << "prop setting returned: " << webcam.set(CV_CAP_PROP_EXPOSURE, 20) << endl;
        webcam.set(CV_CAP_PROP_FPS, 30);
    }

    // make a debug interface with trackbars and shit
    createDebugInterface();
}

void perspectiveTrackerApp::update()
{
    webcam >> frame;

    try {
        // convert to HSV
        cv::cvtColor(frame, hsv_frame, cv::COLOR_BGR2HSV_FULL);

        // blur the HSV frame
        cv::GaussianBlur(hsv_frame, hsv_frame, cv::Size(15, 15), 0.0);

        // threshold, look for green things
        cv::Scalar lower(cv::getTrackbarPos("lh", control_win), cv::getTrackbarPos("ls", control_win), cv::getTrackbarPos("lv", control_win));
        cv::Scalar upper(cv::getTrackbarPos("uh", control_win), cv::getTrackbarPos("us", control_win), cv::getTrackbarPos("uv", control_win));
        cv::inRange(hsv_frame, lower, upper, hsv_frame);

        // first get rid of noise with open
        // then erode
        // then dilate

        open = cv::getTrackbarPos("open", control_win);
        erode = cv::getTrackbarPos("erode", control_win);
        dilate = cv::getTrackbarPos("dilate", control_win);

        cv::Mat open_kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(open, open));
        cv::Mat erode_kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(erode, erode));
        cv::Mat dilate_kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(dilate, dilate));

        cv::morphologyEx(hsv_frame, hsv_frame, cv::MORPH_OPEN, open_kernel);
        cv::imshow("open", hsv_frame);

        cv::HoughCircles(green_stuff_dilate, circles, CV_HOUGH_GRADIENT, 1, 30, 100, 80, 10, 150);
        cv::findContours(hsv_frame, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
        cv::drawContours(frame, contours, -1, cv::Scalar(0, 0, 255), 5);

        cv::imshow("frame", frame);
        cout << contours[0].

        // Yay, hopefull this'll be more optimized. If not, oh well, we'll see.
        //cv::cvtColor(frame, hsv_frame, cv::COLOR_RGB2HSV);
        //cv::imshow("orig", frame);

        //cv::split(hsv_frame, hsv_channels);
        //cv::medianBlur(hsv_channels[0], hsv_channels[0], 11);
        //cv::threshold(hsv_channels[0], frame, cv::getTrackbarPos("lh", control_win), 255, cv::THRESH_BINARY);
        //cv::adaptiveThreshold(hsv_channels[0], frame, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, cv::getTrackbarPos("lh", control_win), cv::getTrackbarPos("uh", control_win));

        cv::imshow("thresh", frame);

    } catch (cv::Exception e) {
        cout << e.what() << endl;
    }
}

void perspectiveTrackerApp::draw()
{
    CameraPersp cam;
    gl::setMatrices(cam);
    gl::drawCube(vec3(0, 0, 0), vec3(1, 1, 1));
}

void findGreenCircles()
{
    // TODO: move work out of update() and into here.
}

void perspectiveTrackerApp::createDebugInterface()
{
    cv::namedWindow(control_win,cv::WINDOW_NORMAL);
    cv::createTrackbar("lh", control_win, &lh, 256);
    cv::createTrackbar("uh", control_win, &uh, 256);
    cv::createTrackbar("ls", control_win, &ls, 256);
    cv::createTrackbar("us", control_win, &us, 256);
    cv::createTrackbar("lv", control_win, &lv, 256);
    cv::createTrackbar("uv", control_win, &uv, 256);

    cv::createTrackbar("open", control_win, &open, 32);
    cv::createTrackbar("erode", control_win, &erode, 16);
    cv::createTrackbar("dilate", control_win, &dilate, 16);
}

CINDER_APP( perspectiveTrackerApp, RendererGl )
