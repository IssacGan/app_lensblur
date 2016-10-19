#ifndef DENSE_LABELING
#define DENSE_LABELING
//
//  denseLabeling.cpp
//  
//
//  Created by Ana B. Cambra on 18/10/16.
//
//

#include <stdio.h>

#include "superpixels.cpp"
#include "least-squares-linear-system2.h"

using namespace Eigen;
typedef Matrix<double, Dynamic, 1> MatrixFX;

class DenseLabeling : public SuperPixels
{
    
    //SYSTEM
    int numUnknows;
    Optimization::LeastSquaresLinearSystem<double> equations;
    Optimization::LeastSquaresLinearSystem<double> unary;
    Optimization::LeastSquaresLinearSystem<double> binary;
    Optimization::LeastSquaresLinearSystem<double> binaryCOLOR;
    

    double w_unary;
    double w_color;
    
    int MAX_BINARIAS=1000000;
    double _MAX_DIFF_LAB;
    
    int num_u=0;
    
public:
    
    
    DenseLabeling(string path, double w_u=0.3, double w_c=0.99, double diff_lab=15.0) : SuperPixels(path)
    {
        this->w_unary = w_u;
        this->w_color = w_c;
        this->_MAX_DIFF_LAB = diff_lab;
        
        numUnknows = (this->maxID+1);
        
        this->unary = Optimization::LeastSquaresLinearSystem<double>((maxID+1));
        
    }
    
    Mat solve()
    {
        clock_t start = clock();
        
        Optimization::LeastSquaresLinearSystem<double> unaryN;
        
        unaryN = unary;
        unaryN.normalize();
        
        double w_u = this->w_unary;
        float w_color=this->w_color;
        
        Optimization::LeastSquaresLinearSystem<double> B = (binary*(double)(1.0 - w_color)) + (binaryCOLOR*(double)(w_color));
        
        B.normalize();
        equations = (unaryN * (double)w_u )+ (B * (double)(1.0 - w_u ));
        
        
        MatrixFX x(maxID+1);
        equations.normalize();
        
        x = equations.solve();
        
        float t = ((double)(clock() - start) /CLOCKS_PER_SEC);
        printf("\t * TIME  solve system %f seconds \n ",t);
        
        
        start = clock();
        
        Mat  final=Mat::ones(_labels.rows,_labels.cols,CV_32F);
        
        //pinta imagen resultado
        // double min=10000, max=0;
        for (unsigned int id=0; id < maxID +1 ; id++) {
            
            Scalar color(x(id,0));
            final.setTo(color, getMask(id));
            _labels.setTo(color,getMask(id));
        }
        
        t = ((double)(clock() - start) /CLOCKS_PER_SEC);
        printf("\t * TIME  build solution %f seconds \n ",t);
        
        return final.clone();
        
    }
    
    //UNARY EQUATIONS
    
    void addEquations_Unaries(string nameLabels)
    {
        clock_t start = clock();
        
        initializeLabeling(nameLabels, MODE_LABEL_NOTZERO);
        
        this->unary = Optimization::LeastSquaresLinearSystem<double>((maxID+1));
        num_u=0;
    
        //añadir todas las ecuaciones unarias form _labels
        for (unsigned int id=0; id< maxID +1; id++) {
            
            //indices de pixels
            // Mat mask=getMask(id);
            Mat nonZeroCoordinates;
            findNonZero(getMask(id), nonZeroCoordinates);
            
            for (int n = 0; n < (int)nonZeroCoordinates.total(); n++ )
            {
                int i=nonZeroCoordinates.at<Point>(n).x;
                int j=nonZeroCoordinates.at<Point>(n).y;
                
                float di = getLabel(i, j);
                
                if (di != 0.0)
                {
                    //UNARY COST
                    Optimization::SparseEquation<float> eq;
                    eq.a[id]=1.0;
                    eq.b=di;
                    unary.add_equation(eq);
                    num_u++;
                }
            }
        }
        
        float t = ((double)(clock() - start) /CLOCKS_PER_SEC);
        printf("\t * TIME  Unaries (%d) %f seconds \n ",num_u,t);
    }
    
