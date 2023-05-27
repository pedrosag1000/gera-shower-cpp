#include <opencv2/opencv.hpp>
#include <math.h>
#include <CppLinuxSerial/SerialPort.hpp>
#include <string.h>

using namespace cv;

using namespace std;

using namespace mn::CppLinuxSerial;

#define PI 3.14159265

void draw_text_center(Mat frame, string text, Point center, int fontFace, double fontScale, Scalar color, int thickness)
{
    int baseLine = 0;
    Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseLine);
    baseLine += thickness;

    Point textOrg = Point(center.x - textSize.width / 2, center.y + textSize.height / 2);
    // rectangle(frame, textOrg + Point(0, baseLine), textOrg + Point(textSize.width, -textSize.height), color);
    // line(frame, textOrg + Point(0, thickness), textOrg + Point(textSize.width, thickness), color);

    putText(frame, text, textOrg, fontFace, fontScale, color, thickness);
}

void draw_arch(Mat frame, Point center, int circle_radius, int view_angle, int size_of_angle, Scalar color, int thickness, bool drawAngle = false,double fontSize=0.5,bool overrideAngleText=false,int overrideAngle=0)
{

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

    if (drawAngle)
    {
        diff_x = int(cos((view_angle) / 180.0 * PI) * circle_radius);
        diff_y = int(sin((view_angle) / 180.0 * PI) * circle_radius);

        draw_text_center(frame, to_string(overrideAngle), center + Point(diff_x * 0.8, diff_y * 0.8), FONT_HERSHEY_SIMPLEX, fontSize, color, thickness);
    }
}

vector<string> explode(const string &s, const char &c)
{
    string buff{""};
    vector<string> v;

    for (auto n : s)
    {
        if (n != c)
            buff += n;
        else if (n == c && buff != "")
        {
            v.push_back(buff);
            buff = "";
        }
    }
    if (buff != "")
        v.push_back(buff);

    return v;
}

