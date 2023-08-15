#include <pthread.h>        // NOTE
#include <semaphore.h>      // NOTE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int max(int a, int b) {
    if(a>=b) return a;
    else return b;
}

// https://greenteapress.com/thinkos/thinkos.pdf
// Page-82
void semaphore_wait(sem_t* sem) {
    if(sem_wait(sem)!=0) {
        perror("sem_wait failed\n");
        exit(1);
    }
}

void semaphore_post(sem_t* sem) {
    if(sem_post(sem)!=0) {
        perror("sem_post failed\n");
        exit(1);
    }
}

sem_t can_modify;
sem_t match_done;

// https://github.com/remzi-arpacidusseau/ostep-code/blob/master/intro/common_threads.h
#define Pthread_create(thread, attr, start_routine, arg) assert(pthread_create(thread, attr, start_routine, arg) == 0);
#define Pthread_join(thread, th_return) assert(pthread_join(thread, th_return) == 0);

int capitals_runs, titans_runs;
int capitals_counter, titans_counter;
int max_indiv_score;

void* player_fn(void* arg) {
    int score = rand()%101;

    semaphore_wait(&can_modify);

    // printf("Player %d started (score %d); %d, %d\n", *(int*)arg, score, capitals_counter, titans_counter);

    if(capitals_counter < 11) {
        capitals_counter++;
        capitals_runs += score;
    }
    else {
        titans_counter++;
        titans_runs += score;
    }
    max_indiv_score = max(max_indiv_score, score);

    // printf("Player %d ended; %d, %d\n", *(int*)arg, capitals_counter, titans_counter);

    if(capitals_counter==11 && titans_counter==11) {
        semaphore_post(&match_done);        // now game thread will post to can_modify
    }
    else {
        semaphore_post(&can_modify);
    }
}

void* game_fn(void* arg) {

    int num_matches = *(int*)arg;
    int num_players = num_matches*22;
    pthread_t pool[num_players];

    int player_nums[num_players];
    for(int i=0; i<num_players; i++) {
        player_nums[i] = i;
        Pthread_create(&pool[i], NULL, &player_fn, (void*)&player_nums[i]);     // *** cannot use &i
                                                                                // address must be different
                                                                                // to get different values
    }

    int capitals_wins = 0, titans_wins = 0;
    int max_indiv_overall = 0;
    int max_team_overall = 0;

    capitals_runs = 0; titans_runs = 0;
    capitals_counter = 0; titans_counter = 0;
    max_indiv_score = 0;

    for(int i=0; i<num_matches; i++) {

        semaphore_post(&can_modify);
        semaphore_wait(&match_done);

        // print this match's stats
        printf("--------------------MATCH : ( %d ) Summary--------------------\n", i+1);
        printf("SCORE: Capitals : %d :: Titans : %d\n", capitals_runs, titans_runs);
        printf("Highest Individual Score : %d\n", max_indiv_score);
        if(capitals_runs>titans_runs) {
            printf("Result : Capitals won by %d runs\n", capitals_runs-titans_runs);
            capitals_wins++;
        }
        else if(capitals_runs<titans_runs) {
            printf("Result : Titans won by %d runs\n", titans_runs-capitals_runs);
            titans_wins++;
        }
        else {
            printf("Match Tied. Both teams scored %d runs\n", capitals_runs);
        }

        max_indiv_overall = max(max_indiv_overall, max_indiv_score);
        max_team_overall = max(max_team_overall, max(capitals_runs, titans_runs));

        capitals_counter = 0; titans_counter = 0;
        capitals_runs = 0; titans_runs = 0;
        max_indiv_score = 0;

    }

    for(int i=0; i<num_players; i++) {
        Pthread_join(pool[i], NULL);
    }

    // print overall
    int num_tied = num_matches-titans_wins-capitals_wins;
    printf("----------------------SUMMARY OF THE DAY----------------------\n");
    printf("Matches Played : %d\n", num_matches);
    printf("Titans   :: Won : %d || Lost : %d || Tied : %d\n", titans_wins, capitals_wins, num_tied);
    printf("Capitals :: Won : %d || Lost : %d || Tied : %d\n", capitals_wins, titans_wins, num_tied);
    printf("Highest Team Score           : %d\n", max_team_overall);
    printf("Highest Individual Score     : %d\n", max_indiv_overall);
    printf("--------------------------------------------------------------\n");

}

int main(int argc, char** argv) {

    if (argc!=2) {
        printf("Usage: ./a.out <number of players>\n");
        exit(1);
    }

    srand(0);

    int num_players = atoi(argv[1]);
    int num_matches = num_players/22;

    if(sem_init(&can_modify,0,0)!=0) {
        perror("sem_init failed");      // e.g., if (&sem,0,-1) => error
        exit(1);
    }
    if(sem_init(&match_done,0,0)!=0) {
        perror("sem_init failed");
        exit(1);
    }

    pthread_t game_thread;
    if(pthread_create(&game_thread, NULL, &game_fn, (void*)&num_matches)!=0) {
        perror("Game thread creation failed");
        exit(1);
    }
    pthread_join(game_thread, NULL);

    if(num_players%22!=0) {
        printf("Match %d could not be played due to lack of players\n", num_matches+1);
    }

}