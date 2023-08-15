#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

int players = 0;
int counter_capital = 0;
int counter_titan = 0;
int capital_score = 0;
int titan_score = 0;
int maxscore = 0;
int match = 0;
int maxscore_match = 0;
int individual_score = 0;
int titanswin = 0;
int capitalswin = 0;
int tied = 0;
int permatchscore = 0;

sem_t barrier;
sem_t mut;
sem_t sig;
int gameover = 0;
int matches = 0;
pthread_mutex_t mutex1;

void *bat(void *playerid)
{

    sem_wait(&barrier);

    sem_wait(&mut);
    int run = 0;
    if (counter_capital < 11)
    {
        
        counter_capital++;
        run = rand() % 101;
        capital_score += run;
        if (run > individual_score)
            individual_score = run;

        if (run > permatchscore)
            permatchscore = run;

        // printf("Counter Capial : %d\n", counter_capital);
    }
    else
    {
        counter_titan++;
        run = rand() % 101;
        titan_score += run;
        if (run > individual_score)
        {
            individual_score = run;
        }
        if (run > permatchscore)
        {
            permatchscore = run;
        }
        // printf("Counter Titan : %d\n", counter_titan);

        if (counter_titan == 11)
        {
            gameover = 1;
            sem_post(&sig);
        }
    }
    sem_post(&mut);
}

void *startGame()
{
    // Start of the Game all variables reset
   

    while (match < matches)
    {
        
        sem_wait(&sig);
        sem_wait(&mut);

        match++;

        printf("---------------------MATCH : ( %d ) Summary------------------\n\n", match);
        printf("SCORE: Capitals : %d :: Titans : %d \n", capital_score, titan_score);
        printf("Highest Individual Score : %d \n", permatchscore);

        if (capital_score > titan_score)
        {
            printf("Result : Capitals won by %d runs\n", capital_score - titan_score);
            capitalswin++;
            if (capital_score > maxscore)
            {
                maxscore = capital_score;
            }
        }
        else if (titan_score > capital_score)
        {
            titanswin++;
            printf("Result : Titans won by %d runs\n", titan_score - capital_score);
            if (titan_score > maxscore)
            {
                maxscore = titan_score;
            }
        }
        else
        {
            tied++;
            printf("Result : Match Tied\n");
            maxscore = capital_score > maxscore ? capital_score : maxscore;
        }
        printf("\n");

        gameover = 0;
        permatchscore = 0;
        counter_capital = 0;
        counter_titan = 0;
        capital_score = 0;
        titan_score = 0;

        for(int j = 0; j<22; j++){
            sem_post(&barrier);
        }
        
         sem_post(&mut);


    }

   
}

int main(int argc, char *argv[])
{

    int threadNum = atoi(argv[1]);
    matches = threadNum / 22;
    pthread_t players[threadNum + 1];
    sem_init(&barrier, 0, 22);
    sem_init(&mut, 0, 1);
    sem_init(&sig, 0, 1);
    sem_wait(&sig);
    srand(time(NULL));

    int i;
    for (i = 0; i < threadNum + 1; i++)
    {
        int *a = malloc(sizeof(int));
        *a = i;
        if (i == 0)
        {
            pthread_create(players + i, NULL, &startGame, NULL);
        }
        else
        {
            if (pthread_create(players + i, NULL, &bat, a) != 0)
            {
                perror("Failed to create Game thread");
                return 1;
            }
        }
    }

    for (i = 0; i < threadNum + 1; i++)
    {
        if (pthread_join(players[i], NULL) != 0)
        {
            // printf("Thread joined : %d", i);
            return 2;
        }
    }

    // Print Days Summary

    printf("---------------------SUMMARY OF THE DAY---------------------\n");
    printf("Matches Played : %d \n", matches);
    printf("Titans   :: Won : %d || Lost : %d || Tied : %d\n", titanswin, capitalswin, tied);
    printf("Capitals :: Won : %d || Lost : %d || Tied : %d\n", capitalswin, titanswin, tied);
    printf("Highest Team Score          : %d\n", maxscore);
    printf("Highest Individual Score    : %d\n", individual_score);
    printf("------------------------------------------------------------\n");

    return 0;
}