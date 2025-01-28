/* Mia Patrikios
   Jantz CS360 Chain Healing Lab 1
   Summary: Urgosa keeps other players alive with his Chain Heal spell. Chain Heal can restore
   PP up to the player's max PP. After one player is healed, the spell jumps to another player within range. 
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>  /* If you include this, you need to compile with -lm */


/* Node for each person */
typedef struct person
{
  char *name;
  int x, y;
  int cur_PP, max_PP;
  struct person *prev;
  int adj_size; /* players within range*/
  struct person **adj;
  _Bool visited;
  int healing; // maintain players healing during DFS call
} Person;

typedef struct {
    int num_jumps;
    double power_reduction;
    int initial_power;
    int best_healing;
    int best_path_length;
    Person **best_path; // store nodes in best_path everytime a new one is found
    int *healing; // store corresponding healing in this array for each player
} GlobalInfo;


/* Returns the distance between two players 
   I knew I'd use Pythagorean theorm one day */
double distance (Person *p1, Person *p2) {
    int dx = p1->x - p2->x;
    int dy = p1->y - p2->y;
    return sqrt(dx*dx + dy*dy);
} 

void DFS(Person *player, int hop_number, GlobalInfo *globals, int total_healing, Person *from) {
    //healing = initial spell * (1 - power reduction)^(hop_number) [ (hop_number - 1)?? ]
    int healing = rint(globals->initial_power * pow(1-globals->power_reduction, hop_number-1));
    // printf("Healing: %d\n", healing);
    int original_PP = player->cur_PP; 


    if (healing < player->max_PP - player->cur_PP) {
        // printf("IF Player: %s, difference PP: %d\n", player->name, (player->max_PP - player->cur_PP));
        total_healing += healing;
        player->cur_PP += healing;
        player->healing = healing;
        player->prev = from; 
    }
    else {
        // printf("ELSE Player: %s, difference PP: %d\n", player->name, (player->max_PP - player->cur_PP));
        total_healing += player->max_PP - player->cur_PP;
        player->cur_PP = player->max_PP;
        player->healing = player->max_PP - player->cur_PP;
        player->prev = from;
    }

    /* base case */ 
    if (hop_number == globals->num_jumps) {
        if (total_healing > globals->best_healing) {
            globals->best_healing = total_healing; //set new best_healing

            for (int i = globals->num_jumps - 1; i >= 0 && player != NULL; i--) {
            globals->best_path[i] = player; 
            globals->healing[i] = player->healing;

            if (player->prev != NULL) {
                // printf("Player prev: %s\n", player->prev->name);
                player = player->prev;
            } else {
                // printf("Player prev: NULL\n");
            }

        }
        }
        player->cur_PP = original_PP;  // Wasn't resetting PP to orig value!!!!
        return;
    }

    /* recursive function */
    player->visited = 1;
    for (int i = 0; i < player->adj_size; i++) {
        if (player->adj[i]->visited == 0) {
            DFS(player->adj[i], hop_number+1, globals, total_healing, player);
        }
    }
    
    player->cur_PP = original_PP;
    player->visited = 0;
    player->prev = from;
    // need to like unvisit or something what they said in lab so that you can enumerate all paths
}

