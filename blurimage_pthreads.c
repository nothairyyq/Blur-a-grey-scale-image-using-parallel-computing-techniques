#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

pthread_barrier_t barr;
//Length, width, and bits of the image
int height,width,depth;
//Read in the image
int **img;
int **img_blur;
//Iterate bluring algorithm 20 times
int it=20;
int nth = 1;

//Create image memory, enter the size of the image
int **mallocImg(int height,int width)
{
    int ** img = (int **)malloc(sizeof(int*)*height);
    for(int i=0;i<height;i++)
    {
        img[i] = (int*) malloc(sizeof(int)*width);
        for(int j=0;j<width;j++)
        {
            img[i][j]=0;
        }
    }
    return img;
}

//Release image memory, input image height
void freeImg(int **img,int height)
{
    for(int i=0;i<height;i++)
    {
        free(img[i]);
    }
    free(img);
}

//Reads an image from a file, returns the size of the image
int** readPmg(char *filename,int* height,int *width,int *depth)
{
    //Open a PGM image file
    FILE* fp=fopen(filename,"r");
    char temp[1024];
    fgets(temp,1024,fp);
    //Read the size of image
    fscanf(fp,"%d %d\n",width,height);
    //Read the bites of the image
    fscanf(fp,"%d \n",depth);
    //Create image memory
    int **img = mallocImg(*height,*width);
    //Read in image data
    for(int i=0;i<*height;i++)
    {
        for(int j=0;j<*width;j++)
        {
            fscanf(fp,"%d ",&img[i][j]);
        }
        fscanf(fp,"\n");
    }
    return img;
}   

//Output image to file, need to specify the image size and bites
void writePmg(char *filename,int** img,int height,int width,int depth)
{
    //Open a file
    FILE* fp=fopen(filename,"w");
    fprintf(fp,"P2\n");
    //Output size
    fprintf(fp,"%d %d\n",height,width);
    //Output the bites of the image
    fprintf(fp,"%d\n",depth);
    //Output image data
    for(int i=0;i<height;i++)
    {
        for(int j=0;j<width;j++)
        {
            fprintf(fp,"%d ",img[i][j]);
        }
        fprintf(fp,"\n");
    }
}  

void *threadFun(void *param)  
{  
    //convert void* pararm to tid(int)
    long tid = (long)param;

    for(int t=0;t<it;t++)
    {
        for(int i=1+tid;i<height-1;i+=nth)
        {
            for(int j=1;j<width-1;j++)
            {
                img_blur[i][j]=0;
                for(int x=-1;x<=1;x++)
                {
                    for(int y=-1;y<=1;y++)
                    {
                        img_blur[i][j]+=img[i+x][j+y];
                    }
                }
                img_blur[i][j]-=img[i][j];
                img_blur[i][j]/=8;
            }
        }

        for(int i=1+tid;i<height-1;i+=nth)
        {
            for(int j=1;j<width-1;j++)
            {
                img[i][j]=img_blur[i][j];
            }
        }
        //set a barrier to wait all threads get here
        pthread_barrier_wait(&barr);
    }

}  

void run(int num_threads,double *timerecord)
{
    nth = num_threads;

    //Timing variables
    clock_t  start,end;
    //Start timing to read in images
    start = clock();

    img = readPmg("im.pgm",&height,&width,&depth);
    //End the timer to read the image
    end = clock();

    timerecord[0]=1.0*(end-start)/CLOCKS_PER_SEC;
    printf("time for reading image :%f s\n",1.0*(end-start)/CLOCKS_PER_SEC);

    img_blur = mallocImg(height,width);


    //init barrier and set barrier count to nth
    pthread_barrier_init(&barr,NULL,nth);

    // malloc handles for nth threads
    pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t)*nth);

    //Start timing iteration
    start = clock();
    
    //fork threads to computing
    for(int i=0;i<nth;i++)
    {
        pthread_create(&threads[i],NULL,threadFun,(void*)i);
    }

    //join threads
    for(int i=0;i<nth;i++)
    {
        pthread_join(threads[i],NULL);
    }

    //End timing iteration
    end = clock();
    printf("time for bluring :%f s\n",1.0*(end-start)/CLOCKS_PER_SEC/num_threads);
    timerecord[1]=1.0*(end-start)/CLOCKS_PER_SEC;

    //Start timing output
    start = clock();
    //Output image to file
    writePmg("im_blur.pgm",img,height,width,depth);
    //End timing output
    end = clock();
    printf("time for writing image :%f s\n",1.0*(end-start)/CLOCKS_PER_SEC);
    timerecord[2]=1.0*(end-start)/CLOCKS_PER_SEC;

    //Free image memory
    freeImg(img,height);
    freeImg(img_blur,height);
    free(threads);
}

// run times
#define TESTTIME 4 

int main(int argc,char ** argv)
{

    int tryThreadsNum[TESTTIME]={1,2,4,8};

    //get a large recorder
    double timeRecords[TESTTIME][3];

    for(int i=0;i<TESTTIME;i++)
    {
        printf("Use threads num: %d\n",tryThreadsNum[i]);
        run(tryThreadsNum[i],&timeRecords[i][0]);
        printf("\n");
    }

    double acc[TESTTIME];
    double eff[TESTTIME];

    acc[0]=1;
    eff[0]=1;
    for(int i=1;i<TESTTIME;i++)
    {
        acc[i]=timeRecords[0][1]/timeRecords[i][1];
        eff[i]=1/acc[i];
    }

    //summary
    for(int i=0;i<TESTTIME;i++)
    {
        printf("Number of Threads is : %d, Acceleration:%f, Effiency : %f\n",tryThreadsNum[i],acc[i],eff[i]);
    }
    return 0;
}
