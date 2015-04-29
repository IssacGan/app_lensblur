//
//  main.cpp
//  test_superpixels
//
//  Created by Ana Cambra on 26/11/14.
//  Copyright (c) 2014 Ana Cambra. All rights reserved.
//

#include <iostream>

#include "superpixels.h"
#include <time.h>

int main(int argc, const char * argv[]) {
    
    SuperPixels *SP;
    
    String path="/Users/acambra/TESIS/CVPR14/test_cvpr/venus/";//images_test/tsukuba/";
  //  String path="/Users/acambra/TESIS/CVPR14/test_cvpr/venus/";//images_test/tsukuba/";
    String name= "imL";//"imL";
    String nameImage = path + "superpixels/" + name + "_SLIC_70.png";
    String nameSP = path + "superpixels/" + name + "_SLIC_70.sp";
    String nameDepth = path + "gt/gt.png";//"resul/LLS-MRF_" + name + ".png";
    
    if (argc==3)
    {
        nameImage = argv[1];
        nameSP = argv[2];
        nameDepth = argv[3];
    }
     
    //cargar imagen original
    SP = new SuperPixels("/Users/acambra/TESIS/papers/CVPR14/examplesInteractiveapp/samples/foto1_peq.JPG");//"/Users/acambra/TESIS/CVPR14/test_cvpr/tsukuba/col3.png");//nameImage.c_str());
    Mat image = imread("/Users/acambra/TESIS/papers/CVPR14/examplesInteractiveapp/samples/foto1_peq.JPG");//"/Users/acambra/TESIS/CVPR14/test_cvpr/tsukuba/col3.png");//nameImage.c_str(),CV_LOAD_IMAGE_COLOR);
    
    imshow("image",image);
    SP->loadSuperPixels("/Users/acambra/Desktop/SLICSuperpixelsAndSupervoxelsCode/test_SLIC/Build/Products/Debug/yourfilename.dat");//nameSP.c_str());
    
    SP->loadDepthBuildSystem();
    
    SP->depthWithBorders();
    imshow("sobel",SP->getImage());
    waitKey(0);
    
  //  SP->loadDepth(nameDepth);
    
    
    //mostrar los superpixels...
    
    
    // TEST PIXELS FRONTERA
    
    //show superpixels
 /*   Mat im1,im2,mask,nonZeroCoordinates,out1,out2;
    cvtColor(SP->_lab,image,CV_Lab2BGR);
    //  image=sp->getImage();
    int id1=1, id2;
    id1=0;//SP->getIdFromPixel(pixel_x,pixel_y);
    id2=1;//SP->getIdFromPixel(last_pixel_x,last_pixel_y);
    
    //generar numeros aleatorios para coger pixels frontera
    printf("\tmeanDIFF\tstdDIFF\t\t\tL1\t\tEMD\n");
    int hora = time(NULL);
    srand(hora);
    int numRandom=0;
    
    for (int id1=0; id1<=SP->maxID-1;id1++)
    {
        Mat mask,mask1,maski;
        
        bitwise_and(SP->arraySP[id1].mask,(SP->_sobel == 0),mask);
       
        //frontera dilatada: mask1
        int dilate_size=10;
        Mat element = getStructuringElement( MORPH_RECT,
                                        Size( 2*dilate_size + 1, 2*dilate_size+1 ),
                                            Point( dilate_size, dilate_size ) );
        
        //mask 1 = frontera dilatada del id1
        dilate(mask, mask1, element);
        //dilatar 1 pixel, para saber si se "tocan"
        dilate(mask, mask, Mat(), Point(-1, -1), 2, 1, 1);
        
        Mat aout,n1,n2;

        cvtColor(mask1,aout,CV_GRAY2BGR);
        
        
        
        for(int i=id1+1; i<=SP->maxID; i++)
        {
            if (id1!=i)
            {
                bitwise_and(SP->arraySP[i].mask,(SP->_sobel == 0),maski);
                //comprobar si son vecinos originales mask
                Mat intersection;
                bitwise_and(mask, maski, intersection);
                
                if (countNonZero(intersection) > 0)
                {
                    //frontera del vecino i
                    //n2 = frontera de id = i
                    bitwise_and(mask1, SP->arraySP[i].mask, n2);
                    //printf("id1= %d es vecino de i %d\n",id1,i);
                    
                    //frontera con id1
                    dilate(maski, maski, element);
                    bitwise_and(maski, SP->arraySP[id1].mask, n1);
                    
                    /* aout.setTo(Scalar(0,255,0), n2);
                     aout.setTo(Scalar(255,0,0), n1);
                     imshow("aout1",aout);
                     waitKey(0);*/
                    
              /*      cvtColor(n1,n1,CV_GRAY2BGR);
                    cvtColor(n2,n2,CV_GRAY2BGR);
                    
                    if ( rand()%10 ==0) {
                        //coger frontera
                        
                        
                        //ver superpixels
                        Mat m1,m2;
                        /*cvtColor(SP->arraySP[i].mask,m1,CV_GRAY2BGR);
                         cvtColor(SP->arraySP[id1].mask,m2,CV_GRAY2BGR);*/
             /*           bitwise_or(SP->arraySP[i].mask, SP->arraySP[id1].mask, m1);
                        cvtColor(m1,m1,CV_GRAY2BGR);
                        bitwise_and(image, m1, m1
                                    );
                       // imshow("superpixels",m1);
                        //ver vecinos
                        Mat cmp1,cmp2;
                        bitwise_and(image, n1, cmp1);
                        bitwise_and(image, n2, cmp2);
                        bitwise_xor(cmp1, cmp2, cmp1);
                        //imshow("CMP",cmp1);
                        
                        //*/
                        
                        //sum
            /*            Mat im1,im2,im3,im4;
                        bitwise_and(image, n1, im1);
                        bitwise_and(image, n2, im2);
                        cvtColor(n1,n1,CV_BGR2GRAY);
                        cvtColor(n2,n2,CV_BGR2GRAY);
                        Scalar mean1,mean2,stddev1,stddev2;
                        // Scalar sum1 = sum(im1);
                        // Scalar sum2 = sum(im2);
                        meanStdDev(im1, mean1, stddev1,n1);
                        meanStdDev(im2, mean2, stddev2,n2);
                        float diff =
                        sqrt((mean1[0]-mean2[0])*(mean1[0]-mean2[0])
                             + (mean1[1]-mean2[1])*(mean1[1]-mean2[1])
                             + (mean1[2]-mean2[2])*(mean1[2]-mean2[2]));
                        
                        Mat sig1 = Mat(1,3,CV_32FC1);
                        sig1.at<float>(0,0)=mean1[0];
                        sig1.at<float>(0,1)=mean1[1];
                        sig1.at<float>(0,2)=mean1[2];
                        
                        Mat sig2 = Mat(1,3,CV_32FC1);
                        
                        sig2.at<float>(0,0)=mean2[0];
                        sig2.at<float>(0,1)=mean2[1];
                        sig2.at<float>(0,2)=mean2[2];
                        
                        float diffEMD=EMD(sig1,sig2,CV_DIST_C);
                        printf("%d RGB:\t%0.2f\t\t%0.2f\t\t%0.2f\t\t%0.2f\n",numRandom,
                               //(%0.2f) %0.2f (%0.2f) = %0.2f \n G: %0.2f (%0.2f) %0.2f (%0.2f) = %0.2f \n R: %0.2f (%0.2f) %0.2f (%0.2f) = %0.2f\n
                               (fabs( mean1[0]-mean2[0]) + fabs( mean1[1]-mean2[1])+ fabs( mean1[2]-mean2[2]))/3.0,
                               (fabs( stddev1[0]-stddev2[0]) + fabs( stddev1[1]-stddev2[1])+ fabs( stddev1[2]-stddev2[2]))/3.0,
                               diff, diffEMD);
                        /* mean1[0], stddev1[0], mean2[0], stddev2[0], fabs( mean1[0] - mean2[0]),
                         mean1[1], stddev1[1], mean2[1], stddev2[1], fabs( mean1[1] - mean2[1]),
                         mean1[2], stddev1[2], mean2[2], stddev2[2], fabs( mean1[2] - mean2[2]));*/
                        
                        
             /*           cvtColor(n1,n1,CV_GRAY2BGR);
                        cvtColor(n1,n1,CV_BGR2Lab);
                        cvtColor(n2,n2,CV_GRAY2BGR);
                        cvtColor(n2,n2,CV_BGR2Lab);
                        bitwise_and(SP->_lab, n1, im3);
                        bitwise_and(SP->_lab, n2, im4);
                        cvtColor(n1,n1,CV_Lab2BGR);
                        cvtColor(n1,n1,CV_BGR2GRAY);
                        cvtColor(n2,n2,CV_Lab2BGR);
                        cvtColor(n2,n2,CV_BGR2GRAY);
                        // Scalar mean1,mean2,stddev1,stddev2;
                        // Scalar sum1 = sum(im1);
                        // Scalar sum2 = sum(im2);
                        meanStdDev(im3, mean1, stddev1,n1);
                        meanStdDev(im4, mean2, stddev2,n2);
                        diff =
                        sqrt((mean1[0]-mean2[0])*(mean1[0]-mean2[0])
                             + (mean1[1]-mean2[1])*(mean1[1]-mean2[1])
                             + (mean1[2]-mean2[2])*(mean1[2]-mean2[2]));
                        sig1 = Mat(1,3,CV_32FC1);
                        sig1.at<float>(0,0)=mean1[0];
                        sig1.at<float>(0,1)=mean1[1];
                        sig1.at<float>(0,2)=mean1[2];
                        
                        sig2 = Mat(1,3,CV_32FC1);
                        sig2.at<float>(0,0)=mean2[0];
                        sig2.at<float>(0,1)=mean2[1];
                        sig2.at<float>(0,2)=mean2[2];
                        
                        diffEMD=EMD(sig1,sig2,CV_DIST_C);
                        printf("%d LAB:\t%0.2f\t\t%0.2f\t\t%0.2f\t\t%0.2f\n",numRandom,
                               (fabs( mean1[0]-mean2[0]) + fabs( mean1[1]-mean2[1])+ fabs( mean1[2]-mean2[2]))/3.0,
                               (fabs( stddev1[0]-stddev2[0]) + fabs( stddev1[1]-stddev2[1])+ fabs( stddev1[2]-stddev2[2]))/3.0,
                               diff, diffEMD);
                        /*
                         printf("CMP LAB = %f \n L: %0.2f (%0.2f) %0.2f (%0.2f) = %0.2f \n A: %0.2f (%0.2f) %0.2f (%0.2f) = %0.2f \n B: %0.2f (%0.2f) %0.2f (%0.2f) = %0.2f \n",
                         // (fabs( mean1[0]-mean2[0]) + fabs( mean1[1]-mean2[1])+ fabs( mean1[2]-mean2[2]))/3.0,
                         diff,
                         mean1[0], stddev1[0], mean2[0], stddev2[0], fabs( mean1[0] - mean2[0]),
                         mean1[1], stddev1[1], mean2[1], stddev2[1], fabs( mean1[1] - mean2[1]),
                         mean1[2], stddev1[2], mean2[2], stddev2[2], fabs( mean1[2] - mean2[2]));*/
                        
                        
                        //waitKey(0);
                 /*       char* nameOut;
                        std::sprintf(nameOut,"%sborders/%s_%d.png",path.c_str(),name.c_str(),numRandom);
                        imwrite(nameOut,cmp1);
                        numRandom++;
                    }
                    
                    
                }
            }
        }
    }
    
    waitKey(0);*/
    
  //  SP->addEquationsBinariesBoundaries(true);
    
    //
    //test BLUR
   /* cvtColor(SP->_lab,image,CV_Lab2BGR);
    Mat imageDepth = SP->_pixelDepth*255.0;
    int nbins=8;
    Mat f= SP->blurImage(image, imageDepth, nbins);
    
    imshow("Gaussian Blur focus 0",f);
    
    Mat f1= SP->blurImage(image, imageDepth, nbins,150,250,2.5);
    
    imshow("Gaussian Blur focus nbins",f1);
    waitKey(0);*/

    return 0;
}
//blur por mas depth
/*    double min, max,diff;
 minMaxLoc(imageDepth, &min, &max);
 cout<< " MIN:" << min << " MAX: " << max << endl;
 diff=max-min;
 
 
 Mat temp;
 Mat final = Mat::ones(image.rows, image.cols, image.type());
 
 /*GaussianBlur(image, temp, Size(9,9), 0.9);
 imshow("gaussian",temp);
 waitKey(0);
 
 blur(image, temp, Size(9,9));
 
 imshow("blur",temp);
 waitKey(0);*/

