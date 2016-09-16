#include "superpixels.h"

#include <stdio.h>

#include "depthoffielddefocus.h"

/**
 * Constructor
 * @param path Path completo de la imagen utilizada como base
 */
SuperPixels::SuperPixels(std::string path)
{
    try{
        _image = imread(path,CV_LOAD_IMAGE_COLOR);// * CV_LOAD_IMAGE_GRAYSCALE);
        
        if(_image.data != NULL){
        
          // *  cvtColor(_image,_image,CV_GRAY2RGB);
            fflush(stdout);
            
            //save CIE LAB
            _lab = imread(path,CV_LOAD_IMAGE_COLOR);
            cvtColor(_lab,_lab,CV_BGR2Lab);
        }
        else
        {
            printf("*** Image not found: %s\n",path.c_str());

        }
        
        printf("Mat _image CV_8UC1 rows %d cols %d\n",_image.rows,_image.cols);
    }
    catch(int e)
    {
        fprintf(stdout,"Image not found: Exception\n");
        fflush(stdout);
    }
}

/**
 * Devuelve una imagen en Qt que apunta a los datos de la imagen
 * @return La imagen en dato Qt que permite mostrarla en la interfaz
 */
/*QImage SuperPixels::getImage()
{
    //QImage point to the data of _image
    return QImage(_image.data, _image.cols, _image.rows, _image.step, QImage::Format_RGB888);
}*/

Mat SuperPixels::getImage()
{
    Mat m;
   // resize(_image, m, Size(640,480));
  //  imshow("IMAGe",m);
  //  waitKey(1);
    return _image;
}
Mat SuperPixels::imageDepth()
{
    Mat d;
   _depth.convertTo(d, CV_8UC1, 255, 0);
    return d;
}


/**
 * Rellena la matriz que contiene los identificadores de los distintos superpixels
 *TODO: leer en bloques el fochero para q sea mas eficiente
 * @param path Path completo del fichero donde estan los datos
 */
void SuperPixels::loadSuperPixels(std::string path)
{
   
    clock_t start = clock();
    
    calculateSLICSuperpixels(_image);
    
    printf("**** TIEMPO: Superpixels: %f seconds\n ",(float) (((double)(clock() - start)) / CLOCKS_PER_SEC) );
    
    //imshow("superpixels", _image);
    //
    return;//*/
    
    
    //.sp tiene w y h
    //.dat no tiene el tamaño
    //path="/Users/acambra/TESIS/images_test/test_sp/col3_30_0_1.sp";
    //Read file with superpixels
    //leer el fichero binario: w,h, id del superpixel de cada pixel
    FILE *f;
    try{
        f = fopen(path.c_str(),"r");//descomentar
       // f= fopen("/Users/acambra/TESIS/DepthSynthesis/appGUI_Xcode/Debug/IMG_1947.dat","r");//
        // f= fopen("/Users/anacambra/.dropbox-alt/Dropbox/IMG_1947.dat","r");//descomentar
        //printf("abierto!!!!!!");
        
        int maxID=0;
        int w=0,h=0;
        if( f != NULL)
        {
            if (path.find(".sp") != -1)
            {
                fread(&w,sizeof(int),1,f);
                fread(&h,sizeof(int),1,f);
            }
            else
            {
               h=_image.rows; w=_image.cols;
            }
           
            _ids= Mat::zeros(h,w,CV_32FC1);
            //TODO: leer el fichero con un buffer, y leer fila a fila
            for(int i=0;i<h;i++)
            {
                for (int j=0; j<w; j++)
                {
                    if(!feof(f))
                    {
                        int id;
                        fread(&id,sizeof(int),1,f);
                        if (id >= maxID) maxID=id;
                        _ids.at<float>(i,j)=(float)id;
                    }
                }
            }
        }
        fclose(f);
        
        if(_ids.data == NULL)
        {
            printf("*** Superpixels not found: %s\n",path.c_str());
        }
        
        printf("Mat _ids (%d,%d,CV_32FC1) rows %d cols %d Max ID Superpixels %d\n",h,w,_ids.rows,_ids.cols,maxID);
        fflush(stdout);
        
        //una vez leido el fichero, creo el vector de superpixels
        this->maxID=maxID;
        arraySP = new SuperPixel[maxID+1];
    }
    catch(int e)
    {
        fprintf(stderr,"*EXCEPTION: Error load file superpixels");
        fflush(stderr);
        
        //calcular
        calculateSLICSuperpixels(_image);
    }
     //infoSuperpixels2file(path+"_info.txt");
    
}

void SuperPixels::calculateSLICSuperpixels(Mat mat)
{
    // Read the Lenna image. The matrix 'mat' will have 3 8 bit channels
    // corresponding to BGR color space.
   // cv::Mat mat = _image;//cv::imread("/Users/acambra/TESIS/CVPR14/test_cvpr/tsukuba/col3.png", CV_LOAD_IMAGE_COLOR);
    
    // Convert image to one-dimensional array.
    float* image = new float[mat.rows*mat.cols*mat.channels()];
    for (int i = 0; i < mat.rows; ++i) {
        for (int j = 0; j < mat.cols; ++j) {
            // Assuming three channels ...
            image[j + mat.cols*i + mat.cols*mat.rows*0] = mat.at<cv::Vec3b>(i, j)[0];
            image[j + mat.cols*i + mat.cols*mat.rows*1] = mat.at<cv::Vec3b>(i, j)[1];
            image[j + mat.cols*i + mat.cols*mat.rows*2] = mat.at<cv::Vec3b>(i, j)[2];
        }
    }
    
    // The algorithm will store the final segmentation in a one-dimensional array.
    vl_uint32* segmentation = new vl_uint32[mat.rows*mat.cols];
    vl_size height = mat.rows;
    vl_size width = mat.cols;
    vl_size channels = mat.channels();
    
    // The region size defines the number of superpixels obtained.
    // Regularization describes a trade-off between the color term and the
    // spatial term.
    
    
    //200 max numero incognitas TODOoooo
    int numSup = mat.rows*mat.cols /(_TAM_SP*_TAM_SP);
    vl_size region;
    if (numSup > _NUM_MAX_SP)
    {
        //cambiar tamño del superpixel
        
         region = sqrt(float(mat.rows*mat.cols)/_NUM_MAX_SP);
    }
    else
         region = _TAM_SP;
    
    
    printf("numpixesl %d num super %d ", mat.rows*mat.cols,numSup);
    //vl_size region = 30//mat.rows*mat.cols/ 200; ///30;
    float regularization = 10000;
    vl_size minRegion = _TAM_SP - 5;
    
    vl_slic_segment(segmentation, image, width, height, channels, region, regularization, minRegion);
    
    // Convert segmentation.
    int** labels = new int*[mat.rows];
    for (int i = 0; i < mat.rows; ++i) {
        labels[i] = new int[mat.cols];
        
        for (int j = 0; j < mat.cols; ++j) {
            labels[i][j] = (int) segmentation[j + mat.cols*i];
            
        }
    }
    
    // Compute a contour image: this actually colors every border pixel
    // red such that we get relatively thick contours.
    int label = 0;
    int labelTop = -1;
    int labelBottom = -1;
    int labelLeft = -1;
    int labelRight = -1;
    
    //
    int maxID=0;
    _ids= Mat::zeros(mat.rows,mat.cols,CV_32FC1);
    
    for (int i = 0; i < mat.rows; i++) {
        for (int j = 0; j < mat.cols; j++) {
            
            label = labels[i][j];
            
            //guardar los ids
            if (label >= maxID) maxID=label;
            _ids.at<float>(i,j)=(float)label;
            
            
            labelTop = label;
            if (i > 0) {
                labelTop = labels[i - 1][j];
            }
            
            labelBottom = label;
            if (i < mat.rows - 1) {
                labelBottom = labels[i + 1][j];
            }
            
            labelLeft = label;
            if (j > 0) {
                labelLeft = labels[i][j - 1];
            }
            
            labelRight = label;
            if (j < mat.cols - 1) {
                labelRight = labels[i][j + 1];
            }
            
            if (label != labelTop || label != labelBottom || label!= labelLeft || label != labelRight) {
                mat.at<cv::Vec3b>(i, j)[0] = 0;
                mat.at<cv::Vec3b>(i, j)[1] = 0;
                mat.at<cv::Vec3b>(i, j)[2] = 255;
            }
        }
    }
    //una vez leido el fichero, creo el vector de superpixels
    this->maxID=maxID;
    arraySP = new SuperPixel[maxID+1];
    
     printf("Max ID %d ",  this->maxID);
    // Save the contour image.
   //imshow("Lenna_contours.png", mat);
   // waitKey(5);
    
}

