//
//  main.cpp
//  test_superpixels
//
//  Created by Ana Cambra on 26/11/14.
//  Copyright (c) 2014 Ana Cambra. All rights reserved.
//

#include <iostream>
//Opencv
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <chrono>
#include <strings.h>
#include <sstream>
#include <iomanip>

#include "tonemapping/tonemapping.h"

/* utility for reading and writing Ward's rgbe image format.
   See rgbe.txt file for more details.
*/




/* default error routine.  change this to change error handling */
int rgbe_error(int rgbe_error_code, const char *msg)
{
  switch (rgbe_error_code) {
  case rgbe_read_error:
    perror("RGBE read error");
    break;
  case rgbe_write_error:
    perror("RGBE write error");
    break;
  case rgbe_format_error:
    fprintf(stderr,"RGBE bad file format: %s\n",msg);
    break;
  default:
  case rgbe_memory_error:
    fprintf(stderr,"RGBE error: %s\n",msg);
  }
  return RGBE_RETURN_FAILURE;
}

/* standard conversion from float pixels to rgbe pixels */
/* note: you can remove the "inline"s if your compiler complains about it */
inline void
float2rgbe(unsigned char rgbe[4], float red, float green, float blue)
{
  float v;
  int e;

  v = red;
  if (green > v) v = green;
  if (blue > v) v = blue;
  if (v < 1e-32) {
    rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
  }
  else {
    v = frexp(v,&e) * 256.0/v;
    rgbe[0] = (unsigned char) (red * v);
    rgbe[1] = (unsigned char) (green * v);
    rgbe[2] = (unsigned char) (blue * v);
    rgbe[3] = (unsigned char) (e + 128);
  }
}

/* standard conversion from rgbe to float pixels */
/* note: Ward uses ldexp(col+0.5,exp-(128+8)).  However we wanted pixels */
/*       in the range [0,1] to map back into the range [0,1].            */
inline void
rgbe2float(float *red, float *green, float *blue, unsigned char rgbe[4])
{
  float f;

  if (rgbe[3]) {   /*nonzero pixel*/
    f = ldexp(1.0,rgbe[3]-(int)(128+8));
    *red = rgbe[0] * f;
    *green = rgbe[1] * f;
    *blue = rgbe[2] * f;
  }
  else
    *red = *green = *blue = 0.0f;
}

/* default minimal header. modify if you want more information in header */
int RGBE_WriteHeader(FILE *fp, int width, int height, rgbe_header_info *info)
{
  char default_program_type[]="RGBE";
  char *programtype = default_program_type;

  if (info && (info->valid & RGBE_VALID_PROGRAMTYPE))
    programtype = info->programtype;
  if (fprintf(fp,"#?%s\n",programtype) < 0)
    return rgbe_error(rgbe_write_error,NULL);
  /* The #? is to identify file type, the programtype is optional. */
  if (info && (info->valid & RGBE_VALID_GAMMA)) {
    if (fprintf(fp,"GAMMA=%g\n",info->gamma) < 0)
      return rgbe_error(rgbe_write_error,NULL);
  }
  if (info && (info->valid & RGBE_VALID_EXPOSURE)) {
    if (fprintf(fp,"EXPOSURE=%g\n",info->exposure) < 0)
      return rgbe_error(rgbe_write_error,NULL);
  }
  if (fprintf(fp,"FORMAT=32-bit_rle_rgbe\n\n") < 0)
    return rgbe_error(rgbe_write_error,NULL);
  if (fprintf(fp, "-Y %d +X %d\n", height, width) < 0)
    return rgbe_error(rgbe_write_error,NULL);
  return RGBE_RETURN_SUCCESS;
}