    void addEquation_Unary(int x,int y,float li, bool saveInput=false)
    {
        
        clock_t start = clock();
        
        if (x < 0 || x > this->_image.cols || y < 0 || y >this->_image.rows)
            return;
        
        int id=getIdFromPixel(x, y);
        
        if (saveInput)
        {
            
            _labelsInput.at<float>(y,x) = li;
            
            Mat image=_labelsInput*255.0;
            image.convertTo(image, CV_8UC1);
        
            cvtColor(image, image,CV_GRAY2RGB);
            imshow("unaries",image);
            //imwrite("userInput.png", image);
        }
        
        if(li  > 0.0)
        {
            
            //COSTE UNARIO
            Optimization::SparseEquation<float> eq;
            eq.a[id]=1.0;
            eq.b=li;
            unary.add_equation(eq);
            num_u++;
        }
        
        float t = ((double)(clock() - start) /CLOCKS_PER_SEC);
        printf("\t * TIME  ONE Unary (%d) %f seconds \n ",num_u,t);
        
    }

    
    //BINARY EQUATIONS
    
    //compare color boundaries with image masks
    void addEquations_BinariesBoundaries(bool verbose=false)
    {
        clock_t start = clock();
        //bool save samples
        // bool saveIMAGES=true;
        //clock_t start = clock();
        
        //show debug windows
        //bool verbose = false;
        //resetear las ecuaciones de color
        this->binaryCOLOR = Optimization::LeastSquaresLinearSystem<double>(maxID+1);
        this->binary = Optimization::LeastSquaresLinearSystem<double>(maxID+1);
        
        int nB=0;
        int num_b=0;
        int num_bc=0;

        Mat _lab;
        cvtColor(this->_image,_lab,CV_BGR2Lab);
        
        
        for (int id1=0; id1<=maxID-1;id1++)
        {
            Mat mask,mask1,maski;
            
            bitwise_and(this->getMask(id1),(_sobel != 0),mask);
            
            //frontera dilatada: mask1
            int dilate_size=5;
            Mat element = getStructuringElement( MORPH_RECT,
                                                Size( 2*dilate_size + 1, 2*dilate_size+1 ),
                                                Point( dilate_size, dilate_size ) );
            
            //mask 1 = frontera dilatada del id1
            dilate(mask, mask1, element);
            //dilatar 1 pixel, para saber si se "tocan"
            dilate(mask, mask, Mat(), Point(-1, -1), 3, 1, 1);
            
            Mat n1,n2;
            
            for(int i=id1+1; i<=maxID; i++)
            {
                if (id1!=i)
                {
                    bitwise_and(this->getMask(i),(_sobel == 0),maski);
                    
                    //comprobar si son vecinos originales mask
                    Mat intersection;
                    bitwise_and(mask, maski, intersection);
                    int total=0;

                    if (countNonZero(intersection) > 0)
                    {
                        //frontera del vecino i
                        //n2 = frontera de id = i
                        bitwise_and(mask1, this->getMask(i), n2);
                        total = countNonZero(n2);
                        
                        //frontera con id1
                        dilate(maski, maski, element);
                        bitwise_and(maski, _arraySP[id1].getMask(), n1);
                        total = total + countNonZero(n1);
                        
                        cvtColor(n1,n1,CV_GRAY2BGR);
                        cvtColor(n2,n2,CV_GRAY2BGR);
                        
                        //ver superpixels
                        /*Mat m1,m2;
                        bitwise_or(this->getMask(i),this->getMask(id1), m1);
                        cvtColor(m1,m1,CV_GRAY2BGR);
                        bitwise_and(this->_image, m1, m1);*/
                        // imshow("superpixels",m1);
                       
                        
                        if (verbose) {
                            //pintar la frontera
                            //ver vecinos
                            Mat cmp1,cmp2;
                            bitwise_and(this->_image, n1, cmp1);
                            bitwise_and(this->_image, n2, cmp2);
                            bitwise_xor(cmp1, cmp2, cmp1);
                            
                            cmp1.setTo(Scalar(255,0,0),intersection);
                            imshow("Pixels frontera = " + to_string(dilate_size),cmp1);
                            
                            cmp1.release();
                            cmp2.release();
                            
                        }
                        
                        
                        //sum
                        //Mat im1,im2,im3,im4;
                        Scalar mean1,mean2,stddev1,stddev2;
                        
                        cvtColor(n1,n1,CV_BGR2GRAY);
                        // cvtColor(n2,n2,CV_Lab2BGR);
                        cvtColor(n2,n2,CV_BGR2GRAY);
                        
                        
                        // Scalar sum2 = sum(im2);
                        meanStdDev(_lab, mean1, stddev1,n1);
                        meanStdDev(_lab, mean2, stddev2,n2);
                        
                        
                        float diff =
                        sqrt((mean1[0]-mean2[0])*(mean1[0]-mean2[0])
                             + (mean1[1]-mean2[1])*(mean1[1]-mean2[1])
                             + (mean1[2]-mean2[2])*(mean1[2]-mean2[2]));
                        
                        Optimization::SparseEquation<float> eq;
                        eq.a[id1] = 1.0;
                        eq.a[i] = -1.0;
                        eq.b=0;
                        
                         if (verbose) printf("diff %f\n",diff);
                        
                        if (diff <= _MAX_DIFF_LAB)
                        {
                            binaryCOLOR.add_equation(eq);
                            num_bc++;
                        }else
                        {
                            binary.add_equation(eq);
                            num_b++;
                        }
                        
                        nB++;
                        
                        if (verbose) {
                            waitKey(0);
                        }
                        
                    }
                    
                    intersection.release();
                }
            }
            
            mask.release();
            mask1.release();
            maski.release();
            element.release();
            
            n1.release();
            n2.release();
        }
        
        //printf("\t * NUM BINARIES: %d \n ",nB);
        float t = ((double)(clock() - start) /CLOCKS_PER_SEC);
        printf("\t * TIME  Binaries (%d) color (%d) %f seconds \n ",num_b,num_bc,t);
        
        _lab.release();
        
        
        
    }
    
