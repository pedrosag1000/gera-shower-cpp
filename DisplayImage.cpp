#include <opencv2/opencv.hpp>
#include <math.h>
#include <CppLinuxSerial/SerialPort.hpp>
#include <chrono>
#include <ctime>
#include <thread>
#include<stdlib.h>
#include <mutex>

using namespace cv;

using namespace std;

using namespace mn::CppLinuxSerial;

#define PI 3.14159265

int ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0;
#define QUEUE_SIZE 10;
Mat paintedFrames[10];

double radar_angle = -100, radar_size_of_angle = 20;
double view_angle = -100, view_size_of_angle = 20;
double horizental_angle = 60, vertical_angle = 60;
double zoom = 2.2;
string serialPortAddress = "";


const vector<string> explode(const string &s, const char &c) {
    string buff{""};
    vector<string> v;

    for (auto n: s) {
        if (n != c) buff += n;
        else if (n == c && buff != "") {
            v.push_back(buff);
            buff = "";
        }
    }
    if (buff != "") v.push_back(buff);

    return v;
}


void shutdown() {
    system("shutdown -h now");
}

void
draw_text_center(Mat frame, string text, Point center, int fontFace, double fontScale, Scalar color, int thickness) {
    int baseLine = 0;
    Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseLine);
    baseLine += thickness;

    Point textOrg = Point(center.x - textSize.width / 2, center.y + textSize.height / 2);
    // rectangle(frame, textOrg + Point(0, baseLine), textOrg + Point(textSize.width, -textSize.height), color);
    // line(frame, textOrg + Point(0, thickness), textOrg + Point(textSize.width, thickness), color);

    putText(frame, text, textOrg, fontFace, fontScale, color, thickness);
}

void
draw_text_vertical_center(Mat frame, string text, Point leftAndVerticalCenter, int fontFace, double fontScale,
                          Scalar color, int thickness) {
    int baseLine = 0;
    Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseLine);
    baseLine += thickness;

    Point textOrg = Point(leftAndVerticalCenter.x, leftAndVerticalCenter.y + textSize.height / 2);
    // rectangle(frame, textOrg + Point(0, baseLine), textOrg + Point(textSize.width, -textSize.height), color);
    // line(frame, textOrg + Point(0, thickness), textOrg + Point(textSize.width, thickness), color);

    putText(frame, text, textOrg, fontFace, fontScale, color, thickness);
}

void
draw_arch(Mat frame, Point center, int circle_radius, int view_angle, int size_of_angle, Scalar color, int thickness,
          bool drawAngle = false, double fontSize = 0.5, bool overrideAngleText = false, int overrideAngle = 0) {

    int half_of_size_of_angle = size_of_angle / 2;
    ellipse(
            frame,
            center,
            Size(circle_radius, circle_radius),
            view_angle - half_of_size_of_angle,
            0,
            size_of_angle,
            color, thickness);

    int diff_x = int(cos((view_angle - half_of_size_of_angle) / 180.0 * PI) * circle_radius);
    int diff_y = int(sin((view_angle - half_of_size_of_angle) / 180.0 * PI) * circle_radius);
    line(
            frame,
            center,
            Point(center.x + diff_x, center.y + diff_y),
            color, thickness);
    diff_x = int(cos((view_angle + half_of_size_of_angle) / 180.0 * PI) * circle_radius);
    diff_y = int(sin((view_angle + half_of_size_of_angle) / 180.0 * PI) * circle_radius);
    line(
            frame,
            center,
            center + Point(diff_x, diff_y),
            color, thickness);

    if (drawAngle) {
        diff_x = int(cos((view_angle) / 180.0 * PI) * circle_radius);
        diff_y = int(sin((view_angle) / 180.0 * PI) * circle_radius);

        draw_text_center(frame, to_string(overrideAngle), center + Point(diff_x * 0.8, diff_y * 0.8),
                         FONT_HERSHEY_SIMPLEX, fontSize, color, thickness);
    }
}


