#include <opencv2/opencv.hpp>
#include <math.h>

using namespace cv;

using namespace std;

#define PI 3.14159265

void draw_arch(Mat frame, Point center, int circle_radius, int view_angle, int size_of_angle, Scalar color, int thickness)
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
        Point(center.x + diff_x, center.y + diff_y),
        color, thickness);



    
}

int main()
{

    Mat frame;
    int frame_id=0;

    namedWindow("Display window");

    VideoCapture cap("/dev/video1");

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
        frame_id= frame_id + 1 % 1000;
        cap >> frame;

        int width = frame.cols;
        int height = frame.rows;

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

        // Draw circle on right top
        int circle_radius = int(quarter_height / 2);
        Point circle_center = Point(int(width - circle_radius) - 10,circle_radius + 10);
        
        circle(frame, circle_center, circle_radius,
               Scalar(0, 255, 0));

        int view_angle = 180 + frame_id;
        int size_of_angle = 30;

        draw_arch(
            frame,
            circle_center,
            circle_radius,
            view_angle,
            size_of_angle,
            Scalar(255, 255, 0),
            3);



        // Draw view sight
        circle_radius = int(quarter_height);
        circle_center = Point(int(width - circle_radius ) - 10,(quarter_height + 10) * 2);


        view_angle = 360 - 45;
        size_of_angle = 90;
        draw_arch(
            frame,
            circle_center,
            circle_radius,
            view_angle,
            size_of_angle,
            Scalar(0, 255, 0),
            1
        );
        view_angle = (280 + frame_id % 70);
        size_of_angle = 20;
        draw_arch(
            frame,
            circle_center,
            circle_radius,
            view_angle,
            size_of_angle,
            Scalar(255, 255, 0),
            3
        );





        imshow("Display window", frame);

        pressed_key = waitKey(25);
    }

    return 0;
}

