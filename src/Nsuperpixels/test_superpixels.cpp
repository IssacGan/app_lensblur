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
    SuperPixels *SPCTE;
    
    String nameImage ="rocas.jpg";
    //"/Users/acambra/TESIS/code/test/build/Debug/test.png";//
    String nameInput = "rocas_userInput.png";
    
    //"/Users/acambra/TESIS/code/test/build/Debug/userInput_p.png";
    
    //cargar imagen original
    SP = new SuperPixels(nameImage);
    
    SP->loadSuperPixels("");
    SP->loadDepthBuildSystemCoef(nameInput);
    SP->addEquationsBinariesBoundariesCoef(false);    
    SP->addEquationsUnariesCoef();
    SP->solveCoef();
    
    
    imshow("solution lineal",SP->getImage());//*/
    //waitKey(0);
    
     SPCTE = new SuperPixels(nameImage);
     SPCTE->loadSuperPixels("");
     SPCTE->loadDepthBuildSystem(nameInput);
     SPCTE->addEquationsUnaries();
     SPCTE->solve();
     imshow("solution CTE",SPCTE->getImage());//*/
    waitKey(0);
    
    return 0;
}