#include <iostream>
#include <numeric>
#include <vector>
#include <array>
#include <thread>

#include <opencv2/opencv.hpp>

#include "runners/RasterApp.hpp"

RasterApp::RasterApp()
{
    std::cout << "Constructed...\n";
}

bool RasterApp::init()
{
    try {
        capture = cv::VideoCapture(0, cv::CAP_ANY);
        if (!capture.isOpened())
            throw std::runtime_error("Can not open camera :(");

        std::cout << "Cam opened successfully.\n";

    }
    catch (std::exception const& e) {
        std::cerr << "Init failed : " << e.what() << std::endl;
        throw;
    }
    std::cout << "Initialized...\n";
    return true;
}

std::vector<uchar> RasterApp::lossy_bw_limit(cv::Mat& input_img, size_t size_limit)
{
    std::string suff(".jpg"); // target format
    if (!cv::haveImageWriter(suff))
        throw std::runtime_error("Can not compress to format:" + suff);

    std::vector<uchar> bytes;
    std::vector<int> compression_params;

    // prepare parameters for JPEG compressor
    // we use only quality, but other parameters are available (progressive, optimization...)
    std::vector<int> compression_params_template;
    compression_params_template.push_back(cv::IMWRITE_JPEG_QUALITY);

    std::cout << '[';

    //try step-by-step to decrease quality by 5%, until it fits into limit
    for (auto i = 100; i > 0; i -= 5) {
        compression_params = compression_params_template; // reset parameters
        compression_params.push_back(i);                  // set desired quality
        std::cout << i << ',';

        // try to encode
        cv::imencode(suff, input_img, bytes, compression_params);

        // check the size limit
        if (bytes.size() <= size_limit)
            break; // ok, done 
    }
    std::cout << "]\n";

    return bytes;
}

double psnr(const cv::Mat& original, const cv::Mat& decompressed) {
    cv::Mat difference;
    cv::absdiff(original, decompressed, difference);
    difference.convertTo(difference, CV_32F);
    difference = difference.mul(difference);

    cv::Scalar channel_sums = cv::sum(difference);
    double sum = channel_sums.val[0] + channel_sums.val[1] + channel_sums.val[2];

    if (sum <= 1e-10) // very small difference
        return 100;

    double mse = sum / (double)(original.channels() * original.total());
    double psnr = 10.0 * log10((255 * 255) / mse);
    return psnr;
}

std::vector<uchar> RasterApp::lossy_quality_limit(const cv::Mat& frame, const float target_coefficient)
{
    std::string suff(".jpg");
    if (!cv::haveImageWriter(suff))
        throw std::runtime_error("Cannot compress to format: " + suff);

    std::vector<uchar> bytes;
    std::vector<int> compression_params = { cv::IMWRITE_JPEG_QUALITY, 100 };

    double target_psnr = target_coefficient * 50.0;
    std::cout << "Target PSNR: " << target_psnr << " dB\n";

    for (int q = 100; q >= 5; q -= 5) {
        compression_params[1] = q;
        cv::imencode(suff, frame, bytes, compression_params);
        cv::Mat decoded = cv::imdecode(bytes, cv::IMREAD_COLOR);

        double p = psnr(frame, decoded);
        std::cout << "Quality=" << q << " - PSNR=" << p << " dB\b";

        if (p <= target_psnr)
            break;
    }
    return bytes;
}

int RasterApp::run(void)
{
    cv::Mat frame;
    std::vector<uchar> bytes;
    float target_coefficient = 1.0f; // used as size-ratio, or quality-coefficient
    try {
        while (capture.isOpened())
        {
            capture >> frame;

            if (frame.empty()) {
                std::cerr << "device closed (or video at the end)" << '\n';
                capture.release();
                break;
            }

            // encode image with bandwidth limit
            auto size_uncompressed = frame.elemSize() * frame.total();
            auto size_compressed_limit = size_uncompressed * target_coefficient;

            // Encode single image with limitation by bandwidth (encoded to original datasize ratio in 0.0 - 1.0)
            //bytes = lossy_bw_limit(frame, size_compressed_limit); // returns JPG compressed stream for single image
            if (bandwidth) {
                bytes = lossy_bw_limit(frame, size_compressed_limit);
            }
            else {
                bytes = lossy_quality_limit(frame, target_coefficient);
            }


            // display compression ratio
            auto size_compreessed = bytes.size();
            std::cout << "Size: uncompressed = " << size_uncompressed << ", compressed = " << size_compreessed << ", = " << size_compreessed / (size_uncompressed / 100.0) << " % \n";

            //
            // decode and display compressed and original data
            //  
            cv::Mat decoded_frame = cv::imdecode(bytes, cv::IMREAD_ANYCOLOR);

            cv::namedWindow("original");
            cv::imshow("original", frame);

            cv::namedWindow("decoded");
            cv::imshow("decoded", decoded_frame);

            // key handling
            int c = cv::pollKey();
            switch (c) {
            case 27:
                return EXIT_SUCCESS;
                break;
            case 'q':
                target_coefficient += 0.03;
                break;
            case 'a':
                target_coefficient -= 0.03;
                break;
            case 'e':
                bandwidth = !bandwidth;
                std::cout << "Using bandwidth limit: " << bandwidth << std::endl;
                break;
            default:
                break;
            }

            target_coefficient = std::clamp(target_coefficient, 0.01f, 1.0f);
            std::cout << "Target coeff: " << target_coefficient * 100.0f << " dB\n";
        }
    }
    catch (std::exception const& e) {
        std::cerr << "App failed : " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Finished OK...\n";
    return EXIT_SUCCESS;
}

RasterApp::~RasterApp()
{
    // clean-up
    cv::destroyAllWindows();
    std::cout << "Bye...\n";
}