/* minimal header reading.  modify if you want to parse more information */
int RGBE_ReadHeader(FILE *fp, int *width, int *height, rgbe_header_info *info)
{
  char buf[128];
//  int found_format;
  float tempf;
  unsigned int i;

//  found_format = 0;
  if (info) {
    info->valid = 0;
    info->programtype[0] = 0;
    info->gamma = info->exposure = 1.0;
  }
  if (fgets(buf,sizeof(buf)/sizeof(buf[0]),fp) == NULL)
    return rgbe_error(rgbe_read_error,NULL);
  if ((buf[0] != '#')||(buf[1] != '?')) {
    /* if you want to require the magic token then uncomment the next line */
    /*return rgbe_error(rgbe_format_error,"bad initial token"); */
  }
  else if (info) {
    info->valid |= RGBE_VALID_PROGRAMTYPE;
    for(i=0;i<sizeof(info->programtype)-1;i++) {
      if ((buf[i+2] == 0) || isspace(buf[i+2]))
	break;
      info->programtype[i] = buf[i+2];
    }
    info->programtype[i] = 0;
    if (fgets(buf,sizeof(buf)/sizeof(buf[0]),fp) == 0)
      return rgbe_error(rgbe_read_error,NULL);
  }
  for(;;) {
    if ((buf[0] == 0)||(buf[0] == '\n'))
      return rgbe_error(rgbe_format_error,"no FORMAT specifier found");
    else if (strcmp(buf,"FORMAT=32-bit_rle_rgbe\n") == 0)
      break;       /* format found so break out of loop */
    else if (info && (sscanf(buf,"GAMMA=%g",&tempf) == 1)) {
      info->gamma = tempf;
      info->valid |= RGBE_VALID_GAMMA;
    }
    else if (info && (sscanf(buf,"EXPOSURE=%g",&tempf) == 1)) {
      info->exposure = tempf;
      info->valid |= RGBE_VALID_EXPOSURE;
    }
    if (fgets(buf,sizeof(buf)/sizeof(buf[0]),fp) == 0)
      return rgbe_error(rgbe_read_error,NULL);
  }
  if (fgets(buf,sizeof(buf)/sizeof(buf[0]),fp) == 0)
    return rgbe_error(rgbe_read_error,NULL);
  if (strcmp(buf,"\n") != 0)
    return rgbe_error(rgbe_format_error,
		      "missing blank line after FORMAT specifier");
  if (fgets(buf,sizeof(buf)/sizeof(buf[0]),fp) == 0)
    return rgbe_error(rgbe_read_error,NULL);
  if (sscanf(buf,"-Y %d +X %d",height,width) < 2)
    return rgbe_error(rgbe_format_error,"missing image size specifier");
  return RGBE_RETURN_SUCCESS;
}

/* simple write routine that does not use run length encoding */
/* These routines can be made faster by allocating a larger buffer and
   fread-ing and fwrite-ing the data in larger chunks */
int RGBE_WritePixels(FILE *fp, float *data, int numpixels)
{
  unsigned char rgbe[4];

  while (numpixels-- > 0) {
    float2rgbe(rgbe,data[RGBE_DATA_RED],
	       data[RGBE_DATA_GREEN],data[RGBE_DATA_BLUE]);
    data += RGBE_DATA_SIZE;
    if (fwrite(rgbe, sizeof(rgbe), 1, fp) < 1)
      return rgbe_error(rgbe_write_error,NULL);
  }
  return RGBE_RETURN_SUCCESS;
}

/* simple read routine.  will not correctly handle run length encoding */
int RGBE_ReadPixels(FILE *fp, float *data, int numpixels)
{
  unsigned char rgbe[4];

  while(numpixels-- > 0) {
    if (fread(rgbe, sizeof(rgbe), 1, fp) < 1)
      return rgbe_error(rgbe_read_error,NULL);
    rgbe2float(&data[RGBE_DATA_RED],&data[RGBE_DATA_GREEN],
	       &data[RGBE_DATA_BLUE],rgbe);
    data += RGBE_DATA_SIZE;
  }
  return RGBE_RETURN_SUCCESS;
}

/* The code below is only needed for the run-length encoded files. */
/* Run length encoding adds considerable complexity but does */
/* save some space.  For each scanline, each channel (r,g,b,e) is */
/* encoded separately for better compression. */