VideoWriter videoWriter;
VideoCapture videoCapture;
string videoCaptureAddress;
int paintedFrameId = 0;
Mat frame, originalFrame, splashScreen;
int pressedKey = 10;
Point touchedPoint(0, 0);
int touchId = 0;
int lastTouchReported = 0;
string outputBuffer;
string videoWriterFileFullPath;
int displayWidth, displayHeight;

void openVideoCapture(bool force = false) {
    while ((!videoCapture.isOpened() || force) && pressedKey != 27) {
        cout << "Waiting for camera" << endl;
        int newPaintedFrame = (paintedFrameId + 1) % 10;
        paintedFrames[newPaintedFrame] = splashScreen;
        paintedFrameId = newPaintedFrame;

        this_thread::sleep_for(chrono::milliseconds(1000));
        videoCapture.release();
        if (!videoCaptureAddress.empty()) {
            videoCapture = VideoCapture(videoCaptureAddress, CAP_GSTREAMER);
        }
        force = false;
    }
}

void setVideoCaptureAddressByIP(string ip) {
    string first = "rtspsrc location=rtsp://admin:Admin1401@";
    string second = ":554/streaming/channels/101 latency=200 is-live=true drop-on-latency=1 tcp-timeout=1000 teardown-timeout=1000 timeout=1000 ! rtph264depay ! h264parse ! decodebin ! autovideoconvert ! video/x-raw,format=BGRx ! videoconvert ! video/x-raw,format=BGR ! appsink drop=true sync=false";
    auto tmp = first + ip + second;
    auto ipTmp = explode(ip, '.');
    ip1 = stoi(ipTmp[0]);
    ip2 = stoi(ipTmp[1]);
    ip3 = stoi(ipTmp[2]);
    ip4 = stoi(ipTmp[3]);
    if (tmp != videoCaptureAddress) {
        videoCapture.release();
        videoCaptureAddress = tmp;
        cout << "New IP Address of video capture: " << ip << endl;
    }
}


