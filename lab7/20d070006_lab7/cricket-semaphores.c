#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

int capitals = 0;
int titans = 0 ;
int capitals_score = 0 ;
int titans_score = 0 ;
int capitals_wins = 0 ;
int capitals_lose = 0 ;
int titans_lose = 0 ;
int titans_wins = 0 ;


int highest_team_score = 0,highest_ind_score = 0 , hmi_score = 0 ;
int completed = 0 ;

sem_t sem1  ;
sem_t sem_score ;
sem_t sem_com ;

int max(int a,int b){
    if (a >= b) return a ;
    else return b ;
}

void* player(void * arg){
   sem_wait(&sem1) ;
   if(capitals < 11){
        sem_wait(&sem_score) ;
        capitals++ ;
        int player = rand()%101 ; 
        hmi_score = max(hmi_score,player) ;
        capitals_score += player ;
        sem_post(&sem_score) ;
    }
    else if(titans < 11){
        sem_wait(&sem_score) ;
        titans++ ;
        int player = rand()%101 ; 
        hmi_score = max(hmi_score,player) ;
        titans_score += player ;
        sem_post(&sem_score) ;
    }
    printf("Player ---------\n") ;
    sem_wait(&sem_com) ;
    completed++ ;
    sem_post(&sem_com) ;
}

void *game(void *arg){
    int num_matches = *((int *)arg) ;
    int threadNum = num_matches*22 ;
    pthread_t th[threadNum];
    for(int i = 0 ; i < num_matches*22 ; i++){
        if (pthread_create(th + i, NULL, &player,NULL) != 0) {
                perror("Failed to create thread");
                return (void *)1;
        }
    }
    for(int i = 0 ; i < num_matches ; i++){
        while( completed < 22){
            sem_wait(&sem_com) ;
            sem_post(&sem_com) ;
        }
        printf("-----------------MATCH : ( %d ) Summary-----------------------\n\n",i+1) ;
        printf("SCORE: Capitals : %d :: Titans : %d\n",capitals_score,titans_score) ;
        printf("Highest Individual Score : %d\n",hmi_score) ;
        if(capitals_score >= titans_score){
            printf("Results : Capitals won by %d runs\n\n",capitals_score - titans_score) ;
            capitals_wins++ ;
            titans_lose++ ;
        }
        else{
            printf("Results : Titans won by %d runs\n\n",titans_score - capitals_score) ;
            capitals_lose++ ;
            titans_wins++ ;
        }
        highest_team_score = max(highest_team_score,max(capitals_score,titans_score)) ;
        highest_ind_score = max(highest_ind_score,hmi_score) ;
        completed = 0 ;
        hmi_score = 0 ;
        titans = 0 ;
        capitals = 0 ;
        titans_score = 0 ;
        capitals_score = 0 ;
        for(int j = 0 ; j < 22 ; j++){
            sem_post(&sem1) ;
        }
    }
    for(int k = 0 ; k < num_matches*22 ; k++){
        if (pthread_join(th[k], NULL) != 0) {
            return (void *)2;
        }
    }
    printf("---------------------SUMMARY OF THE DAY---------------------------\n");
    printf("Titans    :: Won : %d || Lost : %d || Tied : %d\n",titans_wins,titans_lose,num_matches-titans_lose-titans_wins) ;
    printf("Capitalss :: Won : %d || Lost : %d || Tied : %d\n",capitals_wins,capitals_lose,num_matches-titans_lose-titans_wins) ;
    printf("Highest Team Score            : %d\n",highest_team_score) ;
    printf("Highest Individual Score      : %d\n",highest_ind_score) ;
    printf("------------------------------------------------------------------\n") ;
}


int main(int argc, char *argv[]){
    int num_players = atoi(argv[1]) ;
    int num_matches = num_players/22 ;
    sem_init(&sem1,0,22) ;
    sem_init(&sem_score,0,1) ;
    sem_init(&sem_com,0,1) ;
    if(num_players%22 != 0){
        printf("Match %d cannot take place\n",num_matches+1) ;
    }
    pthread_t game_thread ;
    if (pthread_create(&game_thread, NULL, &game,(void *)&num_matches) != 0) {
                perror("Failed to create thread");
                return 1;
    }
    if (pthread_join(game_thread, NULL) != 0) {
            return 2;
    }
}