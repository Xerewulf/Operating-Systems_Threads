/**
 *
 * CENG305 Project-2
 *
 * A Simple Memory Management Unit for a Multithreaded Program.
 * This program simulates a contiguous memory management unit.
 *
 * Usage:  mmu <size_of_memory> <file_name> <alloc_strategy>
 *
 * @group_id 00
 * @author  your names
 *
 * @version 1.0, 23/12/21
 */


#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
int threadnum = 0;
int *MEMORY;
int MEMORY_size;
pthread_mutex_t lock;
int method;

int remaining_frame;
int size_of_internal_fragmantation = 0;
int num_of_rejected_external_fragmantation = 0;
int num_of_insufficient = 0;


int allocated_memory[100][2];



void allocate_memory(int start, int end)
{
    for(int i = start; i <= end; i++)
    {
        MEMORY[i] = 1;
    }
    remaining_frame -= (end - start + 1);
    printf("-> %d frames will be used, remaining #frames: %d", (end - start + 1), remaining_frame) ;

}

void deallocate_memory(int pid)
{
    int start = allocated_memory[pid-1][0];
    int end = allocated_memory[pid-1][1];

    for(int i = allocated_memory[pid-1][0]; i <= allocated_memory[pid-1][1]; i++)
    {
        MEMORY[i] = 0;
    }

    remaining_frame += (end - start + 1);
    printf("	-> %d frames are deallocated, available #frames: %d", (end - start + 1), remaining_frame) ;
}

void calculateHoles()
{
    int start, end;
    bool isStart = false;

    printf("\nHoles: \n");

    for(int i = 0; i < MEMORY_size; i++)
    {
        if(MEMORY[i] == 0 && isStart == false)
        {
            start = i;
            isStart = true;
        }

        else if(MEMORY[i] == 1 && isStart == true)
        {
            end = i-1;
            isStart = false;

            printf("%d %d\n", start, (end - start + 1) * 4);

        }

        else if(i == MEMORY_size - 1 && MEMORY[i] == 0 && isStart == true)
        {
            end = i;
            isStart = false;

            printf("%d %d\n", start, (end - start + 1) * 4);

        }
    }
}


void firstFitAllocate(int pid, int frame_size_of_p)
{
    int start, end;
    bool isStart = false;
    bool isAllocated = false;
    int total_holes = 0;


    for(int i = 0; i < MEMORY_size && isAllocated == false; i++)
    {
        if(MEMORY[i] == 0 && isStart == false)
        {
            start = i;
            isStart = true;
        }

        else if(MEMORY[i] == 1 && isStart == true)
        {
            end = i-1;
            isStart = false;
            total_holes += (end - start + 1);

            if(frame_size_of_p <= (end - start + 1))
            {
                allocate_memory(start, start + frame_size_of_p - 1);
                allocated_memory[pid-1][0] = start;
                allocated_memory[pid-1][1] = start + frame_size_of_p - 1;
                isAllocated =  true;
            }


        }

        else if(i == MEMORY_size - 1 && MEMORY[i] == 0 && isStart == true)
        {
            end = i;
            isStart = false;
            total_holes += (end - start + 1);

            if(frame_size_of_p <= (end - start + 1))
            {
                allocate_memory(start, start + frame_size_of_p - 1);
                allocated_memory[pid-1][0] = start;
                allocated_memory[pid-1][1] = start + frame_size_of_p - 1;
                isAllocated =  true;
            }

        }

    }

    if(isAllocated == false && frame_size_of_p > total_holes)
    {
        printf("-> ERROR! Insufficient memory");
        num_of_insufficient++;
    }

    else if(isAllocated == false && frame_size_of_p <= total_holes)
    {
        printf("-> %d frames will be used, ERROR! External fragmentation", frame_size_of_p);
        num_of_rejected_external_fragmantation++;
    }

}

void bestFitAllocate(int pid, int frame_size_of_p)
{
    int start, end;
    bool isStart = false;
    int total_holes = 0;
    int startOfMinHole = 0, endOfMinfHole = 0;


    for(int i = 0; i < MEMORY_size; i++)
    {
        if(MEMORY[i] == 0 && isStart == false)
        {
            start = i;
            isStart = true;
        }

        else if(MEMORY[i] == 1 && isStart == true)
        {
            end = i-1;
            isStart = false;
            total_holes += (end - start + 1);

            if(endOfMinfHole - startOfMinHole == 0)
            {
                startOfMinHole = start;
                endOfMinfHole = end;
            }

            else if((end - start) < endOfMinfHole - startOfMinHole)
            {
                startOfMinHole = start;
                endOfMinfHole = end;
            }


        }

        else if(i == MEMORY_size - 1 && MEMORY[i] == 0 && isStart == true)
        {
            end = i;
            isStart = false;
            total_holes += (end - start + 1);

            if(endOfMinfHole - startOfMinHole == 0)
            {
                startOfMinHole = start;
                endOfMinfHole = end;
            }

            else if((end - start) < endOfMinfHole - startOfMinHole)
            {
                startOfMinHole = start;
                endOfMinfHole = end;
            }

        }


    }

    if(frame_size_of_p <= (endOfMinfHole - startOfMinHole + 1))
    {
        allocate_memory(startOfMinHole, startOfMinHole + frame_size_of_p - 1);
        allocated_memory[pid-1][0] = startOfMinHole;
        allocated_memory[pid-1][1] = startOfMinHole + frame_size_of_p - 1;
    }

    else if(frame_size_of_p > total_holes)
    {
        printf("-> ERROR! Insufficient memory");
        num_of_insufficient++;
    }

    else if(frame_size_of_p <= total_holes)
    {
        printf("-> %d frames will be used, ERROR! External fragmentation", frame_size_of_p);
        num_of_rejected_external_fragmantation;
    }

}


