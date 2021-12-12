//printf("my notes:\n"
//"- i have left some extra comment lines that helps me while iam working, so you can see how i was thinking.\n"
//"- the entered and left messages are function based because i wanted to see it while debugging. and left them as they are.\n"
//"- if we want of course its easy to change location of a print statement.\n"
//"- the app will start in 5 secs...");
//sleep(5);



#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<semaphore.h>
#include<time.h>


#define randnum(max) ((rand() % (int) ((max))) + (1))
//usage: randnum(max int));


struct patientStruct {
    int id;
    int disease;//0:nothing, 1:medicine, 2:blood lab, 3:surgery
    int hungerMeter;
    int restroomMeter;
};


pthread_mutex_t walletLock;
pthread_mutex_t nurseLock;
pthread_mutex_t surgeonLock;

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

int HUNGER_LIMIT = 100;
int REST_LIMIT = 100;

void gp(struct patientStruct p);

void afterGP(struct patientStruct p);

void sleepByMiliseconds(int t) {
    //sleep(t / 1000);//this was causing segmentation fault sometimes. (probably it is because float number seconds sleep)
    usleep(t * 1000);
}

void walletIncomeUpdate(int cost) {
    //todo use mutexes
    pthread_mutex_lock(&walletLock);
    HOSPITAL_WALLET += cost;
    pthread_mutex_unlock(&walletLock);
}


void hungerAndRestOperations(struct patientStruct p) {
    //in my opinion toilet is more important than hungary.
    // so I am writing my code according to that. if I was hungary and needed to go to toilet. I would prefer toilet %100.

    //I am deleting(commenting) print lines in this method because this method runs(for sem_trywait checks) for many times,
    //so it would print too much.
    //printf("patient%d has arrived to hungerAndRestOperations\n", p.id);
    p.hungerMeter += randnum(HUNGER_INCREASE_RATE);
    p.restroomMeter += randnum(RESTROOM_INCREASE_RATE);
    if (p.restroomMeter >= REST_LIMIT) {
        sem_wait(&semRestroom);
        printf("patient%d is in restroom\n", p.id);
        sleepByMiliseconds(randnum(RESTROOM_TIME));
        sem_post(&semRestroom);
        p.restroomMeter=1;
    }
    if (p.hungerMeter >= HUNGER_LIMIT) {
        sem_wait(&semCafe);
        printf("patient%d is in cafe\n", p.id);
        sleepByMiliseconds(randnum(CAFE_TIME));
        walletIncomeUpdate(randnum(CAFE_COST));
        sem_post(&semCafe);
        p.hungerMeter=1;
    }
    //printf("patient%d has left hungerAndRestOperations\n", p.id);
}


void surgery(struct patientStruct p) {
    //todo there can be deadlocks
    printf("patient%d has arrived to surgery\n", p.id);
    while (1) {//take or
        if(sem_trywait(&semOR)==0){
            break;
        }else{
            hungerAndRestOperations(p);
            sleepByMiliseconds(randnum(WAIT_TIME));
        }
    }
    //select staff count for this operation
    int nurseCountForThisOperation = randnum(NURSE_LIMIT);
    int surgeonCountForThisOperation = randnum(SURGEON_LIMIT);

    pthread_mutex_lock(&nurseLock);//taking nurses

    for (int i = 0; i < nurseCountForThisOperation; ++i) {
        while (1) {
            if(sem_trywait(&semNurse)==0){
                break;
            }else{
                hungerAndRestOperations(p);
                sleepByMiliseconds(randnum(WAIT_TIME));
            }
        }
    }
    pthread_mutex_unlock(&nurseLock);//we took enough nurses
    pthread_mutex_lock(&surgeonLock);//taking the surgeons
    for (int i = 0; i < surgeonCountForThisOperation; ++i) {
        while (1) {
            if(sem_trywait(&semSurgeon)==0){
                break;
            }else{
                hungerAndRestOperations(p);
                sleepByMiliseconds(randnum(WAIT_TIME));
            }
        }
    }
    pthread_mutex_unlock(&surgeonLock);//we took enough surgeons

    //here the deadlock possibility ends, so we are not limiting our whole method with mutexes.
    //other threads can take nurses/surgeons while other surgery operations happening, since they(operations) already took them(stuffs).

    printf("patient%d is in surgery with %d surgeons and %d nurses\n", p.id, surgeonCountForThisOperation,
           nurseCountForThisOperation);
    sleepByMiliseconds(randnum(SURGERY_TIME));

    //calculate cost
    int cost = SURGERY_OR_COST + (surgeonCountForThisOperation * SURGERY_SURGEON_COST) +
               (nurseCountForThisOperation * SURGERY_NURSE_COST);
    walletIncomeUpdate(cost);

    //release the staff and the room
    sem_post(&semOR);
    for (int i = 0; i < nurseCountForThisOperation; ++i) {
        sem_post(&semNurse);
    }
    for (int i = 0; i < surgeonCountForThisOperation; ++i) {
        sem_post(&semSurgeon);
    }
    //hungerAndRestOperations(p); //we dont write this line here because it is already written in the method that we called from.
    printf("patient%d has exited from surgery\n", p.id);
    gp(p);
}