/**
 * Rellena la matriz que contiene los valores de profundidad a partir de un fichero
 * Modifica los valores de la profundidad de cada superpixel como la mediana de sus valores originales
 * PRE: loadSuperPixels()
 * @param path Path completo del fichero donde estan los datos
 */
void SuperPixels::loadDepth(std::string path)
{
   
   // path="/Users/acambra/Dropbox/pointCloud/visualSFM/depth/00000000.png";
    //read image with depth. _image is not null
    try{
        //CV_8U == depth 0;
        //channels 1
        
        Mat depth=imread(path,CV_LOAD_IMAGE_GRAYSCALE);//*///descomentar
        
        //imshow("loadDepth",depth);
        //Mat depth =_ids.clone();//descomentar
        
        //normalize depth [0,1]
        depth.convertTo(_depth,CV_32FC1);
        normalize(_depth, _depth, 0.0, 1.0, NORM_MINMAX, -1);
        
        _pixelDepth=_depth.clone();

        //_depth esta la profundidad del fichero
        //calcular la mediana de las profundidades de cada superpixel
        if (_ids.data != NULL && _depth.data != NULL)
        {
            //id maximo del superpixels, en this->maxID el maximo id leido del fichero
            for(int i=0; i<=maxID; i++ )
            {
                //crear mascara solo con los pixels de superpixel i
                Mat mask_im;
                mask_im = (_ids == (float)i);//compare(_ids, group, mask_im, CV_CMP_EQ);
                //num de pixels del superpixel
                int total = countNonZero(mask_im);

                //iniciar el superpixel en el array de superpixels
                arraySP[i].init(i,mask_im,total);
               
                arraySP[i].meanVarDepth(_depth);
                
                float depth_m;
               
                depth_m = arraySP[i].d_mean;//arraySP[i].medianDepth(_depth);//,d_hist);//, accurancy);
                arraySP[i].depth=arraySP[i].d_mean;
                
                
                arraySP[i].descriptorsLab(_lab);
                
                //modificar la imagen depth de todo el superpixels
                _depth.setTo(depth_m,mask_im);//double)depth_m.val[0],mask_im);
             
                //coger la imagen y colorear cada superpixel con el color de su depth
                Mat im;
                im.setTo(255, mask_im);
                int d=depth_m*255;//(int)(depth_m.val[0]*255);
             
                 Scalar color( d,d,d);
                _image.setTo(color, mask_im);

            }
            //calcular bordes de los superpixelsmedian(_depth,notZero);
            //SOBEL
            int scale = 1;
            int delta = 0;
            int ddepth =-1;// CV_16S;
            /// Generate grad_x and grad_y
            Mat grad_x, grad_y;
            Mat abs_grad_x, abs_grad_y,grad;

            /// Gradient X
            //Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
            Sobel( _ids, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
            convertScaleAbs( grad_x, abs_grad_x );

            /// Gradient Y
            //Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
            Sobel( _ids, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
            convertScaleAbs( grad_y, abs_grad_y );

            /// Total Gradient (approximate)
            addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );
           // grad = (grad != 0);

            _sobel= (grad == 0);

            Scalar color;
            color.val[0]=0;

            _image.setTo(color, grad);//_sobel);
            cvtColor(_image,_imDepth,CV_RGB2GRAY);

            depthWithBorders();

        }
        else{
            printf("Primero cargar superpixels");
            fflush(stdout);
            return;
        }

        printf("Mat _depth CV_32FC1 rows %d cols %d\n",_depth.rows,_depth.cols);
        fflush(stdout);
    }
    catch(int e)
    {
        fprintf(stdout,"Error load file deth");
        fflush(stdout);
    }
    
}
void SuperPixels::loadUserDepth(std::string path)
{
    _depth= Mat::ones(_ids.rows,_ids.cols,CV_32FC1)*-1;
    
   // Mat depth=imread(path,CV_LOAD_IMAGE_GRAYSCALE);//[0..255]
    for(int i=0; i<=maxID; i++ )
    {
       
        printf("id: %d depth: %f \n" ,i,arraySP[i].depth);
        
    }
    
    
}

void SuperPixels::loadDepthBuildSystem()
{

    try{
        //CV_8U == depth 0;
        //channels 1
        
        _depth= Mat::ones(_ids.rows,_ids.cols,CV_32FC1)*-1;
        
        //_imDepth=_depth.clone()*255;
        
        //_depth esta la profundidad del fichero
        if (_ids.data != NULL && _depth.data != NULL)
        {
            //CREAR SISTEMAS DE ECUACIONES
            int numIncognitas = maxID+1;
            //Optimization::LeastSquaresLinearSystem<float>(numIncognitas);
            this->ecuaciones= Optimization::LeastSquaresLinearSystem<double>(numIncognitas);
            this->unary = Optimization::LeastSquaresLinearSystem<double>(numIncognitas);
            this->binary = Optimization::LeastSquaresLinearSystem<double>(numIncognitas);
            this->binaryCOLOR = Optimization::LeastSquaresLinearSystem<double>(numIncognitas);
            
            //id maximo del superpixels, en this->maxID el maximo id leido del fichero
            for(int i=0; i<=maxID; i++ )
            {
                //crear mascara solo con los pixels de superpixel i
                Mat mask_im;
                mask_im = (_ids == (float)i);
                int total = countNonZero(mask_im);
                //iniciar el superpixel en el array de superpixels
                arraySP[i].init(i,mask_im,total);
                arraySP[i].descriptorsLab(_lab);
                arraySP[i].depth=-1;
                //arraySP[i].meanVarDepth(_depth);
                
                //float depth_m;
                
               // depth_m = arraySP[i].medianDepth(_depth);//,d_hist);//, accurancy);
                
               // arraySP[i].descriptorsLab(_lab);
                
                //modificar la imagen depth de todo el superpixels
               // _depth.setTo(depth_m,mask_im);//double)depth_m.val[0],mask_im);
                
                //coger la imagen y colorear cada superpixel con el color de su depth
               /* Mat im;
                im.setTo(255, mask_im);
                int d=depth_m*255;//(int)(depth_m.val[0]*255);
                
                Scalar color( d,d,d);
                _image.setTo(color, mask_im);*/
                
            }
            //calcular bordes de los superpixelsmedian(_depth,notZero);
            //SOBEL
            int scale = 1;
            int delta = 0;
            int ddepth =-1;// CV_16S;
            /// Generate grad_x and grad_y
            Mat grad_x, grad_y;
            Mat abs_grad_x, abs_grad_y,grad;
            
            /// Gradient X
            //Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
            Sobel( _ids, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
            convertScaleAbs( grad_x, abs_grad_x );
            
            /// Gradient Y
            //Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
            Sobel( _ids, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
            convertScaleAbs( grad_y, abs_grad_y );
            
            /// Total Gradient (approximate)
            addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );
            // grad = (grad != 0);
            
            _sobel= (grad == 0);
            
            Scalar color;
            color.val[0]=0;
            
            _image.setTo(color, grad);//_sobel);
            cvtColor(_image,_imDepth,CV_RGB2GRAY);
            
            depthWithBorders();
            
            //addEquationsBinaries();
            
            clock_t start = clock();
            
            addEquationsBinariesBoundaries(false);
            
            printf("**** TIEMPO: Binarias: %f seconds\n ",(float) (((double)(clock() - start)) / CLOCKS_PER_SEC) );
            
            
        }
        else{
            printf("Primero cargar superpixels");
            fflush(stdout);
            return;
        }
        
        printf("Mat _depth CV_32FC1 rows %d cols %d\n",_depth.rows,_depth.cols);
        fflush(stdout);
    }
    catch(int e)
    {
        fprintf(stdout,"Error load file deth");
        fflush(stdout);
    }
    
    
    //
    
}

//add binary equations: compare color boundaries
void SuperPixels::addEquationsBinariesBoundaries(bool verbose)
{
    //bool save samples
   // bool saveIMAGES=true;
    clock_t start = clock();
    
    //show debug windows
    //bool verbose = false;
    //resetear las ecuaciones de color
    this->binaryCOLOR = Optimization::LeastSquaresLinearSystem<double>(maxID+1);
    this->binary = Optimization::LeastSquaresLinearSystem<double>(maxID+1);
    
    int nB=0;
    //copiar lo del test
    //show superpixels
    Mat image;
    Mat im1,im2,mask,nonZeroCoordinates,out1,out2;
    cvtColor(_lab,image,CV_Lab2BGR);
    //  image=sp->getImage();
   // int id1=1, id2;
  //  id1=0;//SP->getIdFromPixel(pixel_x,pixel_y);
  //  id2=1;//SP->getIdFromPixel(last_pixel_x,last_pixel_y);
    
    //generar numeros aleatorios para coger pixels frontera
   // printf("\tmeanDIFF\tstdDIFF\t\t\tL1\t\tEMD_L1\t\tEMD_L2\t\tEMD_C\n");

    
    
    for (int id1=0; id1<=maxID-1;id1++)
    {
        Mat mask,mask1,maski;
        
        bitwise_and(arraySP[id1].mask,(_sobel == 0),mask);
        
        //frontera dilatada: mask1
        int dilate_size=5;
        Mat element = getStructuringElement( MORPH_RECT,
                                            Size( 2*dilate_size + 1, 2*dilate_size+1 ),
                                            Point( dilate_size, dilate_size ) );
        
        //mask 1 = frontera dilatada del id1
        dilate(mask, mask1, element);
        //dilatar 1 pixel, para saber si se "tocan"
        dilate(mask, mask, Mat(), Point(-1, -1), 2, 1, 1);
        
        Mat aout,n1,n2;
        
        cvtColor(mask1,aout,CV_GRAY2BGR);
        
        for(int i=id1+1; i<=maxID; i++)
        {
            if (id1!=i)
            {
                bitwise_and(arraySP[i].mask,(_sobel == 0),maski);
                //comprobar si son vecinos originales mask
                Mat intersection;
                bitwise_and(mask, maski, intersection);
                int total=0;
                
                if (countNonZero(intersection) > 0)
                {
                    //frontera del vecino i
                    //n2 = frontera de id = i
                    bitwise_and(mask1, arraySP[i].mask, n2);
                    total = countNonZero(n2);
                    //printf("id1= %d es vecino de i %d\n",id1,i);
                    
                    //frontera con id1
                    dilate(maski, maski, element);
                    bitwise_and(maski, arraySP[id1].mask, n1);
                    total = total + countNonZero(n1);

                    cvtColor(n1,n1,CV_GRAY2BGR);
                    cvtColor(n2,n2,CV_GRAY2BGR);
                    
                    //ver superpixels
                    Mat m1,m2;
                    /*cvtColor(SP->arraySP[i].mask,m1,CV_GRAY2BGR);
                     cvtColor(SP->arraySP[id1].mask,m2,CV_GRAY2BGR);*/
                    bitwise_or(arraySP[i].mask, arraySP[id1].mask, m1);
                    cvtColor(m1,m1,CV_GRAY2BGR);
                    bitwise_and(image, m1, m1
                                );
                    // imshow("superpixels",m1);
                    //ver vecinos
                    Mat cmp1,cmp2;
                    bitwise_and(image, n1, cmp1);
                    bitwise_and(image, n2, cmp2);
                    bitwise_xor(cmp1, cmp2, cmp1);
                    
                    if (verbose) {
                        //pintar la frontera
                        cmp1.setTo(Scalar(255,0,0),intersection);
                        imshow("Pixels frontera = " + to_string(dilate_size),cmp1);
                        
                    }
                    //imshow("CMP",cmp1);
                    
                    //*/
                    
                    //sum
                    Mat im1,im2,im3,im4;
                    Scalar mean1,mean2,stddev1,stddev2;
                  //  bitwise_and(image, n1, im1);
                  //  bitwise_and(image, n2, im2);
                  //  cvtColor(n1,n1,CV_BGR2GRAY);
                  //  cvtColor(n2,n2,CV_BGR2GRAY);
                    
                    // Scalar sum1 = sum(im1);
                    // Scalar sum2 = sum(im2);
                  /*  meanStdDev(im1, mean1, stddev1,n1);
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
                    
                    float diffEMD=EMD(sig1,sig2,CV_DIST_C);*/
                  /*  printf("%d RGB:\t%0.2f\t\t%0.2f\t\t%0.2f\t\t%0.2f\t\t%0.2f\t\t%0.2f\n",numRandom,
                           //(%0.2f) %0.2f (%0.2f) = %0.2f \n G: %0.2f (%0.2f) %0.2f (%0.2f) = %0.2f \n R: %0.2f (%0.2f) %0.2f (%0.2f) = %0.2f\n
                           (fabs( mean1[0]-mean2[0]) + fabs( mean1[1]-mean2[1])+ fabs( mean1[2]-mean2[2]))/3.0,
                           (fabs( stddev1[0]-stddev2[0]) + fabs( stddev1[1]-stddev2[1])+ fabs( stddev1[2]-stddev2[2]))/3.0,
                           diff, EMD(sig1,sig2,CV_DIST_L1),EMD(sig1,sig2,CV_DIST_L2),EMD(sig1,sig2,CV_DIST_C));//*/
                    /* mean1[0], stddev1[0], mean2[0], stddev2[0], fabs( mean1[0] - mean2[0]),
                     mean1[1], stddev1[1], mean2[1], stddev2[1], fabs( mean1[1] - mean2[1]),
                     mean1[2], stddev1[2], mean2[2], stddev2[2], fabs( mean1[2] - mean2[2]));*/
                    
                    
                    
                    /////
                    /////
                    
                    
                  //  cvtColor(n1,n1,CV_GRAY2BGR);
                  // cvtColor(n1,n1,CV_BGR2Lab);
                 //   cvtColor(n2,n2,CV_GRAY2BGR);
                   // cvtColor(n2,n2,CV_BGR2Lab);
                   // bitwise_and(_lab, n1, im3);
                   // bitwise_and(_lab, n2, im4);
                   // cvtColor(n1,n1,CV_Lab2BGR);
                    cvtColor(n1,n1,CV_BGR2GRAY);
                   // cvtColor(n2,n2,CV_Lab2BGR);
                    cvtColor(n2,n2,CV_BGR2GRAY);
                    
                    
                    
                  //  imshow("im3",im3);
                  //  imshow("n1",n1);
                    // Scalar mean1,mean2,stddev1,stddev2;
                    // Scalar sum1 = sum(im1);
                    float mimean1a=0; int num=0;
                    float mimean1L=0;
                    float mimean1b=0;
                    
                    float mimean2a=0; int num2=0;
                    float mimean2L=0;
                    float mimean2b=0;
                    
                /*    for(int i=0; i< _lab.rows; i++)
                        for(int j=0; j<_lab.cols; j++)
                        {
                            if (n1.at<uchar>(i,j) != 0) {
                                // printf("L: %f\n",(double)_lab.at<Vec3f>(i,j)[0]);
                                // printf("a lab: %f\n",(float)_lab.at<Vec3f>(i,j)[1]);
                                // printf("a im3: %f\n",(float)im3.at<Vec3f>(i,j)[1]);
                              /*  printf("L lab: %d\n",(int)_lab.at<Vec3b>(i,j)[0]);
                                 printf("a lab: %d\n",(int)_lab.at<Vec3b>(i,j)[1]);
                                 printf("b lab: %d\n",(int)_lab.at<Vec3b>(i,j)[2]);*/
                 /*               mimean1L=(int)_lab.at<Vec3b>(i,j)[0] + mimean1L;
                                mimean1a=(int)_lab.at<Vec3b>(i,j)[1] + mimean1a;
                                mimean1b=(int)_lab.at<Vec3b>(i,j)[2] + mimean1b;
                                
                                num=num+1;
                                
                            }
                        }
                    
                    for(int i=0; i< _lab.rows; i++)
                        for(int j=0; j<_lab.cols; j++)
                        {
                            if (n2.at<uchar>(i,j) != 0) {
                                // printf("L: %f\n",(double)_lab.at<Vec3f>(i,j)[0]);
                                // printf("a lab: %f\n",(float)_lab.at<Vec3f>(i,j)[1]);
                                // printf("a im3: %f\n",(float)im3.at<Vec3f>(i,j)[1]);
                                /*  printf("L lab: %d\n",(int)_lab.at<Vec3b>(i,j)[0]);
                                 printf("a lab: %d\n",(int)_lab.at<Vec3b>(i,j)[1]);
                                 printf("b lab: %d\n",(int)_lab.at<Vec3b>(i,j)[2]);*/
                  /*              mimean2L=(int)_lab.at<Vec3b>(i,j)[0] + mimean2L;
                                mimean2a=(int)_lab.at<Vec3b>(i,j)[1] + mimean2a;
                                mimean2b=(int)_lab.at<Vec3b>(i,j)[2] + mimean2b;
                                
                                num2=num2+1;
                                
                            }
                        }*/
                    
                    
                    // Scalar sum2 = sum(im2);
                    meanStdDev(_lab, mean1, stddev1,n1);
                    meanStdDev(_lab, mean2, stddev2,n2);
                    
                    
                    float diff =
                    sqrt((mean1[0]-mean2[0])*(mean1[0]-mean2[0])
                         + (mean1[1]-mean2[1])*(mean1[1]-mean2[1])
                         + (mean1[2]-mean2[2])*(mean1[2]-mean2[2]));
                    
                    
                  /*  Mat sig1 = Mat(1,3,CV_32FC1);
                    sig1.at<float>(0,0)=mean1[0];
                    sig1.at<float>(0,1)=mean1[1];
                    sig1.at<float>(0,2)=mean1[2];
                    
                    Mat sig2 = Mat(1,3,CV_32FC1);
                    sig2.at<float>(0,0)=mean2[0];
                    sig2.at<float>(0,1)=mean2[1];
                    sig2.at<float>(0,2)=mean2[2];
                    
                    //float*/
                   //diffEMD=EMD(sig1,sig2,CV_DIST_C);
                  /*  printf("L:\t%0.2f\t\t%0.2f\n a:\t%0.2f\t\t%0.2f\n b:\t%0.2f\t\t%0.2f\n\n",
                           mean1[0], mean2[0],
                           mean1[1], mean2[1],
                           mean1[2], mean2[2]);
                    
                    
                    
                    printf("mi mean L:%f", (float)mimean1L/(float)num);
                     printf("mi mean a:%f", (float)mimean1a/(float)num);
                     printf("mi mean b:%f", (float)mimean1b/(float)num);/*
                    
                    printf("mi mean2 L:%f", (float)mimean2L/(float)num2);
                    printf("mi mean2 a:%f", (float)mimean2a/(float)num2);
                    printf("mi mean2 b:%f", (float)mimean2b/(float)num2);*/
                    
                 /*   mimean1L = (float)mimean1L/(float)num;
                    mimean1a = (float)mimean1a/(float)num;
                    mimean1b = (float)mimean1b/(float)num;
                    
                    mimean2L = (float)mimean2L/(float)num2;
                    mimean2a = (float)mimean2a/(float)num2;
                    mimean2b = (float)mimean2b/(float)num2;
                    
                    float midiff =
                    sqrt((float)((mimean1L-mimean2L)*(mimean1L-mimean2L))
                         + (float)((mimean1a-mimean2a)*(mimean1a-mimean2a))
                         + (float)((mimean1b-mimean2b)*(mimean1b-mimean2b)));
                    
                    printf("\n\n diff %f midiff :%f", diff, midiff);
                  
                    getchar();*/
                    
                    //umbral para añadir la equacion binaria
                    
                   // for (unsigned int n = 0; n < total; n++)
                   // {
                        Optimization::SparseEquation<float> eq;
                        eq.a[id1] = 1.0;
                        eq.a[i] = -1.0;
                        eq.b=0;
                        
                        if (diff <= 15.0)//25)//50.0)
                        {
                            binaryCOLOR.add_equation(eq);
                        }else
                            binary.add_equation(eq);
                        
                        nB++;
                        
                   // }
                    /*
                     printf("CMP LAB = %f \n L: %0.2f (%0.2f) %0.2f (%0.2f) = %0.2f \n A: %0.2f (%0.2f) %0.2f (%0.2f) = %0.2f \n B: %0.2f (%0.2f) %0.2f (%0.2f) = %0.2f \n",
                     // (fabs( mean1[0]-mean2[0]) + fabs( mean1[1]-mean2[1])+ fabs( mean1[2]-mean2[2]))/3.0,
                     diff,
                     mean1[0], stddev1[0], mean2[0], stddev2[0], fabs( mean1[0] - mean2[0]),
                     mean1[1], stddev1[1], mean2[1], stddev2[1], fabs( mean1[1] - mean2[1]),
                     mean1[2], stddev1[2], mean2[2], stddev2[2], fabs( mean1[2] - mean2[2]));*/
                    
                    
                    //waitKey(0);
                    /*
                    if (saveIMAGES) {
                        char* nameOut;
                        std::sprintf(nameOut,"%sborders/%s_%d.png",path.c_str(),name.c_str(),numRandom);
                        imwrite(nameOut,cmp1);
                    }
                    //*/
                    
                    if (verbose) {
                        waitKey(1);
                    }
                    
                }
            }
        }
    }
    
    printf("\t * NUM BINARIAS Ecuaciones binarias %d \n ",nB);
    
    
  //  binary.normalize();
  //  binaryCOLOR.normalize();
        //waitKey(0);
        
}

float SuperPixels::similarityBinariesBoundaries(int s1, int s2)
{
    
    
    Mat image;
    Mat im1,im2,nonZeroCoordinates,out1,out2;
    
    //for (int id1=0; id1<=maxID-1;id1++)
    //{
    int id1 = s1;
    Mat mask,mask1,maski;
    
    bitwise_and(arraySP[id1].mask,(_sobel == 0),mask);
    
    //frontera dilatada: mask1
    int dilate_size=5;
    Mat element = getStructuringElement( MORPH_RECT,
                                        Size( 2*dilate_size + 1, 2*dilate_size+1 ),
                                        Point( dilate_size, dilate_size ) );
    
    //mask 1 = frontera dilatada del id1
    dilate(mask, mask1, element);
    //dilatar 1 pixel, para saber si se "tocan"
    dilate(mask, mask, Mat(), Point(-1, -1), 2, 1, 1);
    
    Mat aout,n1,n2;
    
    cvtColor(mask1,aout,CV_GRAY2BGR);
    
    //for(int i=id1+1; i<=maxID; i++)
    //{
    int i = s2;
    
    if (id1!=i)
    {
        bitwise_and(arraySP[i].mask,(_sobel == 0),maski);
        //comprobar si son vecinos originales mask
        Mat intersection;
        bitwise_and(mask, maski, intersection);
        int total=0;
        
        if (countNonZero(intersection) > 0)
        {
            //frontera del vecino i
            //n2 = frontera de id = i
            bitwise_and(mask1, arraySP[i].mask, n2);
            total = countNonZero(n2);
            
            //frontera con id1
            dilate(maski, maski, element);
            bitwise_and(maski, arraySP[id1].mask, n1);
            total = total + countNonZero(n1);
            
            cvtColor(n1,n1,CV_GRAY2BGR);
            cvtColor(n2,n2,CV_GRAY2BGR);
            
            //ver superpixels
            Mat m1,m2;
            bitwise_or(arraySP[i].mask, arraySP[id1].mask, m1);
            cvtColor(m1,m1,CV_GRAY2BGR);
            bitwise_and(image, m1, m1
                        );
            //ver vecinos
            Mat cmp1,cmp2;
            bitwise_and(image, n1, cmp1);
            bitwise_and(image, n2, cmp2);
            bitwise_xor(cmp1, cmp2, cmp1);
            
            //sum
            Mat im1,im2,im3,im4;
            Scalar mean1,mean2,stddev1,stddev2;
            
            cvtColor(n1,n1,CV_BGR2GRAY);
            cvtColor(n2,n2,CV_BGR2GRAY);
            meanStdDev(_lab, mean1, stddev1,n1);
            meanStdDev(_lab, mean2, stddev2,n2);
            
            float diff =
            sqrt((mean1[0]-mean2[0])*(mean1[0]-mean2[0])
                 + (mean1[1]-mean2[1])*(mean1[1]-mean2[1])
                 + (mean1[2]-mean2[2])*(mean1[2]-mean2[2]));

            if (diff <= 15.0)//25)//50.0)
            {
                return 0.99;
            }else
                return (1.0 - 0.99);
            
        }//if intersecction
    }//id1!=i
    return 256.0*256.0;
}



    //add binary equations compare histogram

    void SuperPixels::addEquationsBinaries()
    {
        this->binaryCOLOR = Optimization::LeastSquaresLinearSystem<double>(maxID+1);
        
        int numB=0;
        float up,down;
        up=0;
        down=10000;
        clock_t start = clock();
        for (unsigned int id=0; id < maxID +1; id++)
        {
            int count[maxID+1-id];
            calcularVecinos2(id,count);
            for (unsigned int v=0; v < maxID+1-id;v++)
            {
                float cmp = cmpLab(id, id+v,CV_COMP_BHATTACHARYYA);///(_depth.rows*_depth.cols);// CV_COMP_INTERSECT);//CV_COMP_BHATTACHARYYA);//
                // printf("Color: %f \n ",cmp);
                if (cmp > up) up=cmp;
                if (cmp < down) down=cmp;
                
                //0 superpixels lab iguales
                //1 nada en comun
                //if (cmp <= (0.001*19860.216797)) //CV_COMP_CHISQR
                //tsukuba 1355
                //venus 1831
                //teddy 1729
                // if (cmp > (0.9))//CV_COMP_INTERSECT
                if (cmp <= 0.55)//CV_COMP_BHATTACHARYYA
                {
                    for (unsigned int n=0; n< count[v]; n++)
                    {
                        Optimization::SparseEquation<float> eq;
                        eq.a[id] = 1.0;//(1 - w_u);
                        eq.a[id+v] = - 1.0; // - (1 - w_u);
                        eq.b=0;
                        binaryCOLOR.add_equation(eq);
                        numB++;
                    }
                }else{
                    
                    
                    //printf("Color: %f  ",cmp);
                    // arraySP[id].showHistogram(arraySP[id].hist_l);
                    // arraySP[id+v].showHistogram(arraySP[id+v].hist_l);
                    // waitKey(5);
                    // getchar();
                    //0 superpixels lab iguales
                    //1 nada en comun
                    // if (cmp != 1)
                    // {
                    for (unsigned int n=0; n< count[v]; n++)
                    {
                        Optimization::SparseEquation<float> eq;
                        eq.a[id] = 1.0;//(1 - w_u);
                        eq.a[id+v] = - 1.0; // - (1 - w_u);
                        eq.b=0;
                        binary.add_equation(eq);
                        numB++;
                    }
                }
                // }
                // else{
                //     cout << "NADA en comun!!!!!" << endl;
                // }
            }//*/
        }
        
       
       // printf("CMP Color: MAX %f MIN %f\n ",up,down);
       // printf("TIEMPO %d ecuaciones binarias: %f seconds\n ",numB,((double)(clock() - start) /CLOCKS_PER_SEC));
    }
    void SuperPixels::addEquationsUnaries(int x,int y,float di)
    {
        if (x < 0 || x > _image.cols || y < 0 || y >_image.rows)
            return;
        
        clock_t start = clock();
        
        _depth.at<float>(y,x) = di;
        
        int id=getIdFromPixel(x, y);
        arraySP[id].depth=di;
        
        Mat image=_depth*255.0;
        image.convertTo(image, CV_8UC1);
        
        cvtColor(image, _image,CV_GRAY2RGB);
        //imshow("unaries",_image);
        imwrite("/Users/acambra/Desktop/userInput_p.png", image);
        
        //ecuaciones unarias.. añadir
        
            if(di  > 0.0)
            {
                //indices de pixels
                
               // Mat nonZeroCoordinates;
                //findNonZero(getMask(id), nonZeroCoordinates);
                
              //  for (int n = 0; n < (int)countNonZero(mask); n++ )
              //  {
                   // int i=nonZeroCoordinates.at<Point>(n).x;
                   // int j=nonZeroCoordinates.at<Point>(n).y;
                    
                   // float di = getDepthFromPixel(i, j);
                    //depth de cada pixel
                    //  float di= getDepthInitialFromPixel(i, j);
                    /*  if (di > 255.0 || di < 0)
                     cout << "MALLLLLL" << endl;*/
                    //añadir ecuacion coste unario
                    // if (di != 0.0)
                    //  {
                    //COSTE UNARIO
                    Optimization::SparseEquation<float> eq;
                    eq.a[id]=1.0;//w_u;//*id;
                    eq.b=di;
                    unary.add_equation(eq);
                    //numU++;
                    //  }
                    
               // }
                
            }
        
        //añadir solo una ecuacion por pixel
        
       /* Optimization::SparseEquation<float> eq;
        eq.a[getIdFromPixel(x, y)]=1.0;//w_u;//*id;
        eq.b=di;
        unary.add_equation(eq);*/
        
      /*  Mat mask=getMask(id);
        Mat no
        countNonZero(mask););
        
        for (int n = 0; n < (int)nonZeroCoordinates.total(); n++ )
        {
            int i=nonZeroCoordinates.at<Point>(n).x;
            int j=nonZeroCoordinates.at<Point>(n).y;
            
            float di = getDepthFromPixel(i, j);
            //depth de cada pixel
            //  float di= getDepthInitialFromPixel(i, j);
              if (di > 255.0 || di < 0)
             cout << "MALLLLLL" << endl;*/
            //añadir ecuacion coste unario
            // if (di != 0.0)
            //  {
            //COSTE UNARIO
             /*Optimization::SparseEquation<float> eq;
             eq.a[id]=1.0;//w_u;//*id;
             eq.b=di;
             unary.add_equation(eq);
             numU++;
             //  }
             
             }*/
        
      //  printf("\t * TIEMPO  1 ecuacion unarioa: %f seconds\n ",((double)(clock() - start) /CLOCKS_PER_SEC));
        
    }
    void SuperPixels::addEquationsUnaries(int id,float di)
    {
        printf("--------> ID %d\n",id);
        if (id < 0 || id > maxID )
            return;
        
        //!!!!! añadir por pixel no por superpixels
        
        Mat mask=getMask(id);
        _depth.setTo(di,mask);
        arraySP[id].depth=di;
        
        Mat image=_depth*255.0;
        image.convertTo(image, CV_8UC1);
        
        cvtColor(image, _image,CV_GRAY2RGB);
        imshow("unaries",_image);
      /*  resize(image, image, Size(640,480));
        imshow("image",image);//*/
        
      //  imwrite("/Users/acambra/Desktop/userInput.png", image);
        // _image = image.clone();
        // _imDepth.setTo(di*255,mask);
        
        
        /* Mat nonZeroCoordinates;
         findNonZero(mask, nonZeroCoordinates);
         
         for (int n = 0; n < (int)nonZeroCoordinates.total(); n++ )
         {
         // int i=nonZeroCoordinates.at<Point>(n).x;
         // int j=nonZeroCoordinates.at<Point>(n).y;
         
         //float di = (float)_pixelDepth.at<float>(j,i);*/
         /*  if (di > 255.0 || di < 0)
         cout << "MALLLLLL" << endl;*/
        //añadir ecuacion coste unario
        /* if (di != 0.0)
         {
         //COSTE UNARIO
         Optimization::SparseEquation<float> eq;
         eq.a[id]=1.0;//w_u;//*//*id;
         eq.b=di;
         unary.add_equation(eq);
         // numU++;
         }
         
         }*/
    }
    
    void SuperPixels::solve()
    {
        //resolver sistema
        
       // int numU=0;
        
        clock_t start = clock();
        
        //ecuaciones unarias... Construir
      /*  for (unsigned int id=0; id< maxID +1; id++) {
            
            float d=arraySP[id].depth;
            
            if(d > 0.0)
            {
                //indices de pixels
                // Mat mask=getMask(id);
                Mat nonZeroCoordinates;
                findNonZero(getMask(id), nonZeroCoordinates);
                
                for (int n = 0; n < (int)nonZeroCoordinates.total(); n++ )
                {
                    int i=nonZeroCoordinates.at<Point>(n).x;
                    int j=nonZeroCoordinates.at<Point>(n).y;
                    
                    float di = getDepthFromPixel(i, j);
                    //depth de cada pixel
                    //  float di= getDepthInitialFromPixel(i, j);
                    /*  if (di > 255.0 || di < 0)
                     cout << "MALLLLLL" << endl;*/
                    //añadir ecuacion coste unario
                    // if (di != 0.0)
                    //  {
                    //COSTE UNARIO
                /*   Optimization::SparseEquation<float> eq;
                    eq.a[id]=1.0;//w_u;//*id;
                    eq.b=di;
                    unary.add_equation(eq);
                    numU++;
                    //  }
                    
                }
                
            }
        }*/
        
        //pintar imagen userInput en sup
        //pintar superpixels
        
        /*Mat depthSP = _depth.clone();
        for (unsigned int id=0; id< maxID +1; id++) {
            Mat mask=getMask(id);
           
            depthSP.setTo(getDepth(id),mask);
        }
        
        
        Mat image=depthSP*255.0;
        image.convertTo(image, CV_8UC1);
        
        cvtColor(image, image,CV_GRAY2RGB);
        imwrite("/Users/acambra/Desktop/userInput_Sp.png", image);*/

        
        Optimization::LeastSquaresLinearSystem<double> unaryN;
        
        unaryN = unary;
        
        unaryN.normalize();
        
       // printf("TIEMPO %d ecuaciones Unarias: %f seconds\n ",numU,((double)(clock() - start) /CLOCKS_PER_SEC));
        // cout << "Unary A: " << unary.getEigenValuesA() << endl;
        
        
        double w_u = this->w_unary;//0.3;
        float w_color=this->w_color;
        
        Optimization::LeastSquaresLinearSystem<double> B = (binary*(double)(1.0 - w_color)) + (binaryCOLOR*(double)(w_color));
        
        
        printf("w_u: %f binary: %f color: %f",w_u,(1.0 - w_u)*(1.0 - w_color),((1.0 - w_u)*(w_color)));
        
        B.normalize();
        ecuaciones = (unaryN * (double)w_u )+ (B * (double)(1.0 - w_u ));
        
        //ecuaciones = (unary * (float)w_u )+ (binary*(float)((1.0 - w_u)*(1.0 - w_color)))
        //+(binaryCOLOR*(float)((1.0 - w_u)*(w_color)));
        
        //   ecuaciones = (unary * (float)w_u )+ (binary*(float)((1.0 - w_u)));
        //*/
        //cout << "Num ecuaciones unarias: " << numU << " binarias: " << numB << endl;
        // cout << "------> Resolver sistema : ";
        
        MatrixFX x(maxID+1);
        ecuaciones.normalize();
        
        //DATOS ADOLFO
        //MatrixFX eigen(maxID+1);
        // ecuaciones.getEigenValuesA();
        
        start = clock();
        x = ecuaciones.solve();
        
         printf("\t * TIEMPO  Resolver sistema de ecuaciones %f seconds \n ",((double)(clock() - start) /CLOCKS_PER_SEC));
        //FILE *out=fopen("/Users/acambra/Dropbox/test_cvpr/tsukuba/resul/SLIC_15_MRFlineal.txt","wb+");
       // printf("Resolver sistema %d en %f segundos\n",maxID,((double)(clock() - start) /CLOCKS_PER_SEC));
       // printf("\n\n *Solve system %f seconds pixels: %d sup: %d\n ",((double)(clock() - start) /CLOCKS_PER_SEC),_image.rows*_image.cols,maxID+1);
        // fprintf(out,"Resolver sistema %f segundos\n",((double)(clock() - start) /CLOCKS_PER_SEC));
        //poner en _im la imagen a mostrar
        // fclose(out);
        
        start = clock();
        Mat  final=Mat::ones(_depth.rows,_depth.cols,CV_32F);
        
        //pinta imagen resultado
        // double min=10000, max=0;
        for (unsigned int id=0; id < maxID +1 ; id++) {
            
            /*if (x(id,0)>max)
             max = x(id,0);
             if (x(id,0)<min)
             min = x(id,0);*/
            
            // printf("solve %d = %f\n",id,x(id,0));
            Scalar color(x(id,0));
            final.setTo(color, getMask(id));
        }
        
       // char name [50],nameFile[100];//nameBin[200];
       // sprintf (name, "w_u_%0.1f bn_%0.4f bc_%0.4f", w_u, (1.0 - w_u)*(1.0 - w_color), (1.0 - w_u)*(w_color));
        //  sprintf (nameFile,"/Users/acambra/Dropbox/test_cvpr/tsukuba/resul/SLIC_65_MRFlineal_%s.png",name);
        //  sprintf nameBin,"/Users/acambra/Dropbox/test_cvpr/tsukuba/results_tsukuba/MRFlineal_%s.bin\n",name);
        // std::string fileBin = "/Users/anacambra/Desktop/linealMRF_tsukuba_0_7.bin";
        /*FILE *fd= fopen("/Users/acambra/Desktop/255_LLS-MRF_teddy.bin","wb+");
         // FILE *fd= fopen("/Users/acambra/Desktop/SLIC_65_MRFlineal.bin","wb+");
         for (int i=0; i < final.cols; i++)
         for (int j=0; j < final.rows; j++)
         fwrite(&final.at<float>(j,i),1,sizeof(float),fd);
         fclose(fd);//*/
        
        
        
        // string title="w_u: "+ w_u +"binary: %f "+"color: %f";,(1.0 - w_u)*(1.0 - w_color),((1.0 - w_u)*(w_color)))";
        _pixelDepth = final.clone();
        
        imwrite("d_depth.png", _pixelDepth);
        
        final=final*255;
        
        
        final.convertTo(final,CV_8UC1);
        //  imwrite("/Users/acambra/Desktop/255_LLS-MRF_teddy.png",final);
        
        cvtColor(final, _image,CV_GRAY2RGB);
        
        printf("\t * TIEMPO Generar depth map %f seconds \n ",((double)(clock() - start) /CLOCKS_PER_SEC));
        
        //printf("Pintar la depth %d en %f segundos\n",maxID,((double)(clock() - start) /CLOCKS_PER_SEC));
        //printf("\n\n *DEPTH MAP %f seconds pixels: %d sup: %d\n ",((double)(clock() - start) /CLOCKS_PER_SEC),_image.rows*_image.cols,maxID+1);
        /*   imshow(name,final);
         cv::waitKey(0);//*/
        
        
        
    }
    
    void SuperPixels::infoSuperpixels2file(std::string nameFile)
    {
        //numSuperpixels rows cols
        FILE *f;
        try{
            f = fopen(nameFile.c_str(),"w");
            fprintf(f,"//numSuperpixels image.rows image.cols\n");
            fprintf(f,"%d %d %d\n",maxID+1,_ids.rows,_ids.cols);
            fprintf(f,"//id\tLeftUp RightDown width height\tdepth acc var\n");
            for(int id=0; id<= maxID;id++)
            {
                fprintf(f,"%i\t",id);
                Mat mask_im;
                mask_im =(_ids == (float)id);
                // imshow("mask 1",mask_im);
                // mask_i = (_ids == (float)(i));
                //dilata la mascara 1 pixels 8-vecinas
                // dilate(mask_im, mask_im, Mat(), Point(-1, -1), 2, 1, 1);
                // imshow("mask 2",mask_im);
                vector<vector<Point> > contours;
                vector<Vec4i> hierarchy;
                
                /// Find contours
                findContours( mask_im, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
                vector<vector<Point> > contours_poly( contours.size() );
                vector<Rect> boundRect( contours.size() );
                vector<Point2f>center( contours.size() );
                vector<float>radius( contours.size() );
                
                for( int i = 0; i < contours.size(); i++ )
                {
                    //approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
                    boundRect[i] = boundingRect( Mat(contours[i]) );
                    
                }
                
                // _image.setTo(Scalar(0,0,0), mask_im);
                // Mat out = _image.clone();
                //  for( int i = 0; i< contours.size(); i++ )
                // {
                //Scalar color = Scalar( 255, 0, 0 );
                //rectangle( out, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
                fprintf(f,"%d %d %d %d %d %d\t%f %f %f\n",boundRect[0].tl().x,boundRect[0].tl().y,boundRect[0].br().x,boundRect[0].br().y,boundRect[0].width , boundRect[0].height,arraySP[id].d_median,arraySP[id].d_acc,arraySP[id].d_var);
                
                // }
                
                
                //imshow("out",out);
                //waitKey(500);
                
            }
            
            fclose(f);
            
        }catch(int e)
        {
        }
        
    }
    //a y b son los id de los superpixels
    float SuperPixels::cmpLab(int a, int b, int mode)
    {
        float cl,ca,cb;
        cl=arraySP[a].cmpHistogram(arraySP[a].hist_l,arraySP[b].hist_l, mode);
        ca=arraySP[a].cmpHistogram(arraySP[a].hist_a,arraySP[b].hist_a, mode);
        cb=arraySP[a].cmpHistogram(arraySP[a].hist_b,arraySP[b].hist_b, mode);
        
        return (cl + ca + cb)/3.0f;
    }
    
    float SuperPixels::cmpLab(int a, SuperPixel b, int mode)
    {
        float cl,ca,cb;
        cl=arraySP[a].cmpHistogram(arraySP[a].hist_l,b.hist_l, mode);
        ca=arraySP[a].cmpHistogram(arraySP[a].hist_a,b.hist_a, mode);
        cb=arraySP[a].cmpHistogram(arraySP[a].hist_b,b.hist_b, mode);
        return 1;//(cl + ca + cb)/3.0f;
    }
    
    float SuperPixels::cmpDepth(int a, int b, int mode)
    {
        return arraySP[a].cmpHistogram(arraySP[a].hist_depth,arraySP[b].hist_depth, mode);
    }
    
    void SuperPixels::paintZeroDepth()
    {
        //pintar en _sobel aquellos superpixels sin depth
        Mat mask= (_depth > 0.0);
        Mat d_zero;
        //_sobel es GRAY
        bitwise_and(_sobel,mask,d_zero);
        cvtColor(d_zero,_image,CV_GRAY2RGB);
    }
    
    //pone en _imagen la imagen de las profundidades con los contornos en azul
    void SuperPixels::depthWithBorders(){
        
        // cvtColor(_imDepth,_image,CV_GRAY2RGB);
        Scalar* color = new cv::Scalar( 0, 0, 255 );
        Mat mask_sobel = (_sobel == 0);
        _image.setTo(*color,mask_sobel);
    }
    void SuperPixels::updateDepth(Mat depth)
    {
        
        //depth CV_8UC1
        _imDepth = depth.clone();
        resetImage();
        
        depthWithBorders();
        
    }
    
    void SuperPixels::resetImage()
    {
        //reset image
        cvtColor(_imDepth,_image,CV_GRAY2RGB);
    }
    
    Mat SuperPixels::paintSuperpixel(int x,int y,Scalar* color)
    {
        int id;
        float depth;
        // Scalar* color;
        
        id= getIdFromPixel(x,y);
        
        Mat mask_im;
        mask_im = (_ids == (float)id);
        Mat im;
        
        //resetImage();
        
        depthWithBorders();
        
        depth = getDepthFromPixel(x,y);
        im=_image;
        
        if (depth != 0)
        {
            //color= new cv::Scalar( 0, 255, 0 );
            
            im.setTo(*color, mask_im);
        }
        return im;
        
    }
    Mat SuperPixels::paintSuperpixel(int x,int y)
    {
        int id;
        float depth;
        Scalar* color;
        
        id= getIdFromPixel(x,y);
        
        Mat mask_im;
        mask_im = (_ids == (float)id);
        Mat im;
        
        //resetImage();
        
        depthWithBorders();
        
        depth = getDepthFromPixel(x,y);
        im=_image;
        
        if (depth != 0)
        {
            color= new cv::Scalar( 0, 255, 0 );
            
            im.setTo(*color, mask_im);
        }
        return im;
    }
    
    Mat SuperPixels::getMask(int id)
    {
        Mat mask_im;
        mask_im = (_ids == (float)id);
        return mask_im;
        
    }
    
    void SuperPixels::paintSuperpixelByID(int id,int grayLevel)
    {
        //int id;
        //float depth;
        Scalar* color;
        
        //id= getIdFromPixel(x,y);
        
        Mat mask_im;
        mask_im = (_ids == (float)id);
        Mat im;
        
        // resetImage();
        
        // depthWithBorders();
        
        color= new cv::Scalar( grayLevel, grayLevel, grayLevel );
        
        _image.setTo(*color, mask_im);
        
    }
    
    
    void SuperPixels::calcularVecinos2(int id, int *array)
    {
        //int id=0;
        // start timer
        //clock_t start = clock();
        
        //pixels frontera de un superpixels id
        // for (int id=0; id <=maxID; id++)
        // {
        Mat mask_i;
        mask_i = (_ids == (float)(id));
        Mat sobel = (_sobel == 0);
        bitwise_and(mask_i,sobel,mask_i);
        
        //obtener coordenadas pixels frontera
        Mat nonZeroCoordinates;
        
        int numVecinas= maxID-id+1;
        //array
        // int count[numVecinas];
        //array = new int[numVecinas];
        
        //inicializar a cero
        for (int n = 0; n < numVecinas; n++)
            array[n]=0;
        
        findNonZero(mask_i, nonZeroCoordinates);
        Scalar* color = new cv::Scalar( 255, 0, 0 );
        _image.setTo(*color, mask_i);
        for (int n = 0; n < (int)nonZeroCoordinates.total(); n++ ) {
            //printf("(%d, %d)\n", nonZeroCoordinates.at<Point>(i).x , nonZeroCoordinates.at<Point>(i).y);
            
            // image.at<uchar>(nonZeroCoordinates.at<Point>(i).x ,
            //                 nonZeroCoordinates.at<Point>(i).y)=255;
            //
            //comprobar las 8-vecinas de (i,j)
            int i=nonZeroCoordinates.at<Point>(n).x;
            int j=nonZeroCoordinates.at<Point>(n).y;
            
            for (int v_i=i-1; v_i <= i+1; v_i++)
            {
                for (int v_j=j-1; v_j <= j+1; v_j++)
                {
                    //comprobar los indices
                    if ( (v_i>=0 && v_j>=0 && v_i < mask_i.cols && v_j < mask_i.rows))
                    {
                        int v= getIdFromPixel(v_i, v_j);
                        //solo me interesan las vecinas mayores q yo; para no repetir vecindad
                        if (v > id)
                            array[v-id]=array[v-id]+1;
                    }
                }
            }
            
            
        }
        
        
        // }
        
        // stop timer
        /*  clock_t finish = clock();
         float tiempo = (float) (((double)(finish - start)) / CLOCKS_PER_SEC);
         printf("Tiempo total: (%f secs)\n", tiempo);*/
        
        
    }
    
    void SuperPixels::calcularVecinos()
    {
        //VECINAS
        
        printf("\nTOTAL SUPERPIXELS: %d\n",maxID);
        Scalar* color = new cv::Scalar( 255, 0, 0 );
        // start timer
        clock_t start = clock();
        
        
        for (int i=0; i <= maxID-1; i++)
        {
            Mat mask_i;
            mask_i = (_ids == (float)(i));
            //dilata la mascara 1 pixels 8-vecinas
            dilate(mask_i, mask_i, Mat(), Point(-1, -1), 2, 1, 1);
            for (int j=i+1; j <= maxID; j++)
            {
                
                Mat mask_j= (_ids == (float)(j));
                //interseccion
                Mat intersection;
                bitwise_and(mask_i, mask_j, intersection);
                int num;
                if ((num=countNonZero(intersection)) > 0)
                {
                    // if (i==id)
                    // {
                    //son vecinas
                    printf("* i=%d es vecina de j=%d con %d pixels vecinos\n",i,j,num);
                    //pintar sus vecinas en rojo
                    
                    _image.setTo(*color, intersection);
                    // }
                }
            }
            
            // stop timer
            clock_t finish = clock();
            float tiempo = (float) (((double)(finish - start)) / CLOCKS_PER_SEC);
            printf("Tiempo total: (%f secs)\n", tiempo);
            
        }
        
    }
    
    //void SuperPixels::calcularVecinosDepth()
    void calcularVecinosDepth()
    {
        
    }
    
    
    void SuperPixels::copyDepth(int x, int y, float depth)
    {
        //pintar en azul (x,y) y modificar _depth
        int id= getIdFromPixel(x,y);
        Mat mask_im;
        mask_im = (_ids == (float)id);
        
        int grey= depth*255;
        Scalar levelgrey( grey,grey,grey);//(int)d*255, (int)d*255, (int)d*255);
        //_image.setTo(levelgrey, mask_im);
        _depth.setTo((double)depth,mask_im);
        
        //modificar _imDepth que es la imagen en niveles de grises
        _imDepth.setTo(levelgrey,mask_im);
        bitwise_and(mask_im,_sobel,mask_im);
        _sobel.setTo(255,mask_im);
        depthWithBorders();
        
        Scalar* color= new cv::Scalar( 0, 0, 255);
        _image.setTo(*color, mask_im);
        
    }
    void SuperPixels::copySuperpixel(int x,int y, int last_x,int last_y)
    {
        float d= getDepthFromPixel(last_x,last_y);
        
        copyDepth(x,y,d);
        
    }
    int SuperPixels::getIdFromPixel(int x, int y)
    {
        return (int)_ids.at<float>(y,x);
    }
   /* float SuperPixels::setDepthFromPixel(int x, int y)
    {
        return (float)_depth.at<float>(y,x);
    }*/

    float SuperPixels::getDepthFromPixel(int x, int y)
    {
        return (float)_depth.at<float>(y,x);
    }
    float SuperPixels::getDepthInitialFromPixel(int x, int y)
    {
        return (float)_pixelDepth.at<float>(y,x);
    }
    
    bool SuperPixels::isNotNullImage()
    {
        return (_image.data != NULL);
    }
    
    bool SuperPixels::isNotNullDepth()
    {
        return (_depth.data != NULL);
    }
    
    bool SuperPixels::isNotNullIndex()
    {
        return (_ids.data != NULL);
    }
    
    float SuperPixels::getDepth(int i){
        
        return this->arraySP[i].depth;//(this->arraySP[i].d_mean);//.d_median);
        
    }
    
    float SuperPixels::getVar(int i){
        
        return (this->arraySP[i].d_var);
    }
    
    float SuperPixels::getAccu(int i)
    {
        return (this->arraySP[i].d_acc);
    }

    Mat SuperPixels::blurImageDepth(const cv::Mat& image, const cv::Mat& depth,
                             int nbins, float focal_distance, float focal_length, float aperture, bool linear)
    {
        
        
        Mat f1= blur_image_depth(image, depth, nbins,focal_distance,focal_length,aperture, linear);
        return f1;
    }

    Mat SuperPixels::blurImage(Mat image, Mat imageDepth, int nbins,double minFocus, double maxFocus,double size)
    {
        
        /* if (focus >= nbins)
         return Mat::zeros(image.rows, image.cols, image.type());*/
        //nbins=16;
        // double minFocus=128, maxFocus=170; //depths enfocadas
            
        
        double min, max,diff;
        minMaxLoc(imageDepth, &min, &max);
        diff=max-min;
        
        int minB =  (int) nbins - ( (minFocus - min) / (diff/nbins));
        int maxB =  (int) nbins - ( (maxFocus - min ) / (diff/nbins));
        
        printf("Min: %f minFocus: %f minb: %d Max: %f maxFocus: %f maxB: %d\n",min, minFocus,minB,max,maxFocus,maxB );
        
        if (minB < 0  && maxB < 0)
             return image;
        
        clock_t start = clock();
        
        
        Mat temp;
        Mat final = Mat::ones(image.rows, image.cols, image.type());
        
        float depthMin = max -  (diff/nbins);
        float depthMax = max;
        int n = 0;
        Mat mask,m = Mat::ones(image.rows, image.cols, image.type());
        
        
        while (n<=nbins)//(depthMin >= min)
        {
            Mat mask1 = (imageDepth >= depthMin );
            Mat mask2 = (imageDepth <= depthMax);
            bitwise_and( mask1, mask2, mask);
            cvtColor(mask,mask,CV_GRAY2RGB);
            
            
            
            if (n > minB || n < maxB)//(n!=0) //blur a los bins distintos de focus
            {
                //desenfocar segun numero de bins de distancia
                int diffB;
                
                if (n > minB)
                    diffB = n - minB;
                else
                    diffB = maxB - n;
                
                diffB=diffB+size;
                Mat imageBlur;
                GaussianBlur(image, imageBlur, Size((diffB*2-1), (diffB*2-1)),0);
                
                // blur(image, imageBlur, Size(n,n));
            
                // blur(image, imageBlur, Size((diffB*2-1), (diffB*2-1)));//impar
               
                bitwise_and(imageBlur, mask, temp);
                
                printf("n: %d depthmin: %f depthmax: %f DIFF: %d \n",n,depthMin ,depthMax, diffB );
            }
            else
            {
                bitwise_and(image, mask, temp);
                printf("FOCUS n: %d depthmin: %f depthmax: %f \n",n,depthMin ,depthMax );
            }
            
            
            
            n=n+1;
            depthMin = depthMin - (diff/(float)nbins);
            depthMax = depthMax - (diff/(float)nbins);
            
            final= final + temp;
            
        }
        //printf("\n\n *BLUR image %f seconds pixels: %d sup: %d\n ",((double)(clock() - start) /CLOCKS_PER_SEC),_image.rows*_image.cols,maxID+1);
        
        printf("\t * TIEMPO Blur imagen %f seconds \n ",((double)(clock() - start) /CLOCKS_PER_SEC));
        return final;
    }
    
    Mat SuperPixels::blurImage(Mat image, Mat imageDepth, int nbins)
    {
        
        double min, max,diff;
        minMaxLoc(imageDepth, &min, &max);
        diff=max-min;
        printf("Min: %f Max: %f \n",min ,max );
        Mat temp;
        Mat final = Mat::ones(image.rows, image.cols, image.type());
        
        float depthMin = max -  (diff/(float)nbins);
        float depthMax = max;
        int n = 0;
        Mat mask,m = Mat::ones(image.rows, image.cols, image.type());
        while (n<nbins)//(depthMin >= min)
        {
            Mat mask1 = (imageDepth >= (int)depthMin );
            Mat mask2 = (imageDepth <= (int)depthMax);
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
            printf("depthmin: %f depthmax: %f \n",depthMin ,depthMax );
            n=n+1;
            depthMin = depthMin - (diff/(float)nbins);
            depthMax = depthMax - (diff/(float)nbins);
            
            final= final + temp;
            
        }
        
        return final;
    }
    
    
    
    
    /**
     * Destructor
     */
    SuperPixels::~SuperPixels() {
        ~_image;       // Libera la memoria
        ~_ids;
        ~_depth;
    }
    /*////////////////////////////////////////////////////*/
    
    /////////////////AUX opencv con qt
    
    /*//QImage point to the data of mat
     QImage SuperPixels::mat_to_qimage_ref(cv::Mat &mat, QImage::Format format)
     {
     return QImage(mat.data, mat.cols, mat.rows, mat.step, format);
     }
     
     //cv::Mat point to the data of QImage
     cv::Mat qimage_to_mat_ref(QImage &img, int format)
     {
     return cv::Mat(img.height(), img.width(),
     format, img.bits(), img.bytesPerLine());
     }
     
     //copy the data of cv::Mat to QImage
     QImage mat_to_qimage_cpy(cv::Mat const &mat,
     QImage::Format format)
     {
     return QImage(mat.data, mat.cols, mat.rows,
     mat.step, format).copy();
     }
     */
    //copy the data of QImage to cv::Mat
    /*cv::Mat qimage_to_mat_cpy(QImage const &img, int format)
     {
     //  return cv::Mat(img.height(), img.width(), format,
     //           const_cast<uchar>(img.bits()),
     //         img.bytesPerLine()).clone();
     }*/