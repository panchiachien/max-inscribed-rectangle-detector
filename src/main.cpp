// 最大內接矩形檢測：黑底灰階圖像中白色區塊的最大內接(可旋轉)矩形
#include <opencv2/opencv.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

struct RectResult {
    cv::Point2f center;
    float width = 0, height = 0;  // width 沿 angle 方向
    double angle = 0;             // 度，[0,360)，影像座標系（y 向下，順時針為正）
    double area = 0;
    bool valid = false;
};

// 二值矩陣中的最大軸對齊全 1 矩形（直方圖 + 單調堆疊）
static cv::Rect largestAxisAlignedRect(const cv::Mat& bin) {
    std::vector<int> h(bin.cols, 0);
    cv::Rect best;
    long bestArea = 0;
    for (int y = 0; y < bin.rows; ++y) {
        const uchar* row = bin.ptr<uchar>(y);
        for (int x = 0; x < bin.cols; ++x) h[x] = row[x] ? h[x] + 1 : 0;
        std::vector<int> st;
        for (int x = 0; x <= bin.cols; ++x) {
            int cur = (x == bin.cols) ? 0 : h[x];
            while (!st.empty() && h[st.back()] >= cur) {
                int height = h[st.back()];
                st.pop_back();
                int left = st.empty() ? 0 : st.back() + 1;
                long area = (long)height * (x - left);
                if (area > bestArea) {
                    bestArea = area;
                    best = cv::Rect(left, y - height + 1, x - left, height);
                }
            }
            st.push_back(x);
        }
    }
    return best;
}

// 對單一區塊遮罩，以 angleStep 度為間隔搜尋最大內接矩形
static RectResult maxInscribedRect(const cv::Mat& mask, double angleStep) {
    RectResult res;
    cv::Rect bb = cv::boundingRect(mask);
    if (bb.area() == 0) return res;
    cv::Mat roi = mask(bb);

    // 畫布大小取對角線，確保任意角度旋轉後不被裁切
    int diag = (int)std::ceil(std::hypot(roi.cols, roi.rows)) + 2;
    cv::Mat canvas = cv::Mat::zeros(diag, diag, CV_8UC1);
    cv::Point2f c0(diag / 2.0f, diag / 2.0f);
    roi.copyTo(canvas(cv::Rect((diag - roi.cols) / 2, (diag - roi.rows) / 2, roi.cols, roi.rows)));
    cv::Point2f offset((diag - roi.cols) / 2.0f, (diag - roi.rows) / 2.0f);

    cv::Mat rotated;
    for (double a = 0; a < 180.0; a += angleStep) {
        cv::Mat M = cv::getRotationMatrix2D(c0, a, 1.0);
        cv::warpAffine(canvas, rotated, M, canvas.size(), cv::INTER_NEAREST);
        cv::Rect r = largestAxisAlignedRect(rotated);
        double area = (double)r.width * r.height;
        if (area <= res.area) continue;

        cv::Mat Minv;
        cv::invertAffineTransform(M, Minv);
        auto back = [&](cv::Point2f p) {
            return cv::Point2f((float)(Minv.at<double>(0, 0) * p.x + Minv.at<double>(0, 1) * p.y + Minv.at<double>(0, 2)),
                               (float)(Minv.at<double>(1, 0) * p.x + Minv.at<double>(1, 1) * p.y + Minv.at<double>(1, 2)));
        };
        cv::Point2f rc(r.x + r.width / 2.0f, r.y + r.height / 2.0f);
        cv::Point2f p0 = back(cv::Point2f(rc.x - 1, rc.y)), p1 = back(cv::Point2f(rc.x + 1, rc.y));
        double ang = std::atan2(p1.y - p0.y, p1.x - p0.x) * 180.0 / CV_PI;
        if (ang < 0) ang += 360.0;

        res.center = back(rc) - offset + cv::Point2f((float)bb.x, (float)bb.y);
        res.width = (float)r.width;
        res.height = (float)r.height;
        res.angle = ang;
        res.area = area;
        res.valid = true;
    }
    return res;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "用法: " << argv[0] << " <input_image> [output_image] [angle_step=1.0] [threshold=127]\n";
        return 1;
    }
    std::string outPath = argc > 2 ? argv[2] : "result.png";
    double step = argc > 3 ? std::atof(argv[3]) : 1.0;
    int thresh = argc > 4 ? std::atoi(argv[4]) : 127;
    if (step <= 0 || step > 90) {
        std::cerr << "angle_step 必須在 (0, 90] 之間\n";
        return 1;
    }

    cv::Mat gray = cv::imread(argv[1], cv::IMREAD_GRAYSCALE);
    if (gray.empty()) {
        std::cerr << "無法讀取圖像: " << argv[1] << "\n";
        return 1;
    }

    cv::Mat bin;
    cv::threshold(gray, bin, thresh, 255, cv::THRESH_BINARY);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(bin.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    RectResult best;
    for (const auto& cnt : contours) {
        cv::Mat mask = cv::Mat::zeros(bin.size(), CV_8UC1);
        cv::drawContours(mask, std::vector<std::vector<cv::Point>>{cnt}, 0, cv::Scalar(255), cv::FILLED);
        mask &= bin;  // 排除輪廓內部的孔洞
        RectResult r = maxInscribedRect(mask, step);
        if (r.valid && r.area > best.area) best = r;
    }

    if (!best.valid) {
        std::cerr << "未偵測到白色區塊\n";
        return 2;
    }

    std::cout << "中心點: (" << best.center.x << ", " << best.center.y << ")\n"
              << "寬度: " << best.width << "\n"
              << "高度: " << best.height << "\n"
              << "旋轉角度: " << best.angle << " 度\n"
              << "面積: " << best.area << "\n";

    cv::Mat vis;
    cv::cvtColor(gray, vis, cv::COLOR_GRAY2BGR);
    cv::RotatedRect rr(best.center, cv::Size2f(best.width, best.height), (float)best.angle);
    cv::Point2f pts[4];
    rr.points(pts);
    for (int i = 0; i < 4; ++i) cv::line(vis, pts[i], pts[(i + 1) % 4], cv::Scalar(0, 0, 255), 2);
    cv::circle(vis, best.center, 3, cv::Scalar(0, 255, 0), -1);
    if (!cv::imwrite(outPath, vis)) {
        std::cerr << "無法寫入: " << outPath << "\n";
        return 1;
    }
    std::cout << "結果已儲存至 " << outPath << "\n";
    if (std::getenv("DISPLAY")) {
        cv::imshow("Max inscribed rectangle", vis);
        cv::waitKey(0);
    }
    return 0;
}
