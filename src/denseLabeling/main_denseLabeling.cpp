//examples

// ./main_denseLabeling --image /Users/acambra/TESIS/images_test/rocas/rocas.jpg  --labels /Users/acambra/TESIS/images_test/rocas/rocas_userInput_p.png


#include "denseLabeling.cpp"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

int main(int argc, const char * argv[])
{
    
    //************
    string nameImage;
    string nameLabels;
    
    //************
    
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
    ("v", "debug mode ON")
    ("image", boost::program_options::value<std::string>(), "image")
    ("labels", boost::program_options::value<std::string>(), "user input")
    ("numLabels", boost::program_options::value<int>(), "num labels")
    ("help", "produce help message");
    
    
    boost::program_options::variables_map parameters;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc),parameters);
    
    if (parameters.count("help"))
    {
        std::cout << desc << std::endl;
        return 1;
    }
    
    boost::filesystem::path inputImage(parameters["image"].as<std::string>());
    nameImage = inputImage.string();
    
    
    //DENSE LABELING
    
    //1. Superpixels
    DenseLabeling *systemDepth;
    systemDepth =  new DenseLabeling(nameImage,0.3,0.99,10.0);
    
    //show superpixels
    Mat out1;
    hconcat(systemDepth->getImage(),systemDepth->getImageSuperpixels(),out1);
    imshow("image + supeprixels",out1);
    
    if (parameters.count("labels"))
    {
        boost::filesystem::path inputImage(parameters["labels"].as<std::string>());
        nameLabels = inputImage.string();
       
        //2. Unary Equations
        systemDepth->addEquations_Unaries(nameLabels);
        
        //3. Binary Equations
        //choose binary equations
        printf("\n System using addEquations_BinariesBoundaries\n");
        systemDepth->addEquations_BinariesBoundaries(false);
        Mat sol1 = systemDepth->solve();
        sol1 = sol1 * 255.0;
        sol1.convertTo(sol1,CV_8UC1);
        
        printf("\n System using addEquations_BinariesBoundariesPerPixel\n");
        systemDepth->addEquations_BinariesBoundariesPerPixel();
        Mat sol2 = systemDepth->solve();
        sol2 = sol2 * 255.0;
        sol2.convertTo(sol2,CV_8UC1);
        
        printf("\n System using addEquations_BinariesBoundariesPerPixelMean\n");
        systemDepth->addEquations_BinariesBoundariesPerPixelMean();
        
        //4. Solve System
        Mat sol3 = systemDepth->solve();
        sol3 = sol3 * 255.0;
        sol3.convertTo(sol3,CV_8UC1);
        
        Mat out2;
        hconcat(sol1,sol2,out2);
        hconcat(out2,sol3,out2);
        imshow("Binary: Mean mask + per pixel + mean per pixel ",out2);
    }
    
    waitKey(0);
    return 0;
}