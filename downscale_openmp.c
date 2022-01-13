#include <stdio.h>
#include <string.h>
#include <omp.h>
#define it 20            // 20 iterations
#ifndef _TIMER_H_
#define _TIMER_H_

#include <sys/time.h>
#include <stdlib.h>
/* The argument now should be a double (not a pointer to a double) */
#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}

#endif

struct  PGMstructure
{
    int maxVal;          // Image maximum threshold
    int width;           // Used to record the height of the image
    int height;          // Used to record the width of the image
    int** data;          // Store original grey values array
    int** new_data;      // Store updated grey values array
};
typedef struct PGMstructure PGMImage; // Declare new type names and pointers to character variables

void getPGMFile(char fileName[],PGMImage* img);    // Read the pgm file
void blur(PGMImage* image,int thread_count);                        // Use filters to smooth the image
void save(PGMImage* image);                        // Save the pgm file

int main(int argc, char *argv[])
{
    double start, finish, elapsed;
    int thread_count = strtol(argv[1],NULL,10);

    PGMImage* im = malloc(sizeof(PGMImage));
    // Please change the path according to the specific location of your file
    getPGMFile("im.pgm",&(*im));              // Obtain pgm file
    GET_TIME(start);                          // Timing before blur()
    blur(im,thread_count);                                 // Perform mean filtering on the input image src
    GET_TIME(finish);                         // Timing after blur()
    save(im);                                 // Save the image
    elapsed = finish - start;                 // Runtime
    printf("elapsed time: %.6f\n", elapsed);
    return 0;
}

void getPGMFile(char fileName[],PGMImage* img){
    FILE* in_file;
    char version[100];
    int row,col,i;
    // Open a binary file, only allow reading and writing data
    in_file = fopen(fileName,"rb");

    /*File none error*/
    if (in_file == NULL)
    {
        fprintf(stderr, "Error: Unable to open file %s\n\n", fileName);
        exit(1);
    }
    printf("\nReading image file: %s\n", fileName);



    /*Read image information*/
    fscanf(in_file,"%s",version);
    fscanf(in_file,"%d",&(img->width));
    fscanf(in_file,"%d",&(img->height));
    fscanf(in_file,"%d",&(img->maxVal));

    /*Show image information*/
    printf("\n%s", version);
    printf("\n width  = %d",img->width);
    printf("\n height = %d",img->height);
    printf("\n maxVal = %d",img->maxVal);
    printf("\n");


    /*Allocate dynamic array memory*/
    img->data = (int**) malloc(sizeof(int*)* img->height);
    img->new_data = (int**) malloc(sizeof(int*) * img->height);
    for (i = 0; i < img->height; i++) {
        img->data[i] = (int*) malloc(sizeof (int)* img->width);
        img->new_data[i]=(int*) malloc(sizeof(int)* img->width);
    }

    /*Read image data*/
    for (row = 0; row < img-> height ; row++) {
        for (col = 0; col < img->width; col++) {
            fscanf(in_file,"%d",&(img->data[row][col]));
        }
    }

    fclose(in_file);
    printf("\nDone read file information and data\n");

}

// Blur the picture
void blur(PGMImage* image,int thread_count){
    int row,col;
    // Define temporary variables to store processed data
    int temp;
    int k;
    int hi = image->height;
    int wd = image->width;
    // Repeat the blurring process 20 times
    for (k = 0; k < it; k++) {
        // Image boundary processing
        // update four corners of the image
        image->new_data[0][0] = (int) (image->data[1][0]+image->data[0][1]+image->data[1][1])/3;
        image->new_data[0][wd-1]=(int) (image->data[0][wd-2]+image->data[1][wd-1]+image->data[1][wd-2])/3;
        image->new_data[hi-1][0] = (int)(image->data[hi-1][1]+image->data[hi-2][0]+image->data[hi-2][1])/3;
        image->new_data[hi-1][wd-1]=(int)(image->data[hi-1][wd-2]+image->data[hi-2][wd-1]+image->data[hi-2][wd-2])/3;
#pragma omp parallel for num_threads(thread_count) schedule(dynamic) private(col)
        for (row = 1; row < hi-1; ++row) {
                for (col = 1; col < wd-1 ; ++col) {
                    image->new_data[row][col] =
                            (int)(image->data[row - 1][col - 1] + image->data[row - 1][col] + image->data[row - 1][col + 1] +
                             image->data[row][col - 1] + image->data[row][col + 1] + image->data[row + 1][col - 1] +
                             image->data[row + 1][col] + image->data[row + 1][col + 1])/8;
                }
        }
#pragma omp parallel for num_threads(thread_count) schedule(dynamic)
        for(col = 1; col < wd-1; ++col){
            image->new_data[0][col]=(int)(image->data[0][col-1]+
                                                         image->data[0][col+1]+
                                                         image->data[1][col-1]+
                                                         image->data[1][col]+
                                                         image->data[1][col+1])/8;
        }
#pragma omp parallel for num_threads(thread_count) schedule(dynamic)
        for(col = 1; col < wd-1; ++col){
                        image->new_data[hi-1][col]=(int)(image->data[hi-2][col-1] +
                                                   image->data[hi-2][col] +
                                                   image->data[hi-2][col+1] +
                                                   image->data[hi-1][col-1] +
                                                   image->data[hi-1][col+1])/5;
        }
#pragma omp parallel for num_threads(thread_count) schedule(dynamic)
        for(row = 1; row < hi-1; ++row){
            image->new_data[row][0]=(int)(image->data[row-1][0]+
                                                          image->data[row+1][0]+
                                                          image->data[row-1][1]+
                                                          image->data[row][1]+
                                                          image->data[row+1][1])/5;
        }
#pragma omp parallel for num_threads(thread_count) schedule(dynamic)
    for(row = 1; row < hi-1; ++row){
        image->new_data[row][wd-1]=(int)(image->data[row-1][wd-1]+
                                   image->data[row+1][wd-1]+
                                   image->data[row-1][wd-2]+
                                   image->data[row][wd-2]+
                                   image->data[row+1][wd-2])/5;
        }

#pragma omp parallel for num_threads(thread_count) schedule(dynamic)
        for (row = 1; row < hi-1; ++row) {
            memcpy(image->data[row],image->new_data[row],sizeof(int)*wd);
        }
    }
}

// Save the blurred image
void save(PGMImage* image){
    FILE* ou_File;
    int row, col;
    int hi = image->height;
    int wd = image->width;
    int maxV = image->maxVal;

    // Open and name the file
    // Please change the path according to the specific location of your file
    ou_File = fopen("im-blur.pgm", "w+");

    // Print header information of pgm file
    fprintf(ou_File,"P2\n");
    fprintf(ou_File,"%d %d\n",wd,hi);
    fprintf(ou_File,"%d \n",maxV);
    // Write blurred image data
    for (row = 0; row < hi; row++) {
        for (col = 0; col < wd; col++) {
            fprintf(ou_File,"%d ",image->new_data[row][col]);
        }
        fprintf(ou_File,"\n");
    }
    // Close the pgm file
    fclose(ou_File);
}