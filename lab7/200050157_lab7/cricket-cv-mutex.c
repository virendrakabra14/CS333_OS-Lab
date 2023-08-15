#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int max(int a, int b) {
    if(a>=b) return a;
    else return b;
}

int capitals_counter, titans_counter;
int capitals_runs, titans_runs, max_indiv_score;     // in a single match
pthread_mutex_t counters_runs_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t can_enter = PTHREAD_COND_INITIALIZER;

pthread_cond_t match_done = PTHREAD_COND_INITIALIZER;

void* play(void* arg) {

    int score = rand()%101;

    // NOTE: for the below while(...) for cond_wait,
    // a tmp_mutex is NOT OK.
    // What was happening in some executions was:
    // almost ALL player threads run BEFORE game_thread executing its
    // for() loop

    // so, almost ALL player threads run till entirety, even before
    // the game thread starts any matches
    // Seems like ALL threads first pass the while(...) loop without having to wait,
    // and then descheduled,
    // and then play
    //      *** GOT TO KNOW VIA printf's in this function ***

    // SO, using counters_runs_lock at the start was necessary
    // but now, the only thing really "parallelized" is the rand() call!

    pthread_mutex_lock(&counters_runs_lock);
    while(capitals_counter==11 && titans_counter==11) {
        pthread_cond_wait(&can_enter, &counters_runs_lock);
    }

    // printf("Player %d started playing\n", *(int*)arg);
    if(capitals_counter<11) {
        capitals_counter++;
        capitals_runs += score;
    }
    else {
        titans_counter++;
        titans_runs += score;
    }
    max_indiv_score = max(max_indiv_score, score);

    if(capitals_counter==11 && titans_counter==11) {
        pthread_cond_signal(&match_done);
    }
    // printf("Player %d ended playing\n", *(int*)arg);
    pthread_mutex_unlock(&counters_runs_lock);

}

void* game_fn(void* arg) {
    int num_matches = *(int*)(arg);
    int num_players = 22*num_matches;
    
    capitals_counter = 0; titans_counter = 0;
    capitals_runs = 0; titans_runs = 0;

    pthread_t pool[num_players];
    
    // as required, created ALL player threads at the beginning
    for(int j=0; j<num_players; j++) {
        if(pthread_create(&pool[j],NULL,&play,(void*)&j) != 0) {        // passing '&j' is of no use,
                                                                        // as it will finally have
                                                                        // value = num_players+1...
                                                                        // see semaphores.c (used array of values)
            printf("Couldn't create player thread %d (0-indexed)\n",j);
            exit(1);
        }
    }

    int max_team_score = 0, max_individual_score = 0;
    int capitals_wins = 0, titans_wins = 0;
    char winner[10];

    for(int i=0; i<num_matches; i++) {

        pthread_mutex_lock(&counters_runs_lock);
        while(capitals_counter<11 || titans_counter<11) {
            pthread_cond_wait(&match_done, &counters_runs_lock);
        }
        max_team_score = max(max_team_score, max(capitals_runs, titans_runs));
        max_individual_score = max(max_individual_score, max_indiv_score);

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

        capitals_runs = 0; titans_runs = 0;
        capitals_counter = 0; titans_counter = 0;

        // done with the runs; reset counters; now signal next batch:
        // broadcast seems inefficient, since it'll put all remaining players
        // into ready queue, and all will sleep at the first while loop in play()
        for(int i=0; i<22; i++) {
            pthread_cond_signal(&can_enter);
        }
        // pthread_cond_broadcast(&can_enter);

        pthread_mutex_unlock(&counters_runs_lock);

        // may think that: printing for each match possible AFTER unlocking using extra vars. capitals, titans...
        // HOWEVER, THIS IS NOT CORRECT
        // e.g., game thread might be descheduled for 2 matches...
    }

    // make everyone join
    for(int i=0; i<num_players; i++) {
        pthread_join(pool[i], NULL);
    }

    // print overall stats
    int num_tied = num_matches-titans_wins-capitals_wins;
    printf("----------------------SUMMARY OF THE DAY----------------------\n");
    printf("Matches Played : %d\n", num_matches);
    printf("Titans   :: Won : %d || Lost : %d || Tied : %d\n", titans_wins, capitals_wins, num_tied);
    printf("Capitals :: Won : %d || Lost : %d || Tied : %d\n", capitals_wins, titans_wins, num_tied);
    printf("Highest Team Score           : %d\n", max_team_score);
    printf("Highest Individual Score     : %d\n", max_individual_score);
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

    pthread_t game_thread;
    if(pthread_create(&game_thread, NULL, &game_fn, (void*)&num_matches)!=0) {
        printf("Game thread creation failed\n");
        exit(1);
    }
    pthread_join(game_thread, NULL);

    if(num_players%22!=0) {
        printf("Match %d could not be played due to lack of players\n", num_matches+1);
    }

    // IMPORTANT:
    // initialize and destroy everything

    pthread_mutex_destroy(&counters_runs_lock);
    pthread_cond_destroy(&match_done);
    pthread_cond_destroy(&can_enter);

}