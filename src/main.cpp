#include <windows.h>

#include <iostream>
#include <string>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

static const char WINDOW_NAME[] = "gfysplit";
static const char TRACKBAR_NAME[] = "trackbar";

template <typename T>
T clamp(const T& n, const T& lower, const T& upper) {
  return std::max(lower, std::min(n, upper));
}

void showFrame(VideoCapture capture, int frameIndex)
{
    int nextIndex = (int)capture.get(CV_CAP_PROP_POS_FRAMES);
    if(nextIndex != frameIndex) {
        capture.set(CV_CAP_PROP_POS_FRAMES, (double)frameIndex);
    }
    if (capture.grab()) {
        Mat frame;
        if (capture.retrieve(frame)) {
            imshow(WINDOW_NAME, frame);
        }
    }
}

static void trackbarChanged(int newFrame, void *capturePtr)
{
    VideoCapture *capture = (VideoCapture *)capturePtr;
    showFrame(*capture, newFrame);
}

void writeOutput(const std::string &outputFilename, VideoCapture &capture, int startFrame, int endFrame)
{
    // int fourcc = (int)capture.get(CV_CAP_PROP_FOURCC);
    int fourcc = CV_FOURCC('Y', 'V', '1', '2');
    double fps = capture.get(CV_CAP_PROP_FPS);
    Size frameSize;
    frameSize.width = (int)capture.get(CV_CAP_PROP_FRAME_WIDTH);
    frameSize.height = (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT);

    VideoWriter writer;
    if(writer.open(outputFilename, -1, fps, frameSize)) {
        capture.set(CV_CAP_PROP_POS_FRAMES, (double)startFrame);
        for(int currentFrame = startFrame; currentFrame <= endFrame; ++currentFrame) {
            if (capture.grab()) {
                Mat frame;
                if (capture.retrieve(frame)) {
                    writer << frame;
                }
            }
        }
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    string inputFilename = lpCmdLine;
    std::string outputFilename = "wtf.mp4"; //inputFilename + ".out";

    VideoCapture capture(inputFilename);
    if (!capture.isOpened())
        return 0;

    int currentFrame = 0;
    int frameCount = (int)capture.get(CV_CAP_PROP_FRAME_COUNT);
    if (frameCount < 1)
        return 0;

    namedWindow(WINDOW_NAME);
    createTrackbar(TRACKBAR_NAME, WINDOW_NAME, &currentFrame, frameCount, trackbarChanged, &capture);
    showFrame(capture, currentFrame);

    bool quitRequested = false;
    while (!quitRequested) {
        int key = waitKey(0);
        switch (key) {
            case 32: // Space
                writeOutput(outputFilename, capture, 0, currentFrame);
                break;

            case -1: // All windows closed
            case 27: // Escape
                quitRequested = true;
                break;

            case 0x00250000: // Left
                currentFrame = clamp(currentFrame - 1, 0, frameCount);
                setTrackbarPos(TRACKBAR_NAME, WINDOW_NAME, currentFrame);
                break;
            case 0x00270000: // Right
                currentFrame = clamp(currentFrame + 1, 0, frameCount);
                setTrackbarPos(TRACKBAR_NAME, WINDOW_NAME, currentFrame);
                break;
        }
    }
    return 0;
}
