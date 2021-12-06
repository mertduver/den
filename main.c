#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<semaphore.h>
#include<time.h>


#define randnum(max) ((rand() % (int) (((max) + 1) - (1))) + (1))
//usage: randnum(max int));


struct patientStruct {
    int id;
    int disease;//0:nothing, 1:medicine, 2:blood lab, 3:surgery
    float bill;
    int hungerMeter;
    int restroomMeter;
};



pthread_mutex_t walletLock;

sem_t semRestroom;
sem_t semRegistration;
sem_t semCafe;
sem_t semGP;
sem_t semPharmacy;
sem_t semBlood;
sem_t semOR;
sem_t semNurse;
sem_t semSurgeon;


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

int HUNGER_LIMIT=100;
int REST_LIMIT=100;


void sleepByMiliseconds(int t){
//    for (int i = 0; i < 100; ++i) {
//    }
    sleep(t/1000);
}

void walletIncomeUpdate(int cost){
    //todo use mutexes
    pthread_mutex_lock(&walletLock);
    HOSPITAL_WALLET+=cost;
    pthread_mutex_unlock(&walletLock);

}




void hungerAndRestOperations(struct patientStruct p){
    sleepByMiliseconds(randnum(WAIT_TIME));
    p.hungerMeter+= randnum(HUNGER_INCREASE_RATE);
    p.restroomMeter+= randnum(RESTROOM_INCREASE_RATE);
    if(p.restroomMeter>=REST_LIMIT){
        sem_wait(&semRestroom);
        sleepByMiliseconds(randnum(RESTROOM_TIME));
        sem_post(&semRestroom);
        p.restroomMeter=1;
    }
    if(p.hungerMeter>=HUNGER_LIMIT){
        sem_wait(&semCafe);
        sleepByMiliseconds(randnum(CAFE_TIME));
        sem_post(&semCafe);
        walletIncomeUpdate(randnum(CAFE_COST));
        p.hungerMeter=1;
    }
}

void gp(struct patientStruct p){
    //GP
    sem_wait(&semGP);
    printf("patient%d entered to GP\n",p.id);
    sleepByMiliseconds(randnum(GP_TIME));
    sem_post(&semGP);
    hungerAndRestOperations(p);
}

void surgery(struct patientStruct p){
    //todo there can be deadlocks

}

void afterGP(struct patientStruct p){
    //medicine
    if(p.disease==1){
        sem_wait(&semPharmacy);
        printf("patient%d entered to pharmacy\n",p.id);
        sleepByMiliseconds(randnum(PHARMACY_TIME));
        sem_post(&semPharmacy);
        walletIncomeUpdate(randnum(PHARMACY_COST));
    }//blood lab
    else if(p.disease==2){
        sem_wait(&semBlood);
        printf("patient%d entered to blood lab\n",p.id);
        sleepByMiliseconds(randnum(BLOOD_LAB_TIME));
        sem_post(&semBlood);
        walletIncomeUpdate(BLOOD_LAB_COST);//recalculated to a static random value at main
        p.disease= (randnum(2)-1);
        gp(p);
        afterGP(p);//patient needs to check if (s)he needs to buy medicine or not
    }//surgery
    else if(p.disease==3){
        surgery(p);
    }
    hungerAndRestOperations(p);
}


void *patient(void *arg)
{
    //getting and casting the patient
    struct patientStruct* p= (struct patientStruct*) arg;

    //registration
    sem_wait(&semRegistration);
    printf("patient%d entered to registration office\n",p->id);
    sleepByMiliseconds(randnum(REGISTRATION_TIME));
    sem_post(&semRegistration);
    walletIncomeUpdate(REGISTRATION_COST);//recalculated to a static random value at main
    hungerAndRestOperations(*p);

    gp(*p);
    afterGP(*p);





}



int main(void)
{

    //initializing srand for random number function
    srand(time(NULL));
    //usage: randnum(max int));

    //static costs calculations
    int regCost= randnum(REGISTRATION_COST);
    REGISTRATION_COST=regCost;
    int bloodLabCost= randnum(BLOOD_LAB_COST);
    BLOOD_LAB_COST=bloodLabCost;

    int err;
    int value;

    //semaphore initializations
    sem_init(&semRestroom, 0, RESTROOM_SIZE);
    sem_init(&semRegistration, 0, REGISTRATION_SIZE);
    sem_init(&semCafe, 0, CAFE_NUMBER);
    sem_init(&semGP, 0, GP_NUMBER);
    sem_init(&semPharmacy, 0, PHARMACY_NUMBER);
    sem_init(&semBlood, 0, BLOOD_LAB_NUMBER);
    sem_init(&semOR, 0, OR_NUMBER);
    sem_init(&semNurse, 0, NURSE_NUMBER);
    sem_init(&semSurgeon, 0, SURGEON_NUMBER);

    //patient array
    struct patientStruct *patients=(struct patientStruct*) malloc(PATIENT_NUMBER* sizeof(struct patientStruct));
    pthread_t *threads;

    //thread creations
    for (int i = 0; i < PATIENT_NUMBER; ++i) {
        patients[i].id=i;
        patients[i].disease= randnum(3);
        patients[i].hungerMeter= randnum(100);
        patients[i].restroomMeter= randnum(100);
        err=pthread_create(threads+i,NULL,patient,(void *)&patients[i]);
        if (err != 0)
            printf("\nThread creation error: [%s]\n",strerror(err));
    }


    //thread joins
    for (int i = 0; i < PATIENT_NUMBER; ++i) {
        pthread_join(*(threads+i),NULL);
    }

    //final value
    sem_getvalue(&semRegistration, &value);
    printf("\nMain thread: Final value of the semaphore registration is %d\n\n", value);

    //destroy
    sem_destroy(&semRestroom);
    sem_destroy(&semRegistration);
    sem_destroy(&semCafe);
    sem_destroy(&semGP);
    sem_destroy(&semPharmacy);
    sem_destroy(&semBlood);
    sem_destroy(&semOR);
    sem_destroy(&semNurse);

    return 0;
}

