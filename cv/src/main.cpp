#include <opencv2/opencv.hpp>
#include <vector>
using namespace std;

int main() {
    cv::Mat image = cv::imread("pic.png");
    if (image.empty()) {
        cout << "Read error" << endl;
        return 1;
    }
    cv::Mat data;
    cv::cvtColor(image, data, cv::COLOR_BGR2HSV);
    cv::GaussianBlur(data, data, cv::Size(3, 3), 0);
    vector<cv::Mat> hsvData;
    cv::split(data, hsvData);
    cv::Mat light = hsvData[2];
    cv::threshold(light, data, 128, 255, cv::THRESH_BINARY);
    vector<vector<cv::Point>> contours;
    vector<cv::Vec4i> hierarchy;
    cv::findContours(data, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::Mat result = image.clone();
    vector<cv::Rect> allDetectData;
    for (auto i = 0; i < contours.size(); ++i) {
        if (cv::contourArea(contours[i]) < 200) {
            continue;
        }
        cout << i << endl;
        auto conData = contours[i];
        cv::Rect rect = cv::boundingRect(conData);
        if (rect.width * 2 > rect.height) {
            continue;
        }
        
        allDetectData.push_back(rect);
    }
    vector<int> allDetectPointY;
    for (auto i: allDetectData) {
        int x = i.x + i.width / 2;
        int y = i.y + i.height / 2;
        allDetectPointY.push_back(y);
    }

    int xLine = allDetectPointY[allDetectPointY.size() / 2];
    vector<cv::Rect> rightDetectDataLine;
    for (auto i: allDetectData) {
        int x = i.x + i.width / 2;
        int y = i.y + i.height / 2;
        if (std::abs(xLine - y) > 50) {
            continue;
        }
        rightDetectDataLine.push_back(i);
        
    }

    std::sort(rightDetectDataLine.begin(), rightDetectDataLine.end(),
          [](const cv::Rect& a, const cv::Rect& b) {
              return (a.x + a.width / 2) < (b.x + b.width / 2);
          });
    vector<int> distanceVector;
    for (int i = 0;i < rightDetectDataLine.size() - 1; ++i) {
        auto firstPoint = rightDetectDataLine[i];
        auto secondPoint = rightDetectDataLine[i + 1];
        int firstPointX = firstPoint.x + firstPoint.width / 2;
        int secondPointX = secondPoint.x + secondPoint.width / 2;
        int distance = std::abs(secondPointX - firstPointX);
        distanceVector.push_back(distance);
    }
    int distanceLine = distanceVector[distanceVector.size() / 2];
    cv::Rect leftRect;
    cv::Rect rightRect;
    for (int i = 0;i < rightDetectDataLine.size() - 1; ++i) {
        auto firstPoint = rightDetectDataLine[i];
        auto secondPoint = rightDetectDataLine[i + 1];
        int firstPointX = firstPoint.x + firstPoint.width / 2;
        int secondPointX = secondPoint.x + secondPoint.width / 2;
        int distance = std::abs(secondPointX - firstPointX);
        if (std::abs(distance - distanceLine) < 50) {
            leftRect = firstPoint;
            rightRect = secondPoint;
            cv::rectangle(result, leftRect, cv::Scalar(0, 0, 255), 2);
            cv::rectangle(result, rightRect, cv::Scalar(0, 0, 255), 2);
            break;
        }
    }
    cv::Rect roi(
        cv::Point(leftRect.x + leftRect.width, std::max(0, std::max(leftRect.y, rightRect.y) - std::max(leftRect.height, rightRect.height) / 2)), 
        cv::Point(rightRect.x, std::min(std::max(leftRect.y, rightRect.y) + rightRect.height + std::max(leftRect.height, rightRect.height) / 2, image.size().height)));
    cv::findContours(data(roi), contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    int maxRectS = 0;
    cv::Rect maxRect;
    for (auto i = 0; i < contours.size(); ++i) {
        if (cv::contourArea(contours[i]) < maxRectS) {
            continue;
        }
        maxRectS = cv::contourArea(contours[i]);
        auto conData = contours[i];
        cv::Rect rect = cv::boundingRect(conData);
        maxRect = rect;
    }
    cv::rectangle(result, cv::Point(roi.x + maxRect.x, roi.y + maxRect.y), 
        cv::Point(roi.x + maxRect.x + maxRect.width, roi.y + maxRect.y + maxRect.height), cv::Scalar(0, 255, 0), 2);
    cv::imshow("Image", result);
    cv::imwrite("detect.png", result);
    cv::imwrite("output.png", image(roi)(maxRect));
    cv::waitKey(0);
    cv::destroyAllWindows();
    return 0;
}