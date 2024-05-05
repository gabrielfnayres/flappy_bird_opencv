#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>

using namespace std;
using namespace cv;

class Menu
{ 
    public:
        void iniciarHistoria(Mat smallImg)
        {
            putText	(smallImg, "Para iniciar o jogo clique em Y:", Point(300, 50), FONT_HERSHEY_PLAIN, 2, Scalar(255,0,0)); // fonte
            putText	(smallImg, "Para sair do jogo clique em Q:", Point(300, 50), FONT_HERSHEY_PLAIN, 2, Scalar(255,0,0)); // fonte
        }
};

int aux = 1;
int auy = 1;
int x = 6;
int y = 6;
double vida = 100000;
char resp;
 //int points = 10;
 //int fra = 20;

void detectAndDraw( Mat& img, CascadeClassifier& cascade, double scale, bool tryflip);

string cascadeName;

int main( int argc, const char** argv )
{
    Menu menu;
    VideoCapture capture;
    Mat frame;
    bool tryflip;
    CascadeClassifier cascade;
    double scale;

    cascadeName = "haarcascade_frontalface_default.xml";
    scale = 1; // usar 1, 2, 4.
    if (scale < 1)
        scale = 1;
    tryflip = true;

    if (!cascade.load(cascadeName)) {
        cerr << "ERROR: Could not load classifier cascade: " << cascadeName << endl;
        return -1;
    }

    if(!capture.open(0)) // para testar com um video
    //if(!capture.open(0)) // para testar com a webcam
    {
        cout << "Capture from camera #0 didn't work" << endl;
        return 1;
    }

    if( capture.isOpened() ) {
        cout << "Video capturing has been started ..." << endl;

        cin >> resp;
        if(resp == 121)
        {
            while (1)
            {
                capture >> frame;
                if( frame.empty() )
                    break;

                char c = (char)waitKey(10);
                if( c == 27 || c == 'q' || c == 'Q' )
                    break;

                //menu.iniciarHistoria();
                //cin>> resp;
                //if(resp == 121)
                    detectAndDraw( frame, cascade, scale, tryflip );            
             }
        }

       
    }

    return 0;
}

/**
 * @brief Draws a transparent image over a frame Mat.
 * 
 * @param frame the frame where the transparent image will be drawn
 * @param transp the Mat image with transparency, read from a PNG image, with the IMREAD_UNCHANGED flag
 * @param xPos x position of the frame image where the image will start.
 * @param yPos y position of the frame image where the image will start.
 */
void drawTransparency(Mat frame, Mat transp, int xPos, int yPos) {
    Mat mask;
    vector<Mat> layers;

    split(transp, layers); // seperate channels
    Mat rgb[3] = { layers[0],layers[1],layers[2] };
    mask = layers[3]; // png's alpha channel used as mask
    merge(rgb, 3, transp);  // put together the RGB channels, now transp insn't transparent 
    transp.copyTo(frame.rowRange(yPos, yPos + transp.rows).colRange(xPos, xPos + transp.cols), mask);
}

/**
 * @brief Draws a transparent rect over a frame Mat.
 * 
 * @param frame the frame where the transparent image will be drawn
 * @param color the color of the rect
 * @param alpha transparence level. 0 is 100% transparent, 1 is opaque.
 * @param regin rect region where the should be positioned
 */
void drawTransRect(Mat frame, Scalar color, double alpha, Rect region) {
    Mat roi = frame(region);
    Mat rectImg(roi.size(), CV_8UC3, color); 
    addWeighted(rectImg, alpha, roi, 1.0 - alpha , 0, roi); 
}

void drawCircularImage(Mat bg, Mat dest, int &shift) {
    if (shift > dest.cols)
        shift = 0;
    printf("bg::width: %d, height=%d\n", bg.cols, bg.rows );
    printf("smallImg::width: %d, height=%d\n", dest.cols, dest.rows );
    Rect crop1(shift, 0, bg.cols - shift, bg.rows);
    Mat bg1 = bg(crop1);
    Rect crop2(0, 0, shift, bg.rows);
    Mat bg2 = bg(crop2);
    printf("bg1::width: %d, height=%d\n", bg1.cols, bg1.rows );
    printf("bg2::width: %d, height=%d\n", bg2.cols, bg2.rows );
    printf("shift = %d \n", shift);
    if (bg1.cols > 0)
        bg1.copyTo(dest.rowRange(0, bg1.rows).colRange(0, bg1.cols));
    if (bg2.cols > 0)
	bg2.copyTo(dest(cv::Rect(dest.cols - shift, 0, bg2.cols, bg2.rows)));
}

