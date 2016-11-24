//examples
// ./main_superpixels --image /Users/acambra/TESIS/images_test/rocas/rocas.jpg  --labels /Users/acambra/TESIS/images_test/rocas/rocas_userInput_p.png
// ./main_superpixels --image /Users/acambra/Desktop/fcn\ models/models/test/images/catdog_3.jpg  --numLabels 60 --labels /Users/acambra/Desktop/fcn\ models/models/test/images/input/catdog_3-fcn-8s.png

#include "superpixels.cpp"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

int main(int argc, const char * argv[])
{
    
    //************
    string nameImage;
    string nameLabels;
    
    SuperPixels *SPCTE;
    //************
    
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
    ("v", "debug mode ON")
    ("image", boost::program_options::value<std::string>(), "image")
    ("labels", boost::program_options::value<std::string>(), "image GT labeled")
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
    
    
    //SUPERPIXELS
    
    SPCTE = new SuperPixels(nameImage);
    
    Mat out1;
    hconcat(SPCTE->getImage(),SPCTE->getImageSuperpixels(),out1);
    imshow("image + supeprixels",out1);
    
    //input LABELS
    if (parameters.count("numLabels"))
    {
       SPCTE->setNUMLABELS(parameters["numLabels"].as<int>());
    }
    
    if (parameters.count("labels"))
    {
        boost::filesystem::path inputImage(parameters["labels"].as<std::string>());
        nameLabels = inputImage.string();

        SPCTE->initializeLabeling(nameLabels, MODE_LABEL_NOTZERO);//MODE_LABEL_MEDIAN);//
        
        imshow("labels input",SPCTE->getImageLabelsInput());
        imshow("labels",SPCTE->getImageLabels());
        
        
    }
    waitKey(0);
    return 0;
}