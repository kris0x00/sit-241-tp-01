#include <stdio.h>
#include <stdlib.h>
#include "kernel/kernel.h"
#include "ppm/ppm.h"

#define ARGC_MIN 3


/**
 * @brief Function to convert float to uint8_t
 */
uint8_t float_to_uint8_t(float v) {
	uint8_t r = 0;
	if(v < 0.0)
		r = 0;
	else if(v > 255.0)
		r = 255;
	else
		r = (uint8_t)v;
	return r;
}


/**
 * @brief Function to convert float to pixel_t struct type
 */
pixel_t float_to_pixel_t(float r, float g, float b) {
	pixel_t p = {0, 0, 0};
	p.r = float_to_uint8_t(r);
	p.g = float_to_uint8_t(g);
	p.b = float_to_uint8_t(b);
	return p;
}


/**
 * @brief Function used to apply a filter to a ppm image
 * @param kernel_t* The convolution kernel (struct type)
 * @param img_t* The image ppm format
 */
void convolve_pixel(img_t* img_src, img_t* img_dst, kernel_t* k, int x, int y) {
	int half_size = k->size / 2; // Can be optimized
	float r = 0.0;
	float g = 0.0;
	float b = 0.0;

	for(int ky = -half_size; ky <= half_size; ky++) {
		for(int kx = -half_size; kx <= half_size; kx++) {
			pixel_t src_px = get_pixel(img_src, x + kx, y + ky);
			int kernel_value = get_kernel_value(k, kx, ky);

			r += (kernel_value * src_px.r);
			g += (kernel_value * src_px.g);
			b += (kernel_value * src_px.b);
		}
	}
	set_pixel(img_dst, float_to_pixel_t(r, g, b), x, y);
}

/**
 * @brief Function used to apply a 2D convolution to a ppm image
 */
void convolve(img_t* img_src, img_t* img_dst, kernel_t* k) {
	for(int y = 0; y < img_src->height; y++)
		for(int x = 0; x < img_src->width; x++)
			convolve_pixel(img_src, img_dst, k, x, y);
}

void error_input_arg() {
	printf("Mauvais parametre d'entrée : ./conv input output -K -N\n");
	printf("input & output = file PPM\n");
	printf("-K = Kernel to use :\n\tidentity(3x3)\n\tsharpen(3x3)\n\tedge(3x3)\n\temboss(3x3)\n\tblur(3x3)\n\tgauss(5x5)\n\tunsharp(5x5)\n");
	printf("-N = number of thread (0= version séquentiel)\n");
}


int main(int argc, char **argv) {
	if(argc < ARGC_MIN) {
		error_input_arg();	
		return EXIT_FAILURE;
	}

	// Get the input and output filename
	char* img_input_filename = argv[1];
	char* img_output_filename = argv[2];
	char* kernel_select = argv[3];

	// Load the image from filename
	img_t* img = load_ppm(img_input_filename);
	img_t* img_dst = alloc_img(img->width, img->height);
	if (img == NULL) {
		fprintf(stderr, "Failed loading %s!\n", img_input_filename);
		return EXIT_FAILURE;
    }

    // Load the kernel
	kernel_t* k = malloc(sizeof(kernel_t));

	if (!set_select_kernel(k, kernel_select)) {
		error_input_arg();	
		return EXIT_FAILURE;		
	}

	print_kernel(k);

	// Apply the kernel
	convolve(img, img_dst, k);

	// Write the new image
	if (!write_ppm(img_output_filename, img_dst)) {
		fprintf(stderr, "Failed writing %s!\n", img_output_filename);
		free_img(img);
		return EXIT_FAILURE;
	}

	// Free the memory :)
	free_kernel(k);
	free_img(img);
	free_img(img_dst);

	return EXIT_SUCCESS;
}
