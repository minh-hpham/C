#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * Please fill in the following student struct 
 */
student_t student = {
  "Minh Pham",     /* Full name */
  "u0975798@utah.edu",  /* Email address */

};

/******************************************************
 * PINWHEEL KERNEL
 *
 * Your different versions of the pinwheel kernel go here
 ******************************************************/

/* 
 * naive_pinwheel - The naive baseline version of pinwheel 
 */
char naive_pinwheel_descr[] = "naive_pinwheel: baseline implementation";
void naive_pinwheel(pixel *src, pixel *dest)
{
  /* Loop over 4 quadrants: */
  int  i, j,ii, jj,temp, s_idx,d_idx,qjd,qid;

  int block = 4;
  int size = src->dim;
  int h_dim =  size/2;
 
  for (qid = 0; qid <= h_dim; qid += h_dim)
    for (qjd = 0; qjd <= h_dim; qjd += h_dim)
 
	for (i = 0; i < h_dim; i += block)
	  for (j = 0 ; j < h_dim; j += block)

	    for (ii = i; ii < i + block; ii++) 
	      for (jj = j; jj < j + block; jj++) {
	      
	      s_idx = RIDX(qjd + ii,jj + qid,size);
	      d_idx = RIDX(qjd + h_dim - 1 - jj, ii + qid,size);

	      temp = src[s_idx].red;
	      temp += src[s_idx].green;
	      temp += src[s_idx].blue;
	      temp /= 3;
	      
	      dest[d_idx].red = temp; 
	      dest[d_idx].green = temp;
	      dest[d_idx].blue = temp;
	    }
}



/* 
 * pinwheel - Your current working version of pinwheel
 * IMPORTANT: This is the version you will be graded on
 */
char pinwheel_descr[] = "pinwheel: Current working version";
void pinwheel(pixel *src, pixel *dest)
{
  naive_pinwheel(src, dest);
}

/*********************************************************************
 * register_pinwheel_functions - Register all of your different versions
 *     of the pinwheel kernel with the driver by calling the
 *     add_pinwheel_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_pinwheel_functions() {
  add_pinwheel_function(&pinwheel, pinwheel_descr);
  add_pinwheel_function(&naive_pinwheel, naive_pinwheel_descr);
}


/***************************************************************
 * MOTION KERNEL
 * 
 * Starts with various typedefs and helper functions for the motion
 * function, and you may modify these any way you like.
 **************************************************************/

/* A struct used to compute averaged pixel value */
typedef struct {
  int red;
  int green;
  int blue;
} pixel_sum;

/* 
 * initialize_pixel_sum - Initializes all fields of sum to 0 
 */
static void initialize_pixel_sum(pixel_sum *sum) 
{
  sum->red = sum->green = sum->blue = 0;
}

/* 
 * accumulate_sum - Accumulates field values of p in corresponding 
 * fields of sum 
 */
static void accumulate_weighted_sum(pixel_sum *sum, pixel p, int weight) 
{
  sum->red +=  p.red * weight/100;
  sum->green +=  p.green * weight/100;
  sum->blue +=  p.blue * weight/100;
}

/* 
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel 
 */
static void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum) 
{
  current_pixel->red = (unsigned short) sum.red;
  current_pixel->green = (unsigned short) sum.green;
  current_pixel->blue = (unsigned short) sum.blue;
}

/* 
 * weighted_combo - Returns new pixel value at (i,j) 
 */
static pixel weighted_combo(int dim, int i, int j, pixel *src) 
{
  int ii,jj;
  pixel_sum sum;
  pixel current_pixel;
  int weights[3][3] = { { 60, 3, 0 },
			{ 3, 30, 3 },
			{ 0, 3, 10 } };
  initialize_pixel_sum(&sum);
  for (ii=0; ii < 3; ii++)
    for (jj=0; jj < 3; jj++)
	accumulate_weighted_sum(&sum,
				src[RIDX(i+ii,j+jj,dim)],
				weights[ii][jj]);
  
  assign_sum_to_pixel(&current_pixel, sum);

  return current_pixel;
}


/******************************************************
 * Your different versions of the motion kernel go here
 ******************************************************/

/*
 * naive_motion - The naive baseline version of motion 
 */