void detectAndDraw( Mat& img, CascadeClassifier& cascade, double scale, bool tryflip)
{
    double t = 0;
    vector<Rect> faces;
    Mat gray, smallImg;
    Scalar color = Scalar(255,0,0);

    double fx = 1 / scale;
    resize( img, smallImg, Size(), fx, fx, INTER_LINEAR_EXACT );
    if( tryflip )
        flip(smallImg, smallImg, 1);
    cvtColor( smallImg, gray, COLOR_BGR2GRAY );
    equalizeHist( gray, gray );

    t = (double)getTickCount();
    static int x = 0;
    // Desenha BG
    Mat bgbig = cv::imread("flap.png", IMREAD_UNCHANGED);
    Mat bg;
    resize( bgbig, bg, Size(smallImg.cols, smallImg.rows), 1, 1, INTER_LINEAR_EXACT );
    drawCircularImage(bg, smallImg, x);
    x+=20;

    cascade.detectMultiScale( gray, faces,
        1.3, 2, 0
        //|CASCADE_FIND_BIGGEST_OBJECT
        //|CASCADE_DO_ROUGH_SEARCH
        |CASCADE_SCALE_IMAGE,
        Size(40, 40) );
    t = (double)getTickCount() - t;
    printf( "detection time = %g ms\n", t*1000/getTickFrequency());
    
    // Desenha uma imagem
    Mat orange = cv::imread("../data/orange.png", IMREAD_UNCHANGED);
    Rect orangeRect = Rect(y, x, orange.cols -30, orange.rows-30);
    drawTransparency(smallImg, orange, x, y);
    printf("orang::width: %d, height=%d\n", orange.cols, orange.rows);
    x+=(5*aux);
    y+=(5*auy);

    if((x > 532)||(x < 5))
    {
        aux = -aux;
    }

    if((y > 390)||(y < 5))
    {
        auy = -auy;
    }

    // PERCORRE AS FACES ENCONTRADAS
   
    
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        Rect r = faces[i];
        if((r & orangeRect).area() > 10)
            color = Scalar(0,0,255);
        else
        {   
            //vida-=10;

           //putText	(smallImg, "Vida:", Point(30, 500), FONT_HERSHEY_PLAIN, 2, color); // fonte
            //for(int i=9; i=!0; i--){
             //putText	(smallImg, "*", Point(50 + fra, 500), FONT_HERSHEY_PLAIN, 2, color); // fonte
             //fra= fra+30;
           // }
             
            //points--;
            //cout << "Tá dando dano!" << endl;
            //if(vida < 0)
           // {
           //     putText	(smallImg, "GAME OVER!", Point(300, 200), FONT_HERSHEY_PLAIN, 2, Scalar(254,254,254)); // fonte
           // }*/
            //color = Scalar(255,0,0);
        }
        rectangle( smallImg, Point(cvRound(r.x), cvRound(r.y)), Point(cvRound((r.x + r.width-1)), cvRound((r.y + r.height-1))), color, 3);
    }

    /* Desenha quadrados com transparencia
    double alpha = 0.3;
    drawTransRect(smallImg, Scalar(0,255,0), alpha, Rect(  0, 0, 200, 200));
    drawTransRect(smallImg, Scalar(255,0,0), alpha, Rect(200, 0, 200, 200));*/

    // Desenha um texto
    color = Scalar(0,0,255);
    putText	(smallImg, "Placar:", Point(300, 50), FONT_HERSHEY_PLAIN, 2, color); // fonte

    // Desenha o frame na tela
    imshow("result", smallImg );
    printf("image::width: %d, height=%d\n", smallImg.cols, smallImg.rows );
}
