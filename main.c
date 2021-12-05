#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<semaphore.h>
#include <time.h>


#define THREAD_COUNT 20
#define randnum(min, max) ((rand() % (int) (((max) + 1) - (min))) + (min))
//usage: randnum(min int, max int));


struct patientStruct {
    int id;
    int disease;//0:medicine, 1:surgery
    float bill;
    int hungerMeter;
    int restroomMeter;
};

sem_t semRegistration;
sem_t semRestroom;
sem_t semCafe;
sem_t semGP;

//# The number of restrooms that are available.
int RESTROOM_SIZE = 10;
//# The number of registration desks that are available.
int REGISTRATION_SIZE = 10;
//# The number of cashiers in cafe that are available.
int CAFE_NUMBER = 10;
//# The number of General Practitioner that are available.
int GP_NUMBER = 10;
//# The number of cashiers in pharmacy that are available.
int PHARMACY_NUMBER = 10;
//# The number of assistants in blood lab that are available.
int BLOOD_LAB_NUMBER = 10;
//The number of operating rooms, surgeons and nurses that are available.
int OR_NUMBER = 10;
int SURGEON_NUMBER = 30;
int NURSE_NUMBER = 30;

//The maximum number of surgeons and nurses that can do a surgery. A random value is
//calculated for each operation between 1 and given values to determine the required
//number of surgeons and nurses.
int SURGEON_LIMIT = 5;
int NURSE_LIMIT = 5;

// The number of patients that will be generated over the course of this program.
int PATIENT_NUMBER = 1000;
// The account of hospital where the money acquired from patients are stored.
int HOSPITAL_WALLET = 0;

int WAIT_TIME = 100;
int REGISTRATION_TIME = 100;
int GP_TIME = 200;
int PHARMACY_TIME = 100;
int BLOOD_LAB_TIME = 200;
int SURGERY_TIME = 500;
int CAFE_TIME = 100;
int RESTROOM_TIME = 100;

int REGISTRATION_COST = 100;
int PHARMACY_COST = 200; // Calculated randomly between 1 and given value.
int BLOOD_LAB_COST = 200;
int SURGERY_OR_COST = 200;
int SURGERY_SURGEON_COST = 100;
int SURGERY_NURSE_COST = 50;
int CAFE_COST = 200; // Calculated randomly between 1 and given value.

int HUNGER_INCREASE_RATE = 10;
int RESTROOM_INCREASE_RATE = 10;

void *patient(void *arg)
{
    struct patientStruct* p= (struct patientStruct*) arg;
    printf("id:%d",p->id);
    printf("hungerMeter:%d",p->hungerMeter);

    sem_wait(&semRegistration);
    printf("patient%s Entered the registration office\n",p->id);
    sleep(randnum(1,REGISTRATION_TIME/10));
    sem_post(semRegistration);

    sem_wait(semGP);
    printf("patient%s Entered the GP\n",p->id);
    sleep(randnum(1,GP_TIME/10));
    sem_post(&semGP);
}


int main()
{

    srand(time(NULL));
    //usage: randnum(min int, max int));


    int err;
    int value;

    sem_init(&semRegistration, 0, REGISTRATION_SIZE);
    sem_init(&semGP, 0, GP_NUMBER);

    struct patientStruct *patients[PATIENT_NUMBER];

    for (int i = 0; i < PATIENT_NUMBER; ++i) {
        patients[i]->id=i;
        patients[i]->disease= randnum(0,1);
        patients[i]->hungerMeter= randnum(1,100);
        patients[i]->restroomMeter= randnum(1,100);
        err=pthread_create(i,NULL,patient,(void *)patients[i]);
        if (err != 0)
            printf("Thread creation error: [%s]",
                   strerror(err));
    }


    for (int i = 0; i < PATIENT_NUMBER; ++i) {
        pthread_join(patients[i],NULL);
    }

    sem_getvalue(&semRegistration, &value);
    printf("\n");
    printf("Main thread: Final value of the semaphore registration is %d\n\n", value);
    sem_destroy(&semRegistration);

    return 0;
}

