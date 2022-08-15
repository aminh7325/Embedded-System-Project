#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include<opencv2/opencv.hpp>
#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include <stdio.h>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "MQTTClient.h"


#define ADDRESS     "localhost"
#define CLIENTID    "Send_End"

using namespace cv;
using namespace std;

static int Last = 0;
static int counter=0;

void publish(MQTTClient client, char* topic, char* payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen((char*)pubmsg.payload);
    pubmsg.qos = 0;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, 1000L);
    printf("Message '%s' with delivery token %d delivered\n", payload, token);
}


int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payload = (char*)message->payload;
    counter++;
    printf("%d)Received operation %s\n", counter, payload);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}


static char *device = "default";
short buffer[8*1024];
int buffer_size = sizeof(buffer) >> 1;
double RMS(short *buffer) {
    int i;
    long int SquareSum = 0.0;
    for(i=0; i<buffer_size; i++)
        SquareSum += (buffer[i] * buffer[i]);

    double rms = sqrt(SquareSum/buffer_size);
    return rms;
}


int detectAndDisplay( Mat frame );
CascadeClassifier face_cascade;

int main( )
{	

    cv::VideoCapture camera(0);
    if (!camera.isOpened()) {
        std::cerr << "ERROR: Could not open camera" << std::endl;
        return 1;
    }

    if( !face_cascade.load( "/home/aminh7325/EmbSys/OpenCV-20220619/OpenCV/opencv/data/haarcascades/haarcascade_frontalcatface.xml" ) )
    {
        cout << "--(!)Error loading face cascade\n";
        return -1;
    };
    
    FILE *fp;
    char St[100];
    char StTemp[100];
    char StLoad[100];
    char StLoadF[100];
    time_t now = time(0);
    char* DateTime = ctime(&now);
    int Cpu0, Cpu1, Cpu2, Idle, Load;
    
    snd_pcm_t *handle_capture;
    snd_pcm_sframes_t frames;
    char StAudio[200];
    char Temp[100];
    
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.username = "aminh7325";
    conn_opts.password = "AminHaji";

    MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL);
    
    //namedWindow("Face");
	Mat frame;
	int temp;
	int NumberofFaces;
	int OldNumber=0;
	int rc;
	int Err;

    Err = snd_pcm_open(&handle_capture, device, SND_PCM_STREAM_CAPTURE, 0);
    if(Err < 0) {
        printf("Capture open error: %s\n", snd_strerror(Err));
        exit(EXIT_FAILURE);
    }

    Err = snd_pcm_set_params(handle_capture,
                             SND_PCM_FORMAT_S16_LE,
                             SND_PCM_ACCESS_RW_INTERLEAVED,
                             1,
                             48000,
                             1,
                             500000);
    if(Err < 0) {
        printf("Capture open error: %s\n", snd_strerror(Err));
        exit(EXIT_FAILURE);
    }

    	double k = 0.45255;
    	double PValue = 0;
    	double peak = 0;
	
	
	
	
	while(1){
 
        frames = snd_pcm_readi(handle_capture, buffer, buffer_size);

        if(frames < 0) {
            frames = snd_pcm_recover(handle_capture, frames, 0);
            if(frames < 0) {
                printf("snd_pcm_readi failed: %s\n", snd_strerror(Err));
            }
        }

        if(frames > 0 && frames < (long)buffer_size) {
            printf("Short read (expected %li, read %li)\n", (long)buffer_size, frames);
        }
        
        PValue = RMS(buffer) * k;
        if(PValue > peak)
            peak = PValue;
            
        if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
            printf("Failed to connect, return code %d\n", rc);
            exit(-1);
        }
        
        //FaceDetect Publish
	camera >> frame;
	NumberofFaces = detectAndDisplay(frame);
	if (NumberofFaces != OldNumber){
		time_t now = time(0);
        	DateTime = ctime(&now);
                sprintf(St, "%d, %s", NumberofFaces, DateTime);
                OldNumber = NumberofFaces;
    		if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        		printf("Failed to connect, return code %d\n", rc);
            		exit(-1);
        	}

        	publish(client, "sensors/Faces", St);
        	printf("Face Sensors Publish Success!\n");
	
		}
	// Audio Publish	
	time_t now = time(0);
        DateTime = ctime(&now);
        if((int)(20 * log10(PValue)) > 35) {  
            sprintf(StAudio,"%d,%s", (int)(20 * log10(PValue)), DateTime);
            publish(client , "sensors/Audio", StAudio);
            printf("Audio Sensors Publish Success!\n");
        }
        
        // Load and Temp publish
        
        fp = fopen("/proc/stat","r");
        // CPU Word
        fscanf(fp, "%s ", StLoad);
        // First Number
        fscanf(fp, "%s ", StLoad);
        sscanf(StLoad, "%d", &Cpu0);
        //Second Number
        fscanf(fp, "%s ", StLoad);
        sscanf(StLoad, "%d", &Cpu1);
        //Third Number
        fscanf(fp, "%s ", StLoad);
        sscanf(StLoad, "%d", &Cpu2);
        // Idle time
        fscanf(fp, "%s ", StLoad);
        sscanf(StLoad, "%d", &Idle);
        fclose(fp);

        Load = ((Cpu0+Cpu1+Cpu2)/(Idle/100));
        sprintf(StLoadF, "%d", Load);
        //int tempCPU=0;
        //fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
	//fscanf(fp, "%s ", StTemp);
	//sscanf(StTemp, "%d", &tempCPU);
 	temp = 40;
        temp = temp + rand()/500000000;
        sprintf(StTemp, "%d", temp);
        publish(client, "sensors/Temp", StTemp);
        printf("Temp Sensors Publish Success!\n");
        publish(client, "sensors/Load", StLoadF);
        printf("Load Sensors Publish Success!\n");
    	MQTTClient_disconnect(client, 100);
    	
	}
	
	
    snd_pcm_close(handle_capture);
    return 0;
}


int detectAndDisplay( Mat frame )
{

    Mat frame_gray;
    cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );
    std::vector<Rect> faces;
    face_cascade.detectMultiScale( frame_gray, faces );	
    for ( size_t i = 0; i < faces.size(); i++ )
    {

	 cv::putText(frame, 
            to_string(i+1),
            cv::Point(faces[i].x+(faces[i].width/2),faces[i].y - 10), 
            cv::FONT_HERSHEY_COMPLEX_SMALL, 
            1.0, 
            cv::Scalar(255,255,255), 
            1, 
            cv:: LINE_AA);
	 

	rectangle( frame, Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), Scalar(0, 0, 255), 3, LINE_8);
        Mat faceROI = frame_gray( faces[i] );
	}
	
    //imshow( "Capture - Face detection", frame );
    return (faces.size());
}