void gp(struct patientStruct p) {
    //GP
    printf("patient%d has arrived to gp\n", p.id);
    //int val;
    //sem_getvalue(&semGP, &val);
    while (1) {
        if(sem_trywait(&semGP)==0){
            break;
        }else{
            hungerAndRestOperations(p);
            sleepByMiliseconds(randnum(WAIT_TIME));
        }
    }
    printf("patient%d is in GP\n", p.id);
    if (p.disease ==
        0) {//if disease is zero then the patient is coming to the gp for the first time so a disease should be assigned to her/him.
        p.disease = randnum(3);
        printf("patient%d assigned to disease%d by GP\n", p.id, p.disease);
    } else {//else (s)he is already coming from another operation, and we will give medicine or not. just 2 possibilities.
        p.disease = (randnum(2) - 1);
        printf("patient%d reassigned to disease%d by GP\n", p.id, p.disease);
    }
    sleepByMiliseconds(randnum(GP_TIME));
    sem_post(&semGP);
    hungerAndRestOperations(p);
    printf("patient%d has exited from gp\n", p.id);
    afterGP(p);
}

void afterGP(struct patientStruct p) {
    printf("patient%d has arrived to afterGP\n", p.id);
    //since this is an if else if statement i wrote the hunger and restroom checks at the bottom.
    //medicine
    if (p.disease == 1) {
        while (1) {
            if(sem_trywait(&semPharmacy)==0){
                break;
            }else{
                hungerAndRestOperations(p);
                sleepByMiliseconds(randnum(WAIT_TIME));
            }
        }
        printf("patient%d is in pharmacy\n", p.id);
        sleepByMiliseconds(randnum(PHARMACY_TIME));
        sem_post(&semPharmacy);
        walletIncomeUpdate(randnum(PHARMACY_COST));
    }//blood lab
    else if (p.disease == 2) {
        while (1) {
            if(sem_trywait(&semBlood)==0){
                break;
            }else{
                hungerAndRestOperations(p);
                sleepByMiliseconds(randnum(WAIT_TIME));
            }
        }
        printf("patient%d is in blood lab\n", p.id);
        sleepByMiliseconds(randnum(BLOOD_LAB_TIME));
        sem_post(&semBlood);
        walletIncomeUpdate(BLOOD_LAB_COST);//recalculated to a static random value at main
        gp(p);
    }//surgery
    else if (p.disease == 3) {
        surgery(p);
    }
    hungerAndRestOperations(p);
    printf("patient%d has left the hospital\n", p.id);
}


void *patient(void *arg) {
    //getting and casting the patient
    struct patientStruct *p = (struct patientStruct *) arg;
    printf("patient%d has entered to the hospital\n", p->id);

    //registration
    //sem_wait(&semRegistration);

    while (1) {
        if(sem_trywait(&semRegistration)==0){//if we take the semaphore then break so operations can be done.
            break;
        }else{// if we didn't take the semaphore we will try again after time and check for basic needs.
            hungerAndRestOperations(*p);
            sleepByMiliseconds(randnum(WAIT_TIME));
        }
    }
    printf("patient%d is in registration office\n", p->id);
    sleepByMiliseconds(randnum(REGISTRATION_TIME));
    walletIncomeUpdate(REGISTRATION_COST);//recalculated to a static random value at main
    sem_post(&semRegistration);
    hungerAndRestOperations(*p);//think food and toilet after leaving the room of doctor/registering/smt/smt...
    //u shouldn't leave the room while you are in there. This would be disrespectful.
    //so I post the semaphore and then call the hunger things...
    printf("patient%d has exited from registration office\n", p->id);
    gp(*p);

}


int main(void) {

    //I used forks while testing the app. they are not related/necessary with this app.
//    for (int i = 0; i < 100; ++i) {
//        fork();
//    }
//    fork();
//    fork();
//    fork();
//    fork();

    //initializing srand for random number function
    srand(time(NULL));
    //usage: randnum(max int));

    //static costs calculations
    int regCost = randnum(REGISTRATION_COST);
    REGISTRATION_COST = regCost;
    int bloodLabCost = randnum(BLOOD_LAB_COST);
    BLOOD_LAB_COST = bloodLabCost;

    int value;//value for sem get

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
    struct patientStruct *patients = (struct patientStruct *) malloc(PATIENT_NUMBER * sizeof(struct patientStruct));
    //struct patientStruct patients[PATIENT_NUMBER] = (struct patientStruct[PATIENT_NUMBER]) malloc(PATIENT_NUMBER * sizeof(struct patientStruct));
    //struct patientStruct patients[PATIENT_NUMBER];
    pthread_t threads[PATIENT_NUMBER];

    //thread creations
    for (int i = 0; i < PATIENT_NUMBER; ++i) {
//        sleepByMiliseconds(randnum(WAIT_TIME));
        patients[i].id = i;
        patients[i].disease = 0;
        patients[i].hungerMeter = randnum(REST_LIMIT);
        patients[i].restroomMeter = randnum(REST_LIMIT);
        if (pthread_create(threads + i, NULL, patient, (void *) &patients[i]) != 0)
            printf("\nFailed Thread Creation\n");
    }


    //thread joins
    for (int i = 0; i < PATIENT_NUMBER; ++i) {
        if (pthread_join(*(threads + i), NULL) != 0) {
            printf("\nFailed Thread Join\n");
        }
    }

//    //final value of registration semaphore
//    sem_getvalue(&semRegistration, &value);
//    printf("\nMain thread: Final value of the semaphore registration is %d\n\n", value);

    printf("\nHospital wallet is: %d\n\n", HOSPITAL_WALLET);

    //destroy
    sem_destroy(&semRestroom);
    sem_destroy(&semRegistration);
    sem_destroy(&semCafe);
    sem_destroy(&semGP);
    sem_destroy(&semPharmacy);
    sem_destroy(&semBlood);
    sem_destroy(&semOR);
    sem_destroy(&semNurse);
    sem_destroy(&semSurgeon);
    return 0;
}

