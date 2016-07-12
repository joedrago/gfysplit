#include <windows.h>
#include <string>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

struct SimpleBitmap
{
    bool valid_;
    HBITMAP handle_;
    int width_;
    int height_;
};

SimpleBitmap bmp_ = { false };

int convertOpenCVBitDepthToBits(const int value)
{
    auto regular = 0u;

    switch (value)
    {
    case CV_8U:
    case CV_8S:
        regular = 8u;
        break;

    case CV_16U:
    case CV_16S:
        regular = 16u;
        break;

    case CV_32S:
    case CV_32F:
        regular = 32u;
        break;

    case CV_64F:
        regular = 64u;
        break;

    default:
        regular = 0u;
        break;
    }

    return regular;
};

SimpleBitmap ConvertCVMatToBMP(cv::Mat frame)
{
    SimpleBitmap simpleBMP = { false };
    auto imageSize = frame.size();
    assert(imageSize.width && "invalid size provided by frame");
    assert(imageSize.height && "invalid size provided by frame");

    if (imageSize.width && imageSize.height)
    {
        BITMAPINFOHEADER headerInfo;
        ZeroMemory(&headerInfo, sizeof(headerInfo));

        headerInfo.biSize     = sizeof(headerInfo);
        headerInfo.biWidth    = imageSize.width;
        headerInfo.biHeight   = -(imageSize.height); // negative otherwise it will be upsidedown
        headerInfo.biPlanes   = 1;// must be set to 1 as per documentation frame.channels();

        const auto bits       = convertOpenCVBitDepthToBits( frame.depth() );
        headerInfo.biBitCount = frame.channels() * bits;

        BITMAPINFO bitmapInfo;
        ZeroMemory(&bitmapInfo, sizeof(bitmapInfo));

        bitmapInfo.bmiHeader              = headerInfo;
        bitmapInfo.bmiColors->rgbBlue     = 0;
        bitmapInfo.bmiColors->rgbGreen    = 0;
        bitmapInfo.bmiColors->rgbRed      = 0;
        bitmapInfo.bmiColors->rgbReserved = 0;

        auto dc  = GetDC(nullptr);
        assert(dc != nullptr && "Failure to get DC");
        auto bmp = CreateDIBitmap(dc,
                                  &headerInfo,
                                  CBM_INIT,
                                  frame.data,
                                  &bitmapInfo,
                                  DIB_RGB_COLORS);
        assert(bmp != nullptr && "Failure creating bitmap from captured frame");

        simpleBMP.valid_ = true;
        simpleBMP.handle_ = bmp;
        simpleBMP.width_ = imageSize.width;
        simpleBMP.height_ = imageSize.height;
        return simpleBMP;
    }

    return simpleBMP;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;
    RECT clientRect;

    switch (message)
    {
    // case WM_COMMAND:
    //     wmId    = LOWORD(wParam);
    //     wmEvent = HIWORD(wParam);
    //     // Parse the menu selections:
    //     switch (wmId)
    //     {
    //     case IDM_ABOUT:
    //         DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
    //         break;
    //     case IDM_EXIT:
    //         DestroyWindow(hWnd);
    //         break;
    //     default:
    //         return DefWindowProc(hWnd, message, wParam, lParam);
    //     }
    //     break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &clientRect);
        BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, NULL, 0, 0, BLACKNESS);
        if(bmp_.valid_) {
            int dstX = 0;
            int dstY = 0;
            int dstW = clientRect.right;
            int dstH = clientRect.bottom;

            float winAspectRatio = (float)dstW / (float)dstH;
            float vidAspectRatio = (float)bmp_.width_ / (float)bmp_.height_;
            if(winAspectRatio < vidAspectRatio) {
                dstH = dstW / vidAspectRatio;
            } else {
                dstW = dstH * vidAspectRatio;
            }
            dstX = (clientRect.right - dstW) >> 1;
            dstY = (clientRect.bottom - dstH) >> 1;

            HDC bmpDC = CreateCompatibleDC(hdc);
            HBITMAP oldBMP = (HBITMAP)SelectObject(bmpDC, bmp_.handle_);
            SetStretchBltMode(hdc, HALFTONE);
            StretchBlt(hdc,
                dstX, dstY, dstW, dstH,
                bmpDC,
                0, 0, bmp_.width_, bmp_.height_,
                SRCCOPY);
            SelectObject(bmpDC, oldBMP);
            DeleteDC(bmpDC);
        }
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = NULL;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = "gfysplit";
    wcex.hIconSm        = NULL;
    RegisterClassEx(&wcex);

    std::string arg = "C:\\Users\\joe\\Videos\\dave.mp4";
    VideoCapture capture(arg);
    if (!capture.isOpened())
        return FALSE;

    if(capture.grab()) {
        Mat frame;
        if(capture.retrieve(frame)) {
            bmp_ = ConvertCVMatToBMP(frame);
        }
    }

    HWND hWnd = CreateWindow("gfysplit", "GFYSplit", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    if (!hWnd)
        return FALSE;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        // if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return 0;
}