int RGBE_WriteBytes_RLE(FILE *fp, unsigned char *data, int numbytes)
{
#define MINRUNLENGTH 4
  int cur, beg_run, run_count, old_run_count, nonrun_count;
  unsigned char buf[2];

  cur = 0;
  while(cur < numbytes) {
    beg_run = cur;
    /* find next run of length at least 4 if one exists */
    run_count = old_run_count = 0;
    while((run_count < MINRUNLENGTH) && (beg_run < numbytes)) {
      beg_run += run_count;
      old_run_count = run_count;
      run_count = 1;
      while((data[beg_run] == data[beg_run + run_count])
	    && (beg_run + run_count < numbytes) && (run_count < 127))
	run_count++;
    }
    /* if data before next big run is a short run then write it as such */
    if ((old_run_count > 1)&&(old_run_count == beg_run - cur)) {
      buf[0] = 128 + old_run_count;   /*write short run*/
      buf[1] = data[cur];
      if (fwrite(buf,sizeof(buf[0])*2,1,fp) < 1)
	return rgbe_error(rgbe_write_error,NULL);
      cur = beg_run;
    }
    /* write out bytes until we reach the start of the next run */
    while(cur < beg_run) {
      nonrun_count = beg_run - cur;
      if (nonrun_count > 128)
	nonrun_count = 128;
      buf[0] = nonrun_count;
      if (fwrite(buf,sizeof(buf[0]),1,fp) < 1)
	return rgbe_error(rgbe_write_error,NULL);
      if (fwrite(&data[cur],sizeof(data[0])*nonrun_count,1,fp) < 1)
	return rgbe_error(rgbe_write_error,NULL);
      cur += nonrun_count;
    }
    /* write out next run if one was found */
    if (run_count >= MINRUNLENGTH) {
      buf[0] = 128 + run_count;
      buf[1] = data[beg_run];
      if (fwrite(buf,sizeof(buf[0])*2,1,fp) < 1)
	return rgbe_error(rgbe_write_error,NULL);
      cur += run_count;
    }
  }
  return RGBE_RETURN_SUCCESS;
#undef MINRUNLENGTH
}

int RGBE_WritePixels_RLE(FILE *fp, float *data, int scanline_width,
			 int num_scanlines)
{
  unsigned char rgbe[4];
  unsigned char *buffer;
  int i, err;

  if ((scanline_width < 8)||(scanline_width > 0x7fff))
    /* run length encoding is not allowed so write flat*/
    return RGBE_WritePixels(fp,data,scanline_width*num_scanlines);
  buffer = (unsigned char *)malloc(sizeof(unsigned char)*4*scanline_width);
  if (buffer == NULL)
    /* no buffer space so write flat */
    return RGBE_WritePixels(fp,data,scanline_width*num_scanlines);
  while(num_scanlines-- > 0) {
    rgbe[0] = 2;
    rgbe[1] = 2;
    rgbe[2] = scanline_width >> 8;
    rgbe[3] = scanline_width & 0xFF;
    if (fwrite(rgbe, sizeof(rgbe), 1, fp) < 1) {
      free(buffer);
      return rgbe_error(rgbe_write_error,NULL);
    }
    for(i=0;i<scanline_width;i++) {
      float2rgbe(rgbe,data[RGBE_DATA_RED],
		 data[RGBE_DATA_GREEN],data[RGBE_DATA_BLUE]);
      buffer[i] = rgbe[0];
      buffer[i+scanline_width] = rgbe[1];
      buffer[i+2*scanline_width] = rgbe[2];
      buffer[i+3*scanline_width] = rgbe[3];
      data += RGBE_DATA_SIZE;
    }
    /* write out each of the four channels separately run length encoded */
    /* first red, then green, then blue, then exponent */
    for(i=0;i<4;i++) {
      if ((err = RGBE_WriteBytes_RLE(fp,&buffer[i*scanline_width],
				     scanline_width)) != RGBE_RETURN_SUCCESS) {
	free(buffer);
	return err;
      }
    }
  }
  free(buffer);
  return RGBE_RETURN_SUCCESS;
}