//
/*    int nbins=16;
 int depthMin = (int)max -  (diff/nbins);
 int depthMax = (int)max;
 int n = 0;
 Mat mask,m = Mat::ones(image.rows, image.cols, image.type());;
 while (n<nbins)//(depthMin >= min)
 {
 Mat mask1 = (imageDepth >= depthMin );
 Mat mask2 = (imageDepth <= depthMax);
 bitwise_and( mask1, mask2, mask);
 cvtColor(mask,mask,CV_GRAY2RGB);
 
 if (n!=0)
 {
 Mat imageBlur;
 GaussianBlur(image, imageBlur, Size( (n*2-1), (n*2-1)), 2.9);
 // blur(image, imageBlur, Size(n,n));
 // medianBlur(image, imageBlur, (n*2-1));//impar
 
 bitwise_and(imageBlur, mask, temp);
 }
 else
 {
 bitwise_and(image, mask, temp);
 }
 cout<< " depthmin:" << depthMin << " depthmax: " << depthMax << endl;
 n=n+1;
 depthMin = depthMin - (diff/nbins);
 depthMax = depthMax - (diff/nbins);
 
 
 
 
 final= final + temp;
 
 
 }
 
 imshow("final",final);*/


//blur por ID superpixels
/*  for(int i=0; i < SP->maxID+1; i++)
 {
 depth = SP->getDepth(i)*255;
 Mat mask=SP->getMask(i);
 cvtColor(mask,mask,CV_GRAY2RGB);
 cout<< " ID: " << i << " depth: " << depth << endl;
 
 
 if (depth < (min + (diff/nbins)))
 {
 // bitwise_and(median9, mask, temp);
 bitwise_and(blur9, mask, temp);
 
 }else if (depth < (min + 2*(diff/nbins)))
 {
 // bitwise_and(median3, mask, temp);
 bitwise_and(blur3, mask, temp);
 }
 else
 {
 bitwise_and(image, mask, temp);
 }
 
 //combinar
 
 final = final + temp;
 
 }*/