void sendAndReceiveDataFromToThread() {

    // Create serial port object and open serial port at 57600 buad, 8 data bits, no parity bit, one stop bit (8n1),
    // and no flow control
    SerialPort serialPort = SerialPort(serialPortAddress, BaudRate::B_115200, NumDataBits::EIGHT, Parity::NONE,
                                       NumStopBits::ONE);
    // Use SerialPort serialPort("/dev/ttyACM0", 13000); instead if you want to provide a custom baud rate
    serialPort.SetTimeout(1); // Block for up to 0ms to receive data
    serialPort.Open();
    cout << "Serial port is opened" << endl;


    // WARNING: If using the Arduino Uno or similar, you may want to delay here, as opening the serial port causes
    // the micro to reset!

    // Read some data back (will block for up to 100ms due to the SetTimeout(100) call above)
    string readData, allReadData = "";
    char startChar = char(255), secondStartChar = char(254);
    bool isDataStarted = false;
    int startPosition = -1, serialLength = 0;

    while (pressedKey != 27) {
        readData.clear();
        serialPort.Read(readData);

        if (lastTouchReported != touchId || paintedFrameId % 10 == 0) {
            outputBuffer = "";
            int checksum = 0;
            //start flags
            outputBuffer += (char) 255;
            outputBuffer += (char) 254;
            //lenght of packets
            outputBuffer += (char) 18;
            outputBuffer += (char) 5;
            outputBuffer += (char) 2;
            outputBuffer += (char) 15;


            outputBuffer += (char) (radar_angle * 10) / 256;
            outputBuffer += (char) (radar_angle * 10) % 256;
            outputBuffer += (char) ((view_angle + 100) * 10) / 256;
            outputBuffer += (char) ((view_angle + 100) * 10) % 256;

            outputBuffer += (char) (vertical_angle * 10) / 256;
            outputBuffer += (char) (vertical_angle * 10) % 256;

            outputBuffer += (char) (horizental_angle * 10) / 256;
            outputBuffer += (char) (horizental_angle * 10) % 256;

            outputBuffer += (char) zoom * 10;
            outputBuffer += (char) ip1;
            outputBuffer += (char) ip2;
            outputBuffer += (char) ip3;
            outputBuffer += (char) ip4;

            outputBuffer += (char) touchedPoint.x / 256;
            outputBuffer += (char) touchedPoint.x % 256;

            outputBuffer += (char) touchedPoint.y / 256;
            outputBuffer += (char) touchedPoint.y % 256;

            checksum += (char) radar_angle / 256;
            checksum += (char) radar_angle % 256;
            checksum += (char) view_angle + 100;
            checksum += (char) vertical_angle;
            checksum += (char) horizental_angle;
            checksum += (char) zoom * 10;
            checksum += (char) ip1;
            checksum += (char) ip2;
            checksum += (char) ip3;
            checksum += (char) ip4;


            checksum += (char) touchedPoint.x / 256;
            checksum += (char) touchedPoint.x % 256;

            checksum += (char) touchedPoint.y / 256;
            checksum += (char) touchedPoint.y % 256;

            outputBuffer += (char) checksum % 256;
            //checksum

            serialPort.Write(outputBuffer);

            lastTouchReported = touchId;
        }

        allReadData.append(readData);

//        cout << "ALIVE " << endl;

        startPosition = allReadData.find(startChar);
//
//
//        cout << "START:" << (int) startPosition << endl;
//        cout << "SIZE:" << (int) allReadData.size() << endl;
//
        int lengthOfData = 20;

        if (startPosition >= 0 && allReadData.size() >= startPosition + lengthOfData &&
            allReadData[startPosition + 1] == (char) 254 &&
            allReadData[startPosition + 2] == (char) 18 &&
            allReadData[startPosition + 3] == (char) 5 &&
            allReadData[startPosition + 4] == (char) 2 &&
            allReadData[startPosition + 5] == (char) 11) {


            int startIndex = 6;


            int checksum = 0;
            for (int i = startIndex; i < 17; i++) {
                checksum += allReadData[i];
            }

            if (checksum % 256 == allReadData[16]) {
                cout << "checksum is NOT OK !!!!!" << endl;
            } else {

                radar_angle = ((double) (allReadData[startPosition + startIndex] * 256 +
                                         allReadData[startPosition + startIndex + 1])) / 10;


                view_angle = ((double) (allReadData[startPosition + startIndex + 2] * 256 +
                                        allReadData[startPosition + startIndex + 3] - 100)) / 10;


                vertical_angle = ((double) (allReadData[startPosition + startIndex + 4] * 256 +
                                            allReadData[startPosition + startIndex + 5])) / 10;


                horizental_angle = ((double) (allReadData[startPosition + startIndex + 6] * 256 +
                                              allReadData[startPosition + startIndex + 7])) / 10;

                zoom = (int) allReadData[startPosition + startIndex + 8] / 10;


                setVideoCaptureAddressByIP(
                        to_string(allReadData[startPosition + startIndex + 9]) + '.' +
                        to_string(allReadData[startPosition + startIndex + 10]) + '.' +
                        to_string(allReadData[startPosition + startIndex + 11]) + '.' +
                        to_string(allReadData[startPosition + startIndex + 12]));


//                cout << "AZIMUTH ENCODER: " << radar_angle << endl;
//                cout << "ELEVATION ENCODER: " << view_angle << endl;
//                cout << "AZIMUTH RETICLE RANGE: " << vertical_angle << endl;
//                cout << "ELEVATION RETICLE RANGE: " << horizental_angle << endl;
//                cout << "ZOOM: " << zoom << endl;
//
//                cout << "IP: " << to_string(allReadData[startPosition + startIndex + 6]) + '.' +
//                                  to_string(allReadData[startPosition + startIndex + 7]) + '.' +
//                                  to_string(allReadData[startPosition + startIndex + 8]) + '.' +
//                                  to_string(allReadData[startPosition + startIndex + 9]) << endl;


            }
            allReadData.erase(0, startPosition + lengthOfData);

        } else {
            if (allReadData.size() > 1) {
                allReadData.erase(0, startPosition + 1);
            }
        }

        if (allReadData.size() > 100) {
            this_thread::sleep_for(chrono::milliseconds(10));
        } else {
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
    serialPort.Close();
}


void writeFrameToVideoWriter() {

    if (videoWriterFileFullPath == "false") {
        cout << "Video writer is DISABLED!!!" << endl;
        return;
    }
    videoWriterFileFullPath =
            "appsrc ! video/x-raw, format=BGR ! queue ! videoconvert ! video/x-raw,format=I420 ! x264enc ! mp4mux ! filesink location=" +
            videoWriterFileFullPath + " sync=false";
    while (!videoWriter.isOpened()) {
        if (!paintedFrames[paintedFrameId].empty()) {

            cout << "Video writer starting with " << videoWriterFileFullPath << " : "
                 << paintedFrames[paintedFrameId].cols << "x" << paintedFrames[paintedFrameId].rows << endl;
            videoWriter.open(videoWriterFileFullPath, CAP_GSTREAMER, 0, (double) 30,
                             Size(paintedFrames[paintedFrameId].cols, paintedFrames[paintedFrameId].rows));
            if (!videoWriter.isOpened())
                this_thread::sleep_for(chrono::milliseconds(1000));
            else
                cout << "Video writer started" << endl;


        }

    }
    int lastFrameId = 0;
    int diff = 0;
    while (pressedKey != 27) {
        diff = paintedFrameId - lastFrameId;
        if (diff != 0) {
            lastFrameId += diff;
            videoWriter.write(paintedFrames[lastFrameId]);
        } else {
            this_thread::sleep_for(chrono::milliseconds(10));
        }
    }
    videoWriter.release();
}

long currentMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
}

void readFrameFromVideoCapture() {

    time_t now;
    tm *currentTime;
    char dateTimeChar[100];

    double ratio = 1;


    auto lastTime = currentMS();
    auto nowTime = lastTime;
    int frameCount = 0;


    Mat defaultMath(displayHeight, displayWidth, 16, Scalar(0));
    while (pressedKey != 27) {

        nowTime = currentMS();
        time(&now);
        currentTime = localtime(&now);
        strftime(dateTimeChar, 50, "%Y/%m/%d %H:%M:%S", currentTime);


        do {
            openVideoCapture(originalFrame.empty());
            videoCapture.read(originalFrame);
        } while (originalFrame.empty() && pressedKey!=27);


        if(pressedKey==27)
            continue;

        int sourceWidth = originalFrame.cols;
        int sourceHeight = originalFrame.rows;
        int width = sourceWidth, height = sourceHeight;
        int newPaintedFrameId = (paintedFrameId + 1) % 10;

        if (width > displayWidth) {
            ratio = (double) displayWidth / sourceWidth;

            width = (int) (sourceWidth * ratio);
            height = (int) (sourceHeight * ratio);
        }


        // zoom the image
        double realZoom = sqrt(zoom);

        Mat resized = originalFrame.clone();

        resized = originalFrame(Rect(
                (sourceWidth / (2 * realZoom)) * (realZoom - 1),
                (sourceHeight / (2 * realZoom)) * (realZoom - 1),
                sourceWidth - ((sourceWidth / realZoom) * (realZoom - 1)),
                sourceHeight - ((sourceHeight / realZoom) * (realZoom - 1))));

        resize(resized, frame, Size(width, height), INTER_LINEAR);

        paintedFrames[newPaintedFrameId] = defaultMath.clone();
        cv::Rect roi(cv::Point((paintedFrames[newPaintedFrameId].cols - frame.cols) / 2,
                               (paintedFrames[newPaintedFrameId].rows - frame.rows) / 2), frame.size());

        frame.copyTo(paintedFrames[newPaintedFrameId](roi));


        width = paintedFrames[newPaintedFrameId].cols;
        height = paintedFrames[newPaintedFrameId].rows;


        float half_width = width / 2.0;
        float half_height = height / 2.0;

        float quarter_width = width / 4.0;
        float quarter_height = height / 4.0;

        // Draw center lines
        line(paintedFrames[newPaintedFrameId], Point(half_width, 0), Point(half_width, height), Scalar(0, 255, 0), 1);
        line(paintedFrames[newPaintedFrameId], Point(0, half_height), Point(width, half_height), Scalar(0, 255, 0), 1);



        // draw lines on center lines
        float one_height = height / 100.0;
        float one_width = width / 100.0;

        int grid_counts = 20;
        float line_width = width / (float) grid_counts;
        float line_height = height / (float) grid_counts;

        for (int i = 1; i <= grid_counts; i++) {
            line(
                    paintedFrames[newPaintedFrameId],
                    Point(int(i * line_width), int(half_height - one_height)),
                    Point(int(i * line_width), int(half_height + one_height)),
                    Scalar(0, 255, 0),
                    1);
            line(
                    paintedFrames[newPaintedFrameId],
                    Point(int(half_width - one_width), int(i * line_height)),
                    Point(int(half_width + one_width), int(i * line_height)),
                    Scalar(0, 255, 0),
                    1);
        }

        // Draw radar circle on right top
        int circle_radius = int(quarter_height / 3);
        Point circle_center = Point(int(width - circle_radius) - 10, circle_radius + 10);

        circle(paintedFrames[newPaintedFrameId], circle_center, circle_radius,
               Scalar(0, 255, 0));

        draw_arch(
                paintedFrames[newPaintedFrameId],
                circle_center,
                circle_radius,
                // -90 is for start angle from top of the circle not the right angle of it
                (radar_angle < 0 ? (180 + paintedFrameId) : radar_angle) - 90,
                radar_size_of_angle,
                Scalar(0, 0, 255),
                2, true, 0.4, true, radar_angle);

        // putText(frame,
        //         to_string(radar_angle < 0 ? 180 + frameId : radar_angle),

        //         circle_center, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);

        // Draw view sight
        circle_radius = int(quarter_height / 2);
        circle_center = Point(int(width - circle_radius) - 10, height - circle_radius * .5);

        draw_arch(
                paintedFrames[newPaintedFrameId],
                circle_center,
                circle_radius,
                (280 + 370) / 2,
                (370 - 280) + view_size_of_angle,
                Scalar(0, 255, 0),
                1);

        int view_angle_temp = view_angle < -90 ? (paintedFrameId / 10 % 100) - 10 : view_angle;
        view_angle_temp = ((view_angle_temp + 10) % 90) - 10;

        draw_arch(
                paintedFrames[newPaintedFrameId],
                circle_center,
                circle_radius,
                10 - (view_angle_temp + 10),
                view_size_of_angle,
                Scalar(0, 0, 255),
                2, true, .7, true, view_angle_temp);

        // putText(frame,
        //         to_string(view_angle_temp),

        //         circle_center, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);

        // show horizental angle

        draw_text_center(paintedFrames[newPaintedFrameId], to_string(horizental_angle / 2),
                         Point(width - line_width, half_height),
                         FONT_HERSHEY_SIMPLEX, .7, Scalar(0, 0, 255), 2);
        draw_text_center(paintedFrames[newPaintedFrameId], to_string(-horizental_angle / 2),
                         Point(line_width, half_height), FONT_HERSHEY_SIMPLEX,
                         .7, Scalar(0, 0, 255), 2);

        // show vertical angle
        draw_text_center(paintedFrames[newPaintedFrameId], to_string(vertical_angle / 2),
                         Point(half_width, line_height), FONT_HERSHEY_SIMPLEX, .7,
                         Scalar(0, 0, 255), 2);
        draw_text_center(paintedFrames[newPaintedFrameId], to_string(-vertical_angle / 2),
                         Point(half_width, height - line_height),
                         FONT_HERSHEY_SIMPLEX, .7, Scalar(0, 0, 255), 2);

        // show zoom
        draw_text_center(paintedFrames[newPaintedFrameId],
                         to_string((int) zoom) + "." + to_string((int) ((zoom - (int) zoom) / .1) % 10) + "X",
                         Point(line_width, line_height), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);

        draw_text_center(paintedFrames[newPaintedFrameId], string(dateTimeChar), Point(4 * line_width, line_height),
                         FONT_HERSHEY_SIMPLEX, 0.5,
                         Scalar(0, 0, 255), 1);

        bool red = false;

        if (touchedPoint.x > 0 && touchedPoint.x < 2 * line_width &&
            touchedPoint.y > height - line_width && touchedPoint.y < height) {
            red = true;
            shutdown();
        }

        draw_text_vertical_center(paintedFrames[newPaintedFrameId], " Power OFF", Point(0, height - line_height),
                                  FONT_HERSHEY_SIMPLEX, .8f,
                                  red ? Scalar(0, 0, 255) : Scalar(0, 255, 0), 1);


        paintedFrameId = newPaintedFrameId;
        frameCount++;


        if (frameCount >= 30) {
            cout << "frame id: " << paintedFrameId << " Read FPS: " << frameCount / ((nowTime - lastTime) / 1000.0)
                 << endl;

            lastTime = nowTime;
            frameCount = 0;
        }
    }
    videoCapture.release();
}

void showFrameToVideoOutput() {
    int frameId = 0, frameCount = 0;

    int delta = 0;


    auto lastTime = currentMS();
    auto nowTime = lastTime;
    while (pressedKey != 27) {

        nowTime = currentMS();

        delta = paintedFrameId - frameId;
        if (delta != 0) {
            frameId += delta;
            frameCount++;
            if (!paintedFrames[frameId].empty() && paintedFrames[frameId].cols > 0 && paintedFrames[frameId].rows > 0) {
                try {
                    imshow(" ", paintedFrames[frameId]);
                }
                catch (int e) {
                    cout << "Error on showing image: " << e << endl;
                }
            }
        }

        if (frameCount > 30) {
            cout << "Show FPS: " << frameCount / ((nowTime - lastTime) / 1000.0) << " ";
            frameCount = 0;
            lastTime = nowTime;
        }
        pressedKey = waitKey(25);
    }
}


void mouseCallback(int event, int x, int y, int flags, void *userdata) {
    if (event == EVENT_LBUTTONDOWN) {
        touchedPoint = Point(x, y);
        touchId++;
        cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
}


int main(int argc, char *argv[]) {

    if (argc != 6) {

        cout << "You should insert 4 args" << endl
             << "1) Webcam IP (192.168.1.1 or etc)" << endl
             << "2) Serial port address (like: /dev/ttyTHS1 or /dev/ttyS0 or etc)" << endl
             << "3) Video display width in pixel (like 1920 or 1280 or 756 or etc)"
             << "4) Video display height in pixel (like 1080 or 800 or etc)"
             << endl
             << "5) Video output file full path or false for not save the video (for example: output.mp4 or false) "
             << endl
             << "For example:" << endl
             << "./DisplayImage 192.168.1.100 /dev/ttyTHS2 1920 1080 filename.mp4" << endl;;
        return 1;
    }

    videoWriterFileFullPath = argv[5];

    displayWidth = stoi(argv[3]);
    displayHeight = stoi(argv[4]);

    splashScreen = imread("splash.png");


    serialPortAddress = argv[2];


    namedWindow(" ", WINDOW_NORMAL);
    //For use opengl
    //namedWindow(" ", WINDOW_OPENGL);
    setMouseCallback(" ", mouseCallback, nullptr);
    setWindowProperty(" ", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);

    setVideoCaptureAddressByIP(argv[1]);

    cout << "Camera connection is opened" << endl;

    cout << "splash screen read" << endl;

    thread readFrameFromVideoCaptureThread(readFrameFromVideoCapture);
    thread serialThread(sendAndReceiveDataFromToThread);
    thread writerVideoThread(writeFrameToVideoWriter);


    showFrameToVideoOutput();


    cout<<"Join the Video thread"<<endl;
    writerVideoThread.join();
    cout<<"Join on the read thread"<<endl;
    readFrameFromVideoCaptureThread.join();
    cout<<"Join on the serial thread"<<endl;
    serialThread.join();


    return 0;
}