void worstFitAllocate(int pid, int frame_size_of_p)
{
    int start, end;
    bool isStart = false;
    int total_holes = 0;
    int startOfMaxHole = 0, endOfMaxfHole = 0;

    for(int i = 0; i < MEMORY_size; i++)
    {
        if(MEMORY[i] == 0 && isStart == false)
        {
            start = i;
            isStart = true;
        }

        else if(MEMORY[i] == 1 && isStart == true)
        {
            end = i-1;
            isStart = false;
            total_holes += (end - start + 1);

            if(endOfMaxfHole - startOfMaxHole == 0)
            {
                startOfMaxHole = start;
                endOfMaxfHole = end;
            }

            else if((end - start) > endOfMaxfHole - startOfMaxHole)
            {
                startOfMaxHole = start;
                endOfMaxfHole = end;
            }


        }

        else if(i == MEMORY_size - 1 && MEMORY[i] == 0 && isStart == true)
        {
            end = i;
            isStart = false;
            total_holes += (end - start + 1);

            if(endOfMaxfHole - startOfMaxHole == 0)
            {
                startOfMaxHole = start;
                endOfMaxfHole = end;
            }

            else if((end - start) > endOfMaxfHole - startOfMaxHole)
            {
                startOfMaxHole = start;
                endOfMaxfHole = end;
            }

        }


    }

    if(frame_size_of_p <= (endOfMaxfHole - startOfMaxHole + 1))
    {
        allocate_memory(startOfMaxHole, startOfMaxHole + frame_size_of_p - 1);
        allocated_memory[pid-1][0] = startOfMaxHole;
        allocated_memory[pid-1][1] = startOfMaxHole + frame_size_of_p - 1;
    }

    else if(frame_size_of_p > total_holes)
    {
        printf("-> ERROR! Insufficient memory");
        num_of_insufficient++;
    }

    else if(frame_size_of_p <= total_holes)
    {
        printf("%d -> d frames will be used, ERROR! External fragmentation", frame_size_of_p);
        num_of_rejected_external_fragmantation++;
    }
}

void print_memory_condition()
{
    printf("\n");
    printf("Total free memory in holes: %d frames, %d KB\n", remaining_frame, remaining_frame * 4);
    printf("Total memory wasted as an internal fragmentation: %d KB\n", size_of_internal_fragmantation);
    printf("Total number of rejected processes due to external fragmentation: %d\n", num_of_rejected_external_fragmantation);
    printf("Total number of rejected processes due to insufficient memory: %d\n", num_of_insufficient);
}




void *threadFun(void *filename){
   int myid;

   pthread_mutex_lock(&lock);
   myid=threadnum++;

   char my_filename[50];
   sprintf(my_filename, "%s_%d.txt", (char *)filename, myid);

   printf("Thread %d will read %s \n",myid,(char*)my_filename);

//  your code goes here.
    int pid, size_of_p;
    int frame_size_of_p;
    char *letter;
    char line [50];
    char string[50] = "Hello world";

    FILE * file;
    file = fopen(my_filename , "r");

    while(fgets(line,sizeof line,file)!= NULL)
    {

            char *p;
            p = strtok(line, "\t");
            if(p)
            {
                letter=p;
                printf("%s\t", letter);
                p = strtok(NULL, "\t");
            }

            if(p)
            {
                pid=atoi(p);
                printf("%d\t", pid);
                p = strtok(NULL, "\t");
            }


            if(p)
            {
                size_of_p=atoi(p);
                printf("%d\t", size_of_p);
            }

            frame_size_of_p = size_of_p / 4;

            if(size_of_p % 4 != 0)
            {
                frame_size_of_p++;
                size_of_internal_fragmantation += frame_size_of_p * 4 - size_of_p;
            }


            if(*letter == 'B' && method == 1)
            {
                firstFitAllocate(pid, frame_size_of_p);
            }

            else if(*letter == 'B' && method == 2)
            {
                bestFitAllocate(pid, frame_size_of_p);
            }

            else if(*letter == 'B' && method == 3)
            {
                worstFitAllocate(pid, frame_size_of_p);
            }

            else if(*letter == 'E')
                deallocate_memory(pid);

            else
                printf("There is an error in %s file!", my_filename);

            printf("\n");
    }

    print_memory_condition();
    calculateHoles();


    printf("\n");
    pthread_mutex_unlock(&lock);

}

//
// Do not modify main function
//
int main(int argc, char *argv[]){
   int size = atoi(argv[1]);
   MEMORY_size=size/4;
   remaining_frame = size / 4;
   MEMORY = calloc(MEMORY_size,sizeof(int));  // each block is 4KB

   method = atoi(argv[3]);

   pthread_mutex_init(&lock, NULL);
   char method_str[50];
   if(method == 1){
      sprintf(method_str,"First_fit");
   }
   else if(method == 2){
      sprintf(method_str,"Best_fit");
   }
   else if(method == 3){
      sprintf(method_str,"Worst_fit");
   }
   printf("Program has launched, %s strategy will be used\n",method_str);
   pthread_t tid[4];
   for(int i=0;i<4;i++){
      pthread_create(&tid[i], NULL, threadFun, argv[2]);
   }
   for(int i=0;i<4;i++){
      pthread_join(tid[i], NULL);
   }
   return 0;
}