char naive_motion_descr[] = "naive_motion: baseline implementation";
void naive_motion(pixel *src, pixel *dst) 
{
  int i, j;
  int size = src->dim;

  pixel_sum sum;
  pixel current_pixel;
  int weights[3][3] = { { 60, 3, 0 },
			{ 3, 30, 3 },
			{ 0, 3, 10 } };

  // have enough 9 cells
  for (i = 0; i < size-2; i++) {
    for (j = 0; j < size-2; j++) 
      dst[RIDX(i, j, size)] = weighted_combo(size, i, j, src);

    // rightmost and second-to-last rightmost
    initialize_pixel_sum(&sum);
    j = size - 2;

    accumulate_weighted_sum(&sum, src[RIDX(i,j,size)], weights[0][0]);
    accumulate_weighted_sum(&sum, src[RIDX(i,j+1,size)], weights[0][1]);
    accumulate_weighted_sum(&sum, src[RIDX(i+1,j,size)], weights[1][0]);
    accumulate_weighted_sum(&sum, src[RIDX(i+1,j+1,size)], weights[1][1]);
    accumulate_weighted_sum(&sum, src[RIDX(i+2,j,size)], weights[2][0]);
    accumulate_weighted_sum(&sum, src[RIDX(i+2,j+1,size)], weights[2][1]);
    assign_sum_to_pixel(&current_pixel, sum);
    dst[RIDX(i, j, size)] = current_pixel;

    initialize_pixel_sum(&sum);
    j = size - 1;
    accumulate_weighted_sum(&sum, src[RIDX(i,j,size)], weights[0][0]);
    accumulate_weighted_sum(&sum, src[RIDX(i+1,j,size)], weights[1][0]);
    accumulate_weighted_sum(&sum, src[RIDX(i+2,j,size)], weights[2][0]);
    assign_sum_to_pixel(&current_pixel, sum);
    dst[RIDX(i, j, size)] = current_pixel;   
  }

  // second to last row
  i = size - 2;
  for (j = 0; j < size-2; j++) {
    initialize_pixel_sum(&sum);
    accumulate_weighted_sum(&sum, src[RIDX(i,j,size)], weights[0][0]);
    accumulate_weighted_sum(&sum, src[RIDX(i,j+1,size)], weights[0][1]);
    accumulate_weighted_sum(&sum, src[RIDX(i,j+2,size)], weights[0][2]);
    accumulate_weighted_sum(&sum, src[RIDX(i+1,j,size)], weights[1][0]);
    accumulate_weighted_sum(&sum, src[RIDX(i+1,j+1,size)], weights[1][1]);
    accumulate_weighted_sum(&sum, src[RIDX(i+1,j+2,size)], weights[1][2]);
    assign_sum_to_pixel(&current_pixel, sum);
    dst[RIDX(i, j, size)] = current_pixel;
  }
  // cell [size-2,size-2]
  initialize_pixel_sum(&sum);
  j = size - 2;
  accumulate_weighted_sum(&sum, src[RIDX(i,j,size)], weights[0][0]);
  accumulate_weighted_sum(&sum, src[RIDX(i,j+1,size)], weights[0][1]);
  accumulate_weighted_sum(&sum, src[RIDX(i+1,j,size)], weights[1][0]);
  accumulate_weighted_sum(&sum, src[RIDX(i+1,j+1,size)], weights[1][1]);
  assign_sum_to_pixel(&current_pixel, sum);
  dst[RIDX(i, j, size)] = current_pixel;

  // cell [size-2, size -1]
  initialize_pixel_sum(&sum);
  j = size - 1;
  accumulate_weighted_sum(&sum, src[RIDX(i,j,size)], weights[0][0]);
  accumulate_weighted_sum(&sum, src[RIDX(i+1,j,size)], weights[1][0]);
  assign_sum_to_pixel(&current_pixel, sum);
  dst[RIDX(i, j, size)] = current_pixel;

  // bottom row
  i = size - 1;
  for(j = 0; j < size-2; j++){
    initialize_pixel_sum(&sum);

    accumulate_weighted_sum(&sum, src[RIDX(i,j,size)], weights[0][0]);
    accumulate_weighted_sum(&sum, src[RIDX(i,j+1,size)], weights[0][1]);
    accumulate_weighted_sum(&sum, src[RIDX(i,j+2,size)], weights[0][2]);
    assign_sum_to_pixel(&current_pixel, sum);
    dst[RIDX(i, j, size)] = current_pixel;
  }

  // cell [size-1,size-2]
  initialize_pixel_sum(&sum);
  j = size - 2;
  accumulate_weighted_sum(&sum, src[RIDX(i,j,size)], weights[0][0]);
  accumulate_weighted_sum(&sum, src[RIDX(i,j+1,size)], weights[0][1]);
  assign_sum_to_pixel(&current_pixel, sum);
  dst[RIDX(i, j, size)] = current_pixel;

  // cell [size-1,size-1]
  initialize_pixel_sum(&sum);
  j = size - 1;
  accumulate_weighted_sum(&sum, src[RIDX(i,j,size)], weights[0][0]);
  assign_sum_to_pixel(&current_pixel, sum);
  dst[RIDX(i, j, size)] = current_pixel;

}

/*
 * motion - Your current working version of motion. 
 * IMPORTANT: This is the version you will be graded on
 */
char motion_descr[] = "motion: Current working version";
void motion(pixel *src, pixel *dst) 
{
  naive_motion(src, dst);
}

/********************************************************************* 
 * register_motion_functions - Register all of your different versions
 *     of the motion kernel with the driver by calling the
 *     add_motion_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_motion_functions() {
  add_motion_function(&motion, motion_descr);
  add_motion_function(&naive_motion, naive_motion_descr);
}
