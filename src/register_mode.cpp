#include "header.h"
#include "video_camera.h"
#include "detect_face.h"
#include "drawImage.h"
#include "create_new_thread.h"
#include "getCardInfo.h"
#include "faceAlign.h"
#include "facePoint.h"
using namespace std;
using namespace cv;


void register_mode()
{
	cout<<"您选择了注册模式！\n";
	cout<<"开始注册过程..\n";

	Main job;
	job.face_frame_num=0;
	job.mode_index=1;

	People people;//保存视频中出现的人物信息
	Card_Info info("D:/photo");

	//新建线程
	create_new_thread(&job);

	

	//调摄像头
	Camera cam;

	FacePoints fp;
	fp.load_model();

	while(cam.suc_flag)
	{//3
		
		//获取图像
		job.image_In=cam.read_from_camera();
		job.image_Show=cvCreateImage(cvGetSize(job.image_In),job.image_In->depth,job.image_In->nChannels);
		cvCopy(job.image_In,job.image_Show);

		job.image_gray = cvCreateImage(cvGetSize(job.image_In),8,1);
		cvCvtColor(job.image_In,job.image_gray,CV_RGB2GRAY);
		job.image_align=cvCreateImage(cvSize(60,60),job.image_In->depth,job.image_In->nChannels);
		
		//获取刷身份证的信息
		//info.detectAddFile();

		if (true)
		{
			//有人刷身份证
			people.name=info.get_card_name();
			people.cardNum=info.get_card_number();
			people.card_pic=info.get_card_pic();

			job.people=people;

			/* face_detect */
			FaceDetector fd(job.image_In);
			fd.runFaceDetector();

			if (fd.getDetectFaceCount()==1)
			{
				job.face_frame_num++;

				fd.getDetectFacePos(0,job.pos);

				/* 标出人脸位置 */
				draw_image(job.pos,job.image_Show);

				
				if(job.face_frame_num<10)
				{
					draw_image(job.pos,"Register Prepare...",job.image_Show,cvScalar(255,255,0));

				}

				else if(job.face_frame_num==10)
				{
					draw_image(job.pos,"Register ...",job.image_Show,cvScalar(255,0,255));
					
					/*< get face points */
					fp.facePoint_init(job.image_gray);
					fp.runFacePoints(1,job.pos);
					fp.getFacePoints(job.keyPoints);

					/*< face align */
					FaceAlign fa(job.image_In,job.pos,job.keyPoints);
					fa.runFaceAlign();
					cvCopy(fa.getAlignimg(),job.image_align);

					/*< 截取人脸图片 */
					CvRect rect = cvRect(job.pos[0], job.pos[1], job.pos[2] - job.pos[0], job.pos[3] - job.pos[1]);
					job.image_face_tmp = cvCreateImage(cvSize(job.pos[2] - job.pos[0], job.pos[3] - job.pos[1]),
						job.image_In->depth, job.image_In->nChannels);
					cvSetImageROI(job.image_In,rect);
					cvCopy(job.image_In,job.image_face_tmp);
					cvResetImageROI(job.image_In);

					
					

					Mat matmtx(job.image_align,true);
					job.image_Face=matmtx;

					job.g_event.SetEvent();
					
				}

				else if(job.face_frame_num>10&&job.register_OK==false)
				{
					draw_image(job.pos,"Register ...",job.image_Show,cvScalar(255,0,255));
				}
				else if(job.face_frame_num>10/*&&job.register_OK==true*/)
				{
					draw_image(job.pos,"Register OK ...",job.image_Show,cvScalar(0,0,255));
				}

	    }
			else
			{
				job.face_frame_num=0;
				info.flag=false;
				job.register_OK=false;
			}
		}

		cvShowImage("VIDEO",job.image_Show);
		cvWaitKey(2);

		cvReleaseImage(&job.image_Show);
		cvReleaseImage(&job.image_gray);
		cvReleaseImage(&job.image_align);
	}
}