int main(int argc, char **argv) {
   /* Read in input, don't need to error check per write-up */
    GlobalInfo *globals = malloc(sizeof(GlobalInfo));
    int initial_range = atoi(argv[1]); /* Urgosa's spell casting range */
    int jump_range = atoi(argv[2]); /* range Chain Heal can jump*/
    globals->num_jumps = atoi(argv[3]);  /* Number of jumps the spell can make */
    globals->initial_power = atoi(argv[4]); /* Amount of healing to initial target*/
    globals->power_reduction = atof(argv[5]); /* Healing to subsequent targets is reduced by this every jump*/
    globals->best_healing = 0;
    globals->best_path = (Person **)malloc(globals->num_jumps * sizeof(Person *)); 
    globals->healing = (int *)malloc(globals->num_jumps * sizeof(int)); 

    /* stdin format: x-coordiante, y-coordinate, current pp points, max pp points, player name*/
    int x, y, cur_PP, max_PP;
    char name[100];
    int num_players = 0; // Keep track of number of players for array size
    Person *prev_person = NULL; // Keep track of previous person


    /* Read in the first person */
    scanf("%d %d %d %d %s", &x, &y, &cur_PP, &max_PP, name);
        Person *new_person = (Person *)malloc(sizeof(Person));
        new_person->x = x;
        new_person->y = y;
        new_person->cur_PP = cur_PP;
        new_person->max_PP = max_PP;
        new_person->healing = 0;
        new_person->name = (char *)malloc(strlen(name) + 1); /* Note to self: https://stackoverflow.com/questions/36075558/using-scanf-to-read-strings-into-an-array-of-strings */
        strcpy(new_person->name, name); 

        new_person->prev = NULL;
        prev_person = new_person;
        new_person->visited = 0;
        num_players++;

    while (scanf("%d %d %d %d %s", &x, &y, &cur_PP, &max_PP, name) == 5) {
        /* Use malloc for each new node */
        Person *new_person = (Person *)malloc(sizeof(Person));
        new_person->x = x;
        new_person->y = y;
        new_person->cur_PP = cur_PP;
        new_person->max_PP = max_PP;
        new_person->healing = 0;
        new_person->name = (char *)malloc(strlen(name) + 1); 
        strcpy(new_person->name, name);   

        new_person->prev = prev_person;
        prev_person = new_person;
        new_person->visited = 0;
        num_players++;
    }

    /* Create an array of node pointers and assign nodes by traversing the linked nodes 
       The size of the arrya is the number of nodes (keep track of that)*/
     
    /* Note for self:
        The double asterick is a pointer to a pointer (an array of Person pointers). The ** points to the start of 
        an array of pointers. First, dynamically allocate an array. Then, each element of the array needs to point to a Person struct.
        Kind of like how a table of contents "points" to certain pages in the book */
     Person **player_array = (Person **)malloc(num_players * sizeof(Person *));
     Person *current_person = prev_person; /* Last person added */

    /* Traverse the linked list backwards */
    for (int i = num_players -1; i >= 0 ; i--) {
         player_array[i] = current_person;
         current_person = current_person->prev;
     }

    
    /* Create adjacency list for each player (can prob make into a function after the fact)
    1. Calculate size of adjacency list for each node
    2. Allocate adjaceny list for each node. 
    An array of node pointers, should be exact size of the list.
    3. Add nodes onto their adjacency list
    */

    /* Step 1 = Size of adjaceny list*/

    for (int i = 0; i < num_players; i++) {
        player_array[i]->adj_size = 0;
        for (int j = 0; j < num_players; j++) {
            if (i != j) { /* Not itself */
                double dist = distance(player_array[i], player_array[j]);
                if (dist <= jump_range) {
                    player_array[i]->adj_size++;
                }
            }
        }
    }

    /* Step 2 = Allocate adjacency list */
    for (int i = 0; i < num_players; i++) {
        /* If no connections */
        if (player_array[i]->adj_size == 0) {
            player_array[i]->adj = NULL;
        }
        else {
        /*  Person ** : cast to a Person pointer because we defined a list of person's in the struct */ 
        player_array[i]->adj = (Person **)malloc(player_array[i]->adj_size * sizeof(Person *));
        }
    }
    
    /* Step 3 = Add nodes to adjacency list */

    for (int i = 0; i < num_players; i++) {
        int index = 0;
        for (int j = 0; j < num_players; j++) {
            if (i != j) { /* Not itself */
                double dist = distance(player_array[i], player_array[j]);
                if (dist <= jump_range) {
                    player_array[i]->adj[index] = player_array[j];
                    index++;
                }
            }
        }
    }

   /*check adjaceny list*/
//    for (int i = 0; i < num_players; i++) {
//        printf("%s: ", player_array[i]->name);
//        for (int j = 0; j < player_array[i]->adj_size; j++) {
//            printf("%s ", player_array[i]->adj[j]->name);
//        }
//        printf("\n\n");
//    }
    
     /* DFS can only start for people who are within the initial range of Urgosa */
     /* Should still work if urgosa technically cant reach anyone from the initial jump but if another player can reach him in their jump range. How should I do this? */

    for (int i = 0; i < num_players; i++) {
        double dist = distance(player_array[0], player_array[i]);
        if (dist <= initial_range) {
            DFS(player_array[i], 1, globals, 0, NULL);  // Start with total_healing = 0
        }
    }   

    for (int i = 0; i < globals->num_jumps; i++) {
        printf("%s %d\n", globals->best_path[i]->name, globals->healing[i]);
    }

    printf("Total healing: %d\n", globals->best_healing);  // Print once at the end 

    for (int i = 0; i < num_players; i++) {
        if (player_array[i]->adj != NULL) {
            free(player_array[i]->adj);
        }
        free(player_array[i]->name);
        free(player_array[i]);
    }
    free(player_array);

    free(globals->best_path);
    free(globals->healing);
    free(globals);


    return 0;

}