int RGBE_ReadPixels_RLE(FILE *fp, float *data, int scanline_width,
			int num_scanlines)
{
  unsigned char rgbe[4], *scanline_buffer, *ptr, *ptr_end;
  int i, count;
  unsigned char buf[2];

  if ((scanline_width < 8)||(scanline_width > 0x7fff))
    /* run length encoding is not allowed so read flat*/
    return RGBE_ReadPixels(fp,data,scanline_width*num_scanlines);
  scanline_buffer = NULL;
  /* read in each successive scanline */
  while(num_scanlines > 0) {
    if (fread(rgbe,sizeof(rgbe),1,fp) < 1) {
      free(scanline_buffer);
      return rgbe_error(rgbe_read_error,NULL);
    }
    if ((rgbe[0] != 2)||(rgbe[1] != 2)||(rgbe[2] & 0x80)) {
      /* this file is not run length encoded */
      rgbe2float(&data[0],&data[1],&data[2],rgbe);
      data += RGBE_DATA_SIZE;
      free(scanline_buffer);
      return RGBE_ReadPixels(fp,data,scanline_width*num_scanlines-1);
    }
    if ((((int)rgbe[2])<<8 | rgbe[3]) != scanline_width) {
      free(scanline_buffer);
      return rgbe_error(rgbe_format_error,"wrong scanline width");
    }
    if (scanline_buffer == NULL)
      scanline_buffer = (unsigned char *)
	malloc(sizeof(unsigned char)*4*scanline_width);
    if (scanline_buffer == NULL)
      return rgbe_error(rgbe_memory_error,"unable to allocate buffer space");

    ptr = &scanline_buffer[0];
    /* read each of the four channels for the scanline into the buffer */
    for(i=0;i<4;i++) {
      ptr_end = &scanline_buffer[(i+1)*scanline_width];
      while(ptr < ptr_end) {
	if (fread(buf,sizeof(buf[0])*2,1,fp) < 1) {
	  free(scanline_buffer);
	  return rgbe_error(rgbe_read_error,NULL);
	}
	if (buf[0] > 128) {
	  /* a run of the same value */
	  count = buf[0]-128;
	  if ((count == 0)||(count > ptr_end - ptr)) {
	    free(scanline_buffer);
	    return rgbe_error(rgbe_format_error,"bad scanline data");
	  }
	  while(count-- > 0)
	    *ptr++ = buf[1];
	}
	else {
	  /* a non-run */
	  count = buf[0];
	  if ((count == 0)||(count > ptr_end - ptr)) {
	    free(scanline_buffer);
	    return rgbe_error(rgbe_format_error,"bad scanline data");
	  }
	  *ptr++ = buf[1];
	  if (--count > 0) {
	    if (fread(ptr,sizeof(*ptr)*count,1,fp) < 1) {
	      free(scanline_buffer);
	      return rgbe_error(rgbe_read_error,NULL);
	    }
	    ptr += count;
	  }
	}
      }
    }
    /* now convert data from buffer into floats */
    for(i=0;i<scanline_width;i++) {
      rgbe[0] = scanline_buffer[i];
      rgbe[1] = scanline_buffer[i+scanline_width];
      rgbe[2] = scanline_buffer[i+2*scanline_width];
      rgbe[3] = scanline_buffer[i+3*scanline_width];
      rgbe2float(&data[RGBE_DATA_RED],&data[RGBE_DATA_GREEN],
		 &data[RGBE_DATA_BLUE],rgbe);
      data += RGBE_DATA_SIZE;
    }
    num_scanlines--;
  }
  free(scanline_buffer);
  return RGBE_RETURN_SUCCESS;
}


std::string type2str(int type) {
	std::string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}

std::string cvMat_description(const char* name, const cv::Mat& m) 
{
    std::stringstream sstr;
    double min, max;
    cv::minMaxLoc(m, &min, &max);
    sstr<<std::setw(20)<<name<<"\t - "<<m.size()<<"  "<<type2str(m.type())<<" ["<<min<<","<<max<<"]";
//    cv::imshow(name, m);
//    cv::waitKey(0);
    return sstr.str();
}

