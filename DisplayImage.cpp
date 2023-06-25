#include <opencv2/opencv.hpp>
#include <math.h>
#include <CppLinuxSerial/SerialPort.hpp>
#include <string.h>
#include <ctime>

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

void draw_arch(Mat frame, Point center, int circle_radius, int view_angle, int size_of_angle, Scalar color, int thickness, bool drawAngle = false, double fontSize = 0.5, bool overrideAngleText = false, int overrideAngle = 0)
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

int findCharInString(const string &s,const char &c){
	for(int i=0;i<s.length();i++)
		if(s[i]==c)
			return i;
	return -1;
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

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        
        cout << "You sould insert 4 args" << endl
             << "1) Webcam address (like 0 or 1, or dev/video1 or rstp://192.168.1.1 or etc)" << endl
             << "2) Serial port address (like: /dev/ttyTHS1 or /dev/ttyS0 or etc)" << endl
             << "3) Video display width in pixel - this number should be less than webcam width (like 1920 or 1080 or 756 or etc)" << endl
	     << "4) Video output file (GStreamer) " << endl
             << "For example:" << endl
             << "./DisplayImage /dev/video1 /dev/ttyTHS2 1920 1080" << endl;
        ;
        return 1;
    }

    int display_width = stoi(argv[3]);

    Mat frame;
    int frame_id = 0;

    // Create serial port object and open serial port at 57600 buad, 8 data bits, no parity bit, one stop bit (8n1),
    // and no flow control
    SerialPort serialPort(argv[2], BaudRate::B_115200, NumDataBits::EIGHT, Parity::NONE, NumStopBits::ONE);
    // Use SerialPort serialPort("/dev/ttyACM0", 13000); instead if you want to provide a custom baud rate
    serialPort.SetTimeout(1); // Block for up to 0ms to receive data
    serialPort.Open();
    cout << "Serial port is opened" << endl;

    // WARNING: If using the Arduino Uno or similar, you may want to delay here, as opening the serial port causes
    // the micro to reset!

    // Read some data back (will block for up to 100ms due to the SetTimeout(100) call above)
    string readData, allReadData;
    char  startChar = char(255), endChar = char(254);
    bool isDataStarted = false;
    int startPosition = -1, endPosition = -1;

    int radar_angle = -100, radar_size_of_angle = 20;
    int view_angle = -100, view_size_of_angle = 20;
    int horizental_angle = 60, vertical_angle = 60;
    double zoom = 1;

    namedWindow(" ");
    cout<<"Video address: " << argv[1]<<endl; 
    VideoCapture cap(argv[1],CAP_GSTREAMER);
    cout << "Camera connection is oppened" << endl;
    
    if (!cap.isOpened())
    {
        cout << "Error: cannot open camera" << endl;
        return 0;
    }

    


    int pressed_key = 10;

    cap >> frame;
    int width = frame.cols;
    int height = frame.rows;


    if(width>display_width){
	double ratio=(double)display_width / width;
	Mat newFrame;
        resize(frame, newFrame, Size((int)(width*ratio), (int)(height*ratio)), INTER_LINEAR);
	frame = newFrame;
	width=(int)(width*ratio);
	height=(int)(height*ratio);
    }
    time_t now;
    tm* currentTime;
    char dateTimeChar[100];
    cout<<"Video writer started with : "<<width<<"x"<<height<<endl;

    VideoWriter video;
    video.open(argv[4],CAP_GSTREAMER,0,(double)10,Size(width,height));

    while (pressed_key != 27)
    {
	long long tickCount=getTickCount();
	time(&now);
	currentTime=localtime(&now);
        strftime(dateTimeChar,50,"%Y/%m/%d %H:%M:%S",currentTime);

	
        frame_id = (frame_id + 1) % 360;
        cap >> frame;
    
        // zoom the image
        double realZoom = sqrt(zoom);

        Mat resized = frame(Rect(
            (width / (2 * realZoom)) * (realZoom - 1),
            (height / (2 * realZoom)) * (realZoom - 1),
            width - ((width / realZoom) * (realZoom - 1)),
            height - ((height / realZoom) * (realZoom - 1))));

        Mat newFrame;
        resize(resized, newFrame, Size(width, height), INTER_LINEAR);
        frame = newFrame;

        float half_width = width / 2.0;
        float half_height = height / 2.0;

        float quarter_width = width / 4.0;
        float quarter_height = height / 4.0;

        // Draw center lines
        line(frame, Point(half_width, 0), Point(half_width, height), Scalar(0, 255, 0), 1);
        line(frame, Point(0, half_height), Point(width, half_height), Scalar(0, 255, 0), 1);

    

        // draw lines on center lines
        float one_height = height / 100.0;
        float one_width = width / 100.0;

        int grid_counts = 20;
        float line_width = width / (float)grid_counts;
        float line_height = height / (float)grid_counts;

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
        int circle_radius = int(quarter_height / 3);
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
            2, true, 0.4, true, radar_angle);

        // putText(frame,
        //         to_string(radar_angle < 0 ? 180 + frame_id : radar_angle),

        //         circle_center, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);

        // Draw view sight
        circle_radius = int(quarter_height / 2);
        circle_center = Point(int(width - circle_radius) - 10, height - circle_radius * .5);

        draw_arch(
            frame,
            circle_center,
            circle_radius,
            (280 + 370) / 2,
            (370 - 280) + view_size_of_angle,
            Scalar(0, 255, 0),
            1);

        int view_angle_temp = view_angle < -90 ? (frame_id / 10 % 100) - 10 : view_angle;
        view_angle_temp = ((view_angle_temp + 10) % 90) - 10;

        draw_arch(
            frame,
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

        draw_text_center(frame, to_string(horizental_angle / 2), Point(width - line_width, half_height), FONT_HERSHEY_SIMPLEX, .7, Scalar(0, 0, 255), 2);
        draw_text_center(frame, to_string(-horizental_angle / 2), Point(line_width, half_height), FONT_HERSHEY_SIMPLEX, .7, Scalar(0, 0, 255), 2);

        // show vertical angle
        draw_text_center(frame, to_string(vertical_angle / 2), Point(half_width, line_height), FONT_HERSHEY_SIMPLEX, .7, Scalar(0, 0, 255), 2);
        draw_text_center(frame, to_string(-vertical_angle / 2), Point(half_width, height - line_height), FONT_HERSHEY_SIMPLEX, .7, Scalar(0, 0, 255), 2);

        // show zoom
        draw_text_center(frame, to_string((int)zoom) + "." + to_string((int)((zoom - (int)zoom) / .1) % 10) + "X", Point(line_width, line_height), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);

	draw_text_center(frame, string(dateTimeChar) , Point(4*line_width,line_height), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 1);
	
        	
	
	video.write(frame);

        imshow(" ", frame);


        pressed_key = waitKey(1);
        readData.clear();
        serialPort.Read(readData);

        allReadData.append(readData);

        if (!isDataStarted)
        {
            startPosition = allReadData.find(startChar);
            if (startPosition >= 0)
            {
                allReadData = allReadData.substr(startPosition + 1, allReadData.length());
                isDataStarted = true;
                startPosition = -1;
            }
        }
        else
        {
            endPosition = allReadData.find(endChar);
		
            if (endPosition >= 0)
            {
                allReadData = allReadData.substr(0, endPosition);

                // process data in allReadData
 

	
		//radar 2 byte
		//view angle 1 byte
		//vertical angle 1 byte
		//horizental angle 1 byte
		//zoom 1 byte
		if(allReadData.length() == 6){
			
		 
    		    radar_angle=allReadData[0]*256 + allReadData[1];
			
                 //   cout << "Processed data: " << allReadData << endl;
			
                    view_angle = (int)allReadData[2]-100;
                    vertical_angle = (int)allReadData[3];
                    horizental_angle = (int)allReadData[4];
                    zoom = (int)allReadData[5] / 10;
                    if(zoom < 1)
                       zoom=1;
                }
                else
                {
                    cout << "Data size is not correct" << endl;
                }

                isDataStarted = false;
                endPosition = -1;
                // flush
                allReadData.clear();
            }
	    else{
		cout<< "waiting for data to be finished" <<endl;
	    }
        }

        if (!readData.empty())
        {
//	    for(int i=0; i< readData.length();i++){
//		cout<< readData[i] << " | "<<(int)readData[i]<<endl;
//	    }
 //           cout << "Recieved data = \"" << readData << "\"" << endl;
        }

	if(frame_id % 30 ==0){
		cout<<"FPS "<<getTickFrequency() / (getTickCount() - tickCount)<<endl;
	}
    }

    // Close the serial port
    serialPort.Close();
    cap.release();
    video.release();
    return 0;
}
