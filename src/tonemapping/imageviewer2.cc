 /****************************************************************************
 **
 ** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 ** Contact: http://www.qt-project.org/legal
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** You may use this file under the terms of the BSD license as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
 **     of its contributors may be used to endorse or promote products derived
 **     from this software without specific prior written permission.
 **
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

 #include "imageviewer2.h"

#include <stdio.h>
#include "mythread.h"

 ImageViewer::ImageViewer()
 {
    
     this->focusWidget = new WidgetImage();
     
     //lo asigno para que se vea
     setCentralWidget(focusWidget);//setCentralWidget(imageWidget);

     createActions();
     createMenus();

     //setWindowTitle(tr("Image Viewer"));

     //setWindowState(Qt::WindowMaximized);

     //backThread to loadData
     backThread = new MyThread(this->focusWidget);
     //QObject::connect(backThread, SIGNAL(finished()),  focusWidget, SLOT(stopProgressBar()));

 }

 void ImageViewer::open()
 {
    QString fileName = QFileDialog::getOpenFileName(this,
                                     tr("Open File"), QDir::currentPath());

   //QApplication::setOverrideCursor(Qt::BusyCursor);

    if (fileName.isEmpty())
    {
            QMessageBox::information(this, tr("Image Viewer"),
                                          tr("Cannot load %1.").arg(fileName));
            return;
    }
    else
    {
        if (backThread->isRunning())
            QMessageBox::information(this, tr("Image Viewer"),
                                          tr("Busy"));
        else
        {
            backThread->setFile(fileName);
            backThread->setMode(0);
            backThread->start();
        }
    }
 }

void ImageViewer::openImage()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"), QDir::currentPath());
    
    QApplication::setOverrideCursor(Qt::BusyCursor);
    
    if (fileName.isEmpty())
    {
        QMessageBox::information(this, tr("Image Viewer"),
                                 tr("Cannot load %1.").arg(fileName));
        return;
    }
    else
    {
        if (backThread->isRunning())
            QMessageBox::information(this, tr("Image Viewer"),
                                     tr("Busy"));
        else
        {
            backThread->setFile(fileName);
            backThread->setMode(1);
            backThread->start();
        }
    }
}

void ImageViewer::loadSuperpixels()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"), QDir::currentPath());
    
    QApplication::setOverrideCursor(Qt::BusyCursor);
    
    if (fileName.isEmpty())
    {
        QMessageBox::information(this, tr("Image Viewer"),
                                 tr("Cannot load %1.").arg(fileName));
        return;
    }
    else
    {
        if (backThread->isRunning())
            QMessageBox::information(this, tr("Image Viewer"),
                                     tr("Busy"));
        else
        {
            backThread->setFile(fileName);
            backThread->setMode(2);
            backThread->start();
        }
    }
}
void ImageViewer::loadDepth()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"), QDir::currentPath());
    
    QApplication::setOverrideCursor(Qt::BusyCursor);
    
    if (fileName.isEmpty())
    {
        QMessageBox::information(this, tr("Image Viewer"),
                                 tr("Cannot load %1.").arg(fileName));
        return;
    }
    else
    {
        if (backThread->isRunning())
            QMessageBox::information(this, tr("Image Viewer"),
                                     tr("Busy"));
        else
        {
            backThread->setFile(fileName);
            backThread->setMode(3);
            backThread->start();
        }
    }
}

void ImageViewer::loadBuildSystem()
{
   
        if (backThread->isRunning())
            QMessageBox::information(this, tr("Image Viewer"),
                                     tr("Busy"));
        else
        {
           // backThread->getFile(fileName);
            backThread->setMode(4);
            backThread->start();
        }
}

void ImageViewer::save()
{
   
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if (backThread->isRunning())
        QMessageBox::information(this, tr("Image Viewer"),
                                 tr("Busy"));
    else
    {
        backThread->setDir(dir);
        backThread->setMode(5);
        backThread->start();
    }
    QMessageBox::information(this, tr("Image Viewer"),tr("DONE"));
}


 void ImageViewer::createActions()
 {
     openAct = new QAction(tr("&Load all..."), this);
     openAct->setShortcut(tr("Ctrl+O"));
     connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
     
    /* imageAct = new QAction(tr("&Open image..."), this);
     imageAct->setShortcut(tr("Ctrl+I"));
     connect(imageAct, SIGNAL(triggered()), this, SLOT(openImage()));*/
     
     /*superpixelsAct = new QAction(tr("&Load superpixels..."), this);
     superpixelsAct->setShortcut(tr("Ctrl+S"));
     connect(superpixelsAct, SIGNAL(triggered()), this, SLOT(loadSuperpixels()));*/

     /*equationAct = new QAction(tr("&Build system..."), this);
     equationAct->setShortcut(tr("Ctrl+B"));
     connect(equationAct, SIGNAL(triggered()), this, SLOT(loadBuildSystem()));*/
     
     depthAct = new QAction(tr("&Load depth..."), this);
     depthAct->setShortcut(tr("Ctrl+D"));
     connect(depthAct, SIGNAL(triggered()), this, SLOT(loadDepth()));
     
     saveAct = new QAction(tr("&Save"), this);
     saveAct->setShortcut(tr("Ctrl+S"));
     connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
     

     exitAct = new QAction(tr("&Exit"), this);
     exitAct->setShortcut(tr("Ctrl+Q"));
     connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
 }


 void ImageViewer::createMenus()
 {
     fileMenu = new QMenu(tr("&File"), this);
     fileMenu->addAction(openAct);
     
    // fileMenu->addSeparator();
    // fileMenu->addAction(imageAct);
     
    // fileMenu->addSeparator();
    // fileMenu->addAction(superpixelsAct);
    // fileMenu->addSeparator();
    // fileMenu->addAction(equationAct);
     fileMenu->addSeparator();
     fileMenu->addAction(depthAct);
     fileMenu->addSeparator();
     fileMenu->addAction(saveAct);

     fileMenu->addSeparator();
     fileMenu->addAction(exitAct);

     menuBar()->addMenu(fileMenu);

 }


