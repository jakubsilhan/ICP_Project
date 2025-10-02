#include "include/recognizers/FaceRecognizer.hpp"
#include "include/recognizers/RedRecognizer.hpp"


int main()
{
    // Helloworld
    //runProject();

    // Static
    //static_treshold_search();

    // HSV
    //runHsv();

    // Red Cup image
    //RedRecognizer recognizer;
    //recognizer.run_static();

    // Red Cup video
    RedRecognizer recognizer;
    recognizer.run_video("resources/video.mkv");

    // Face
    //FaceRecognizer recognizer;
    //// Initialize classifier and camera
    //if (!recognizer.init()) {
    //    std::cerr << "Failed to initialize FaceRecognizer." << std::endl;
    //    return -1;
    //}

    //// Run face detection loop
    //int result = recognizer.run();
    //if (result != 0) {
    //    std::cerr << "FaceRecognizer encountered an error." << std::endl;
    //}

    return 0;
}
