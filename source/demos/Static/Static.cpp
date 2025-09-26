#include "include/demos/Static/Static.hpp"
#include "include/render/drawing.hpp"

#include <iostream>
#include <opencv2/opencv.hpp>

int static_treshold_search(void)
{
    try {
        // read image
        cv::Mat frame = cv::imread("../resources/lightbulb.jpg");  //can be JPG,PNG,GIF,TIFF,...

        if (frame.empty())
            throw std::runtime_error("Empty file? Wrong path?");

        //start timer
        auto start = std::chrono::steady_clock::now();

        // create copy of the frame
        cv::Mat frame2;
        frame.copyTo(frame2);

        // convert to grayscale, create threshold, sum white pixels
        // compute centroid of white pixels (average X,Y coordinate of all white pixels)
        cv::Point2f center;
        cv::Point2f center_normalized;
        float white_pixel_count = 0;

        for (int y = 0; y < frame.rows; y++) //y
        {
            for (int x = 0; x < frame.cols; x++) //x
            {
                // load source pixel
                cv::Vec3b pixel = frame.at<cv::Vec3b>(y, x);

                // compute temp grayscale value (convert from colors to Y)
                unsigned char Y = 0.114 * pixel[0] + 0.587 * pixel[1] + 0.299 * pixel[2]; // BGR


                // FIND THRESHOLD (value 0..255)
                if (Y < 240) {
                    // set output pixel black
                    frame2.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
                }
                else {
                    // set output pixel white
                    frame2.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);

                    //update centroid...
                    center.x += x;
                    center.y += y;
                    white_pixel_count += 1;
                }

            }
        }


        std::cout << "Center absolute: " << center << '\n';
        std::cout << "Center normalized: " << center_normalized << '\n';

        // how long the computation took?
        auto end = std::chrono::steady_clock::now();

        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time: " << elapsed_seconds.count() << "sec" << std::endl;

        if (white_pixel_count > 0) {
            center.x /= white_pixel_count;
            center.y /= white_pixel_count;

            center_normalized.x = center.x;
            center_normalized.y = center.y;

            center_normalized.x /= frame.cols;
            center_normalized.y /= frame.rows;
        }

        // highlight the center of object
        draw_cross(frame, center.x, center.y, 25);
        draw_cross_normalized(frame2, center_normalized, 25);

        // show me the result
        cv::namedWindow("frame");
        cv::imshow("frame", frame);

        cv::namedWindow("frame2");
        cv::imshow("frame2", frame2);

        // keep application open until ESC is pressed
        while (true)
        {
            int key = cv::pollKey(); // poll OS events (key press, mouse move, ...)
            if (key == 27) // test for ESC key
                break;
        }

    }
    catch (std::exception const& e) {
        std::cerr << "App failed : " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