    //compare color pixel boundaries
    void addEquations_BinariesBoundariesPerPixel()
    {
        clock_t start = clock();
        //resetear las ecuaciones de color
        this->binaryCOLOR = Optimization::LeastSquaresLinearSystem<double>(maxID+1);
        this->binary = Optimization::LeastSquaresLinearSystem<double>(maxID+1);
        
        //recorrer sobel pixels
        //pixels forntera
        Mat nonZeroCoordinates;
        
        Mat sobel=_sobel;
        dilate(_sobel, sobel, Mat(), Point(-1, -1), 2, 1, 1);
        
        //imshow("sobel",sobel); waitKey(0);
        
        findNonZero((sobel != 0), nonZeroCoordinates);
        
        int num_b=0;
        int num_bc=0;
        /*int num = (int)nonZeroCoordinates.total()*0.050; //5%
         int n=0;//*/
        
        
        int size=maxID+1;
        vector<vector<int> > v_array(size,vector<int>(size));
        for (int i=0; i < size; i++)
            for (int j=0; j < size; j++)
                v_array[i][j]=-1;
        
        Mat _lab,noConnected;
        cvtColor(this->_image, _lab, CV_BGR2Lab);
        cvtColor(_lab,noConnected,CV_Lab2BGR);
        cvtColor(noConnected,noConnected,CV_BGR2GRAY);
        cvtColor(noConnected,noConnected,CV_GRAY2RGB);
        
        //aplicar filtro mediana a la image _lab
        Mat mlab = _lab;
        //medianBlur(_lab, mlab, 5);
        /* cvtColor(mlab,mlab,CV_Lab2BGR);
         imshow("Median",mlab);*/
        
        
        for (int n = 0; n < (int)nonZeroCoordinates.total(); n++ )
        {
            int x=nonZeroCoordinates.at<Point>(n).x;
            int y=nonZeroCoordinates.at<Point>(n).y;//*/
            
            int id1=getIdFromPixel(x, y);
            // printf("pixel %d %d de %d (cols, rows) (%d,%d)\n",x,y,id1,_sobel.cols,_sobel.rows);;//getchar();
            
            int i=0;
            int j=0;
            //vecinos del pixel (x,y)
            /*for ( i=(x-4); i<= (x+4); i++)
            {
                for ( j=(y-4); j<=(y+4); j++)*/ //79334
            for ( i=x; i<= (x+1); i++)
            {
                for ( j=y; j<=(y+1); j++)
                {
                   int id_v = getIdFromPixel(i, j);
                    //printf("vecino pixel %d %d de %d: \n",i,j,id_v);
                    if (
                         //((x!=i) && (y !=j)) &&
                        (i >= 0 && i <= _sobel.cols-1) &&
                        (j >= 0 && j <= _sobel.rows-1)
                        //solo 4 vecinas
                        /* &&(
                         ((i == (x-1)) && (j == (y)))  ||
                         ((i == (x+1)) && (j == (y)))  ||
                         ((i == (x)) && (j == (y-1)))  ||
                         ((i == (x)) && (j == (y+1))) )//*/
                        //solo 4 vecinas sigueintes
                       /* &&(
                           
                           ((i == (x+1)) && (j == (y)))  ||
                           ((i == (x)) && (j == (y+1))) )//*/
                        )
                    {
                        //superpixel vecino
                        //printf("check vecinos!!!!!! %d %d\n",id1,id_v);
                        
                        if (id1 != id_v)
                        {
                           
                            // pixel %d %d de %d: \n",i,j,id_v);
                            //mlab=_lab;
                            //color de _lab(i,j)
                            float diff = sqrt(
                                              (((double)(mlab.at<Vec3b>(y,x)[0])-(double)(mlab.at<Vec3b>(j,i)[0]))*
                                               ((double)(mlab.at<Vec3b>(y,x)[0])-(double)(mlab.at<Vec3b>(j,i)[0])))
                                              + (((double)(mlab.at<Vec3b>(y,x)[1])-(double)(mlab.at<Vec3b>(j,i)[1]))*
                                                 ((double)(mlab.at<Vec3b>(y,x)[1])-(double)(mlab.at<Vec3b>(j,i)[1])))
                                              + (((double)(mlab.at<Vec3b>(y,x)[2])-(double)(mlab.at<Vec3b>(j,i)[2]))*
                                                 ((double)(mlab.at<Vec3b>(y,x)[2])-(double)(mlab.at<Vec3b>(j,i)[2])))
                                              );
                            
                           // printf("diff: %f  \n",diff);
                            
                            //Eq binaria de color, boundaries
                            // a1*x1 + b1*y1 = a2*x2 + b2*y2
                            
                            if (i!=0 && j!=0 && x!=0 && y!=0 && id1 != id_v)
                            {
                                
                                //if (count[id_v-id1] != 0) printf("DIFF: %d con %d = %f exp=%f\n",id1,id_v,diff,exp(-diff));
                                
                                if ((diff <=_MAX_DIFF_LAB) )//&& (id_v > id1 ))
                                {
                                    Optimization::SparseEquation<float> eq;
                                    eq.a[id1] = 1.0;
                                    eq.a[id_v] = -1.0;
                                    eq.b=0;
                                    binaryCOLOR.add_equation(eq);
                                    
                                    num_bc++;
                                    
                                    //da igual q valor tenga poner a 0; significa q no habra q añadir a binry
                                    v_array[id1][id_v]=0; //para saber q estan coenctadas pero no son similares
                                    v_array[id_v][id1]=0;
                                    
                                    //pixel
                                    /* image.at<Vec3b> (j,i)[0]=255;
                                     image.at<Vec3b> (j,i)[1]=0;
                                     image.at<Vec3b> (j,i)[2]=0; //*/
                                    
                                    if (num_bc > MAX_BINARIAS) {
                                        break;
                                    }
                                }
                                else
                                {
                                    
                                    Optimization::SparseEquation<float> eq;
                                    eq.a[id1] = 1.0;
                                    eq.a[id_v] = -1.0;
                                    eq.b=0;
                                    
                                    binary.add_equation(eq);
                                    num_b++;
                                    
                                    //si es la primera vecina
                                    if (v_array[id1][id_v] == -1)
                                    {
                                        // printf("*BINARY: %d cpm %d\n",id1,id_v);
                                        v_array[id1][id_v]=1;
                                        v_array[id_v][id1]=1;
                                    }
                                    //si es 0 significa q no hay q añadirla
                                    //y si es 1; ya esta añadida
                                    
                                    //pixel
                                    /* noConnected.at<Vec3b> (j,i)[0]=0;//B
                                     noConnected.at<Vec3b> (j,i)[1]=0;//G
                                     noConnected.at<Vec3b> (j,i)[2]=255;//R*/
                                }
                                //al acabar:
                                //bynaryColor las vecinas similares; v(id1,v)=0
                                //bynary iran todas las otras v(id1,v)==1
                                if (num_bc > MAX_BINARIAS) {
                                    break;
                                }
                            }
                        }//ids !=
                    }
                }//for j
                if (num_bc > MAX_BINARIAS) {
                    break;
                }
                
            }//for i
            
            if (num_bc > MAX_BINARIAS) {
                break;
            }
            //n++;
        }//sobel
        
        //imshow
        //  imshow("boundariesConnected.png",image);
        //imshow
        //  imshow("boundariesNOconnected.png",noConnected);
        //waitKey(0);//*/
        
        //al acabar de comprobar todas las fronteras
        //unir solo las vecinas que no han sido unidas
       /* for (unsigned int id1=0; id1 < maxID+1;id1++)
        {
            for (unsigned int v=id1+1; v < maxID+1;v++)
            {
                if ((id1 != v)&&(v_array[id1][v] == 1 ))
                {
                    Optimization::SparseEquation<float> eq;
                    eq.a[id1] = 1.0;
                    eq.a[v] = -1.0;
                    eq.b=0;
                    
                    binary.add_equation(eq);
                    num_b++;
                }
            }
        }//*/
        
        float t = ((double)(clock() - start) /CLOCKS_PER_SEC);
        printf("\t * TIME  Binaries (%d) color (%d) %f seconds \n ",num_b,num_bc,t);
        /* printf("**** Binarias pixel: %f seconds\n ",(float) (((double)(clock() - start)) / CLOCKS_PER_SEC));
         printf("\t Equations color: %d others: %d\n ",num_bc,num_b);*/
        
    }
    
