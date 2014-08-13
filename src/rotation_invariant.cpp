#include "mex.h"
#define cimg_plugin 1
//#define cimg_use_png
#include "CImg.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>


using namespace cimg_library;
using namespace std;

void denoise(CImg<float> img, float patch_size, float samrpling, float sigma,
		float lamda, float alpha);

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	CImg<float> img3("lena.jpg");
//	CImgDisplay main_disp(image, "My noisy image");

//	CImg<float> img1(100, 100), img2(img1), img3(img1); // Declare three 100x100 scalar images with unitialized pixel values.
//	img1 = "0,50,100,150,200,250,200,150,100,50"; // Set pixel values of 'img1' from a value sequence.
//	img2 = "10*((x*y)%25)"; // Set pixel values of 'img2' from a formula.
//	img3 = "map.png"; // Set pixel values of 'img3' from a file (image size is modified).
//	(img3).display();
	CImg<float> res = img3.get_noise(10);
	CImg<float> dest = res.get_nlmeans();
	dest.display();
	denoise(res, 3, 3, 3, 3, 3);
//	CImg<float> out = nlmeans2(res);
//	(img3, img3.get_sqr().normalize(0, 255), res).display();

//	float sum = 0;
//	for (CImg<float>::iterator it = image.begin(); it < image.end(); ++it) {
//		sum += *it; // Compute sum of all pixel values, through a CImg iterator.
//		mexPrintf("%f\n",*it);
//	}
//	const float sum2 = image.sum(); // Do the same with a built-in method.

//	while (!main_disp.is_closed()) {
//		main_disp.wait();
//	}
}

void denoise(CImg<float> img, float patch_size, float sampling, float sigma,
		float lamda, float alpha) {

	if (!img.is_empty()) {
		float height = img.height();
		float width = img.width();
		float halfPatch = 2;
		float halfWindow = 17;
		sigma = 30;
		float h = 0.4*sigma;
		float H = h*h;
		float error = 0.5;
		float div = 3*pow(2*halfPatch+1,2);

		CImg<float> temp(2 * halfPatch + 1, 2 * halfPatch + 1, 1, 3);
		CImg<float> temp2(2 * halfPatch + 1, 2 * halfPatch + 1, 1, 3);
		CImg<float> diff(2 * halfPatch + 1, 2 * halfPatch + 1, 1, 3);
		CImg<float> dest(img.width(), img.height());
		img.display();
		int counter = 0;
		float borderWidth = halfPatch;
		for (int i = halfPatch; i < img.height() - halfPatch - 1; i++) {
			for (int j = halfPatch; j < img.width() - halfPatch - 1; j++) {
				float wmax = -1;
				CImg<float> summ(2 * halfPatch + 1, 2 * halfPatch + 1, 1, 3);
				for (int c = 0; c < 3; c++) {
					for (int yi = i - halfPatch; yi < i + halfPatch + 1; yi++) {
						for (int xj = j - halfPatch; xj < j + halfPatch + 1; xj++) {
							float *tmp = img.data(xj, yi, 0, c);
							float *cur = temp.data(xj-j+halfPatch, yi-i+halfPatch, 0, c);
							*cur = *tmp;
						}
					}
//					*r = 0;
				}

				int ymin = cimg::max(halfPatch, (int) (i - halfWindow));
				int ymax = cimg::min(img.height() - halfPatch - 1, (int) (i + halfWindow + 1));
				int xmin = cimg::max(halfPatch, (int) (j - halfWindow));
				int xmax = cimg::min(img.width() - halfPatch - 1, (int) (j + halfWindow + 1));
				CImg<float> window(xmax-xmin, ymax-ymin, 1, 2);
				float window_sum = 0;
				for (int y = ymin; y < ymax; y++) {
					for (int x = xmin; x < xmax; x++) {
						float patch_sum = 0;
						if(x == j && y == i) continue;
						for (int c = 0; c < 3; c++) {
							for (int yi = y - halfPatch; yi < y + halfPatch + 1; yi++) {
								for (int xj = x - halfPatch;xj < x + halfPatch + 1; xj++) {
									float *cur = img.data(xj, yi, 0, c);
									float *v = temp2.data(xj-x+halfPatch, yi-y+halfPatch, 0, c);
									float *u = temp.data(xj-x+halfPatch, yi-y+halfPatch, 0, c);
									*v = *cur;
									float *d = diff.data(xj-x+halfPatch, yi-y+halfPatch, 0, c);
									*d = *u-*v;
									*d = (*d*(*d));
									patch_sum = patch_sum + (*d);
								}
							}
						}
						float *weight = window.data(x-xmin,y-ymin,0,0);
						patch_sum = patch_sum/div;
						patch_sum = patch_sum - 2*sigma*sigma;
						patch_sum = std::max(patch_sum,0.0f);
						patch_sum = -patch_sum/H;
//						mexPrintf("pow = %f\t",patch_sum);
						float mexp = std::exp(patch_sum);
//						mexPrintf("exp = %f\t",mexp);
						*weight = mexp;
						 wmax = *weight>wmax?*weight:wmax;
//						mexPrintf("prev = %f\t",window_sum);
						window_sum = window_sum + mexp;
//						mexPrintf("sum = %f\n",window_sum);
					}
				}
//				mexPrintf("x = %f\t",j-xmin);
//				mexPrintf("y = %f\n",i-ymin);
//				if((ymax - ymin <= 0) || (xmax-xmin<=0))
//					continue;
//				float *myw = window.data(j-xmin,i-ymin,0,0);
//				*myw = wmax;
//				window_sum += wmax;

				float *r = dest.data(j, i, 0, 0);
				float *g = dest.data(j, i, 0, 1);
				float *b = dest.data(j, i, 0, 2);
				*r = 0;
				*g = 0;
				*b = 0;
				float sumpx = 0;
				for (int y = ymin; y < ymax; y++) {
					for (int x = xmin; x < xmax; x++) {
						if(x == j && y == i) continue;
						for (int c = 0; c < 3; c++) {
							float *weight = window.data(x-xmin,y-ymin,0,0);
							float *cur = img.data(x, y, 0, c);
							float *pixel = dest.data(j, i, 0, c);
//							mexPrintf("window_sum = %f\n",(window_sum));
//							mexPrintf("weight = %f\n",(*weight));
							*pixel = (*pixel) + (*weight*(*cur)/window_sum);
							sumpx += *weight;
						}
					}
				}
//				mexPrintf("R = %f;\t",*r);
//				mexPrintf("G = %f;\t",*g);
//				mexPrintf("B = %f;\n",*b);
//				mexPrintf("weight summ = %f\n",sumpx/window_sum/3);
			}
		}
		(img,img-dest,dest).display();
	}
	label:
	return;
}