int main()
{

    Mat frame;
    int frame_id = 0;

    namedWindow("Display window");

    VideoCapture cap("/dev/video1");

    // Create serial port object and open serial port at 57600 buad, 8 data bits, no parity bit, one stop bit (8n1),
    // and no flow control
    SerialPort serialPort("/dev/ttyTHS2", BaudRate::B_115200, NumDataBits::EIGHT, Parity::NONE, NumStopBits::ONE);
    // Use SerialPort serialPort("/dev/ttyACM0", 13000); instead if you want to provide a custom baud rate
    serialPort.SetTimeout(1); // Block for up to 0ms to receive data
    serialPort.Open();

    // WARNING: If using the Arduino Uno or similar, you may want to delay here, as opening the serial port causes
    // the micro to reset!

    // Read some data back (will block for up to 100ms due to the SetTimeout(100) call above)
    string readData, allReadData, startString = "Start:", endString = ":End";
    bool isDataStarted = false;
    int startPosition = -1, endPosition = -1;

    int radar_angle = -100, radar_size_of_angle = 20;
    int view_angle = -100, view_size_of_angle = 20;
    int horizental_angle = 60, vertical_angle = 60;
    double zoom = 1;

    cap.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M', 'J', 'P', 'G'));

    cap.set(CAP_PROP_FRAME_WIDTH, 1080);
    cap.set(CAP_PROP_FRAME_HEIGHT, 1080);

    if (!cap.isOpened())
    {
        cout << "cannot open camera";
    }

    int pressed_key = 10;
    while (pressed_key != 27)
    {
        frame_id = frame_id + 1 % 1000;
        cap >> frame;

        int width = frame.cols;
        int height = frame.rows;

        
        // zoom the image
        

        //
        double realZoom= sqrt(zoom);

        Mat resized;
        resize(frame,resized,Size(width*realZoom,height*realZoom),INTER_LINEAR);

        frame=resized(Rect(resized.size().width/2 - width/2,resized.size().height/2 - height/2,width,height));




        int half_width = int(width / 2);
        int half_height = int(height / 2);

        int quarter_width = int(width / 4);
        int quarter_height = int(height / 4);



        // Draw center lines
        line(frame, Point(half_width, 0), Point(half_width, height), Scalar(0, 255, 0), 3);
        line(frame, Point(0, half_height), Point(width, half_height), Scalar(0, 255, 0), 3);

        // draw lines on center lines
        int one_height = int(height / 100);
        int one_width = int(width / 100);

        int grid_counts = 20;
        int line_width = width / grid_counts;
        int line_height = height / grid_counts;

        for (int i = 1; i <= grid_counts; i++)
        {
            line(
                frame,
                Point(int(i * line_width), int(half_height - one_height)),
                Point(int(i * line_width), int(half_height + one_height)),
                Scalar(0, 255, 0),
                1);
            line(
                frame,
                Point(int(half_width - one_width), int(i * line_height)),
                Point(int(half_width + one_width), int(i * line_height)),
                Scalar(0, 255, 0),
                1);
        }

        // Draw radar circle on right top
        int circle_radius = int(quarter_height / 2);
        Point circle_center = Point(int(width - circle_radius) - 10, circle_radius + 10);

        circle(frame, circle_center, circle_radius,
               Scalar(0, 255, 0));

        draw_arch(
            frame,
            circle_center,
            circle_radius,
            // -90 is for start angle from top of the circle not the right angle of it
            (radar_angle < 0 ? (180 + frame_id) : radar_angle) - 90,
            radar_size_of_angle,
            Scalar(0, 0, 255),
            2,true,0.4,true,radar_angle);

        // putText(frame,
        //         to_string(radar_angle < 0 ? 180 + frame_id : radar_angle),

        //         circle_center, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);

        // Draw view sight
        circle_radius = int(quarter_height);
        circle_center = Point(int(width - circle_radius) - 10, height - circle_radius * .5);

        draw_arch(
            frame,
            circle_center,
            circle_radius,
            (270 + 370) / 2,
            (370 - 270) + view_size_of_angle,
            Scalar(0, 255, 0),
            1);

        int view_angle_temp = view_angle < -90 ? (frame_id / 10 % 100) - 10 : view_angle;
        view_angle_temp = ((view_angle_temp + 10) % 100) - 10;

        draw_arch(
            frame,
            circle_center,
            circle_radius,
            10 - (view_angle_temp + 10),
            view_size_of_angle,
            Scalar(0, 0, 255),
            2,true,.7,true,view_angle_temp);

        // putText(frame,
        //         to_string(view_angle_temp),

        //         circle_center, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);

        // show horizental angle

        draw_text_center(frame, to_string(horizental_angle / 2), Point(width - line_width, half_height), FONT_HERSHEY_SIMPLEX, .7, Scalar(0, 0, 255), 2);
        draw_text_center(frame, to_string(-horizental_angle / 2), Point(line_width, half_height), FONT_HERSHEY_SIMPLEX, .7, Scalar(0, 0, 255), 2);

        // show vertical angle
        draw_text_center(frame, to_string(vertical_angle / 2), Point(half_width, line_height), FONT_HERSHEY_SIMPLEX, .7, Scalar(0, 0, 255), 2);
        draw_text_center(frame, to_string(-vertical_angle / 2), Point(half_width, height - line_height), FONT_HERSHEY_SIMPLEX, .7, Scalar(0, 0, 255), 2);

        // show zoom
        draw_text_center(frame, to_string((int)zoom)+"."+to_string((int)((zoom-(int)zoom)/.1)%10) + "X", Point(line_width, line_height), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);



        
        
    

        imshow("Display window", frame);

        pressed_key = waitKey(25);
        readData.clear();
        serialPort.Read(readData);

        allReadData.append(readData);

        if (!isDataStarted)
        {
            startPosition = allReadData.rfind(startString);
            if (startPosition >= 0)
            {
                allReadData = allReadData.substr(startPosition + startString.length(), allReadData.length());
                isDataStarted = true;
                startPosition = -1;
            }
        }
        else
        {
            endPosition = allReadData.rfind(endString);
            if (endPosition >= 0)
            {
                allReadData = allReadData.substr(0, endPosition);

                // process data in allReadData
                vector<string> output = explode(allReadData, ',');

                if (output.size() == 5)
                {
                    cout << "Processed data: " << allReadData << endl;
                    radar_angle = stoi(output.at(0));
                    view_angle = stoi(output.at(1));
                    vertical_angle = stoi(output.at(2));
                    horizental_angle = stoi(output.at(3));
                    zoom = stod(output.at(4));
                }
                else
                {
                    cout << "Data size is correct" << endl;
                }

                isDataStarted = false;
                endPosition = -1;
                // flush
                allReadData.clear();
            }
        }

        if (!readData.empty())
        {
            cout << "Recieved data = \"" << readData << "\"" << endl;
        }
    }

    // Close the serial port
    serialPort.Close();
    return 0;
}