cv::Mat load_hdr(const char* filename)
{	
	if (!filename)
		throw "load_hdr() : Specified filename is (null).";
	
	std::FILE* f = std::fopen(filename,"rb");

	if (!f) 
	{
		std::fclose(f);
    		throw "load_hdr() : Could not open HDR file.";
	}

    	int width, height;
    	rgbe_header_info info;

    	if (RGBE_ReadHeader(f,&width,&height,&info)==RGBE_RETURN_FAILURE)
	{
		std::fclose(f);
    		throw "load_hdr() : HDR header error.";
	}

	cv::Mat sol(cv::Size(width,height),CV_32FC3);
	float* buffer = new float[3*width];

	for (int i=0; i<height; i++)
	{
		if (RGBE_ReadPixels_RLE(f,buffer,width,1)==RGBE_RETURN_FAILURE)
		{
			std::fclose(f);
	    		throw "load_hdr() : HDR reading error.";
		}

		int idx = 0;
		for (int j=0; j<width; j++,idx+=3) 
			for (int c=0;c<3;c++) {
				sol.at<float>(i,3*j+2-c)=buffer[idx+c];
			}
	}
	
	std::fclose(f);
	delete[] buffer;
	return sol;
}



//Dehaze image
cv::Mat tonemap(const cv::Mat& image, const cv::Mat& factor, double a, double lum_white)
{
    std::chrono::time_point<std::chrono::system_clock> start;
    start = std::chrono::system_clock::now();
    
    std::vector<cv::Mat> channels;
    cv::split(image, channels);

    cv::Mat luminance = 0.27*channels[0] + 0.67*channels[1] + 0.06*channels[2];
    
    double lmin, lmax;
    cv::minMaxLoc(luminance, &lmin, &lmax);
    double lw_ = lum_white*2.0/lmax;

    cv::Mat log_luminance;
    cv::log(luminance, log_luminance);
    cv::Mat positive = luminance > 0;
    double log_mean = std::exp(cv::mean(log_luminance, positive)[0]);
    
    cv::Mat al = (a/log_mean)*luminance;
    cv::Mat lw = lw_*factor;
    cv::Mat lw2;
    cv::multiply(lw, lw, lw2, 1, CV_32F);
    cv::Mat ld;
    cv::divide(al,lw2,ld,1,CV_32F);
    ld = (cv::Scalar(1.0,1.0,1.0) + ld)/(cv::Scalar(1.0,1.0,1.0) + al);
    cv::multiply(al,ld,ld,255.0,CV_32F);
    cv::multiply(luminance, lw, luminance,1,CV_32F);
    cv::Mat f;
    cv::divide(ld,luminance, f, 1, CV_32F);
    cv::Mat sol;
    cv::cvtColor(f, f, CV_GRAY2RGB);
    cv::multiply(f, image, sol, 1, CV_32FC3);
    cv::max(sol, cv::Scalar(0.0,0.0,0.0), sol);
    cv::min(sol, cv::Scalar(255.0,255.0,255.0), sol);
    sol.convertTo(sol, CV_8UC3);

    std::cerr<<"Tiempo tonemap : "<<std::chrono::duration<double>(std::chrono::system_clock::now() - start).count()<<std::endl;
    return sol;
}


/*int main(int argc, const char** argv) {
	const char* nimage   = "memorial.hdr";
	cv::String nlumwhite = "memorial_lumwhite.jpg";
	cv::String nresult   = "memorial_tonemapped.jpg";
	double a        = 0.18;
	double lumwhite = 1.0;

    for (int i = 1;i<argc;++i)
    {
    if (i<argc - 1) {
	if (strcmp("-input",argv[i])==0)               nimage    = argv[++i];
	else if (strcmp("-lumwhite-map",argv[i])==0)   nlumwhite = argv[++i];
	else if (strcmp("-output",argv[i])==0)         nresult   = argv[++i];
	else if (strcmp("-a",argv[i])==0)              a         = atof(argv[++i]);
	else if (strcmp("-lumwhite",argv[i])==0)       lumwhite  = atof(argv[++i]);
    }
    }
 

    cv::Mat image = load_hdr(nimage);
    
    cv::Mat lumwhite_map = cv::imread(nlumwhite,CV_LOAD_IMAGE_GRAYSCALE);
    lumwhite_map.convertTo(lumwhite_map,CV_32FC1);
    
    cv::Mat f1= tonemap(image, lumwhite_map, a, lumwhite);
   
    cv::imwrite(nresult, f1); 
}*/