    //compare mean color pixel boundaries
    void addEquations_BinariesBoundariesPerPixelMean()
    {
        clock_t start = clock();
        //resetear las ecuaciones de color
        this->binaryCOLOR = Optimization::LeastSquaresLinearSystem<double>(maxID+1);
        this->binary = Optimization::LeastSquaresLinearSystem<double>(maxID+1);
        
        //recorrer sobel pixels
        //pixels forntera
        Mat nonZeroCoordinates;
        
        Mat sobel=_sobel;
        dilate(_sobel, sobel, Mat(), Point(-1, -1), 2, 1, 1);

        
        findNonZero((sobel != 0), nonZeroCoordinates);
        
        int num_b=0;
        int num_bc=0;
       
        
        int size=maxID+1;
        vector<vector<int> > v_array(size,vector<int>(size));
        vector<vector<int> > v_count(size,vector<int>(size));
        
        for (int i=0; i < size; i++)
            for (int j=0; j < size; j++)
            {
                v_array[i][j]=-1;
                v_count[i][j]=0;
            }
        
        Mat mlab;
        cvtColor(this->_image, mlab, CV_BGR2Lab);
        

        for (int n = 0; n < (int)nonZeroCoordinates.total(); n++ )
        {
            int x=nonZeroCoordinates.at<Point>(n).x;
            int y=nonZeroCoordinates.at<Point>(n).y;//*/
            
            int id1=getIdFromPixel(x, y);
           

            //vecinos del pixel (x,y)
            for (int i=x; i<= (x+1); i++)
            {
                for (int j=y; j<=(y+1); j++)
                {
                    int id_v = getIdFromPixel(i, j);
                    if ((i >= 0 && i <= _sobel.cols-1) &&
                        (j >= 0 && j <= _sobel.rows-1))
                    {
                        if (id1 != id_v)
                        {
                            //color de _lab(i,j)
                            float diff = sqrt(
                                              (((double)(mlab.at<Vec3b>(y,x)[0])-(double)(mlab.at<Vec3b>(j,i)[0]))*
                                               ((double)(mlab.at<Vec3b>(y,x)[0])-(double)(mlab.at<Vec3b>(j,i)[0])))
                                              + (((double)(mlab.at<Vec3b>(y,x)[1])-(double)(mlab.at<Vec3b>(j,i)[1]))*
                                                 ((double)(mlab.at<Vec3b>(y,x)[1])-(double)(mlab.at<Vec3b>(j,i)[1])))
                                              + (((double)(mlab.at<Vec3b>(y,x)[2])-(double)(mlab.at<Vec3b>(j,i)[2]))*
                                                 ((double)(mlab.at<Vec3b>(y,x)[2])-(double)(mlab.at<Vec3b>(j,i)[2])))
                                              );
                            
                            if (i!=0 && j!=0 && x!=0 && y!=0 && id1 != id_v)
                            {
                                
                                if (v_array[id1][id_v] == -1 || v_array[id_v][id1] == -1)
                                {
                                    v_array[id1][id_v]= diff;
                                    v_array[id_v][id1]= diff;
                                    
                                    v_count[id1][id_v]= 1;
                                    v_count[id_v][id1]= 1;
                                }
                                else
                                {
                                    v_array[id1][id_v] += diff;
                                    v_array[id_v][id1] += diff;
                                    
                                    v_count[id1][id_v]++;
                                    v_count[id_v][id1]++;
                                }
                                
                            }
                        }//ids !=
                    }
                }//for j
                
            }//for i
            
        }//for nonZero sobel
        
        //add eq
        for (unsigned int id1=0; id1 < maxID+1;id1++)
        {
            for (unsigned int v=id1+1; v < maxID+1;v++)
            {
                if (id1 != v)
                {
                    if (v_array[id1][v] != -1 )
                    {
                        float diff = (float)v_array[id1][v]/(float)v_count[id1][v];
                        
                        if ((diff <=_MAX_DIFF_LAB) )
                        {
                            Optimization::SparseEquation<float> eq;
                            eq.a[id1] = 1.0;
                            eq.a[v] = -1.0;
                            eq.b=0;
                            binaryCOLOR.add_equation(eq);
                            
                            num_bc++;
                        }
                        else
                        {
                            //eq normal
                            Optimization::SparseEquation<float> eq;
                            eq.a[id1] = 1.0;
                            eq.a[v] = -1.0;
                            eq.b=0;
                            
                            binary.add_equation(eq);
                            num_b++;
                        }
                    }
                }
            }
        }
    
        float t = ((double)(clock() - start) /CLOCKS_PER_SEC);
        printf("\t * TIME  Binaries (%d) color (%d) %f seconds \n ",num_b,num_bc,t);
        
    }

    
};

#endif //DENSE_LABELING
