// Amit Nayak Summative Project, June 2 2017
//TRON: Reborn
//This game is modeled after the original TRON game where each user controls a 'light bike.
//The objective of the game is to cause the other user to crash into the wall or a 'light trail'.
//Allegro 5 game library was used for this project. eg: al_... or ALLEGRO_...
//Title screen shows the instructions.

#include <stdio.h> 
#include <math.h> 
#include <allegro5/allegro.h> 
#include <allegro5/allegro_image.h>

//Initializing constants.
const float FPS = 120;
const int SCREEN_W = 1000;
const int SCREEN_H = 1000;
const int BOUNCER_SIZE = 32;
const int BOUNCER_SIZE2 = 32;
const int GRID_SIZE = 32;
const int PLAYER_SPEED = 2;
const int MAX_BLOCKS = 1000;
const int cd = 32 / PLAYER_SPEED;

//Declaring array of keys for Allegro 5.
enum MYKEYS {
    KEY_LEFT,
    KEY_RIGHT,
    KEY_A,
    KEY_D,
    KEY_SPACE
};

//Struct for player coordinates, directional values and cooldowns.
struct player {
    int xdir; //direction of travel along x axis position. -1, 0, 1.
    int ydir; //direction of travel along y axis position. -1, 0, 1.
    int dir; //direction of travel. (0-3) : up, right, down, left.
    int x; //x coordinate of current position.
    int y; //y coordinate of current position.
    int blockCooldown; //Block drawing cooldown. 
    int blockCounter; //Block drawing counter.
};

//struct for light trail blocks coordinates, activity and draw value.
struct trailBlock {
    int x; //x coordinate of current position. 
    int y; //y coordinate of current position. 
    bool isActive; //boolean if block is active or not.
    bool draw; //boolean on whether to draw or not.
};

//Function Prototypes.
void setDir(player &player);
void collDetect(bool &collision, player &player);
void collDetectWall(bool &collision, player &player);
void init_trails();
void init_player1();
void init_player2();
int lightcycle_Spawn1();
int lightcycle_Spawn2();

//Global Variables, Based off existing structs.
//Player Structs for each player.
//Player 1 is Player Blue and Player 2 is Player Red.
player player1;
player player2;

//Declaring an array of 'trail blocks' for each player.
//p1Trail is for Player Blue and p2Trail is for Player Red.
trailBlock p1Trail[MAX_BLOCKS];
trailBlock p2Trail[MAX_BLOCKS];

int main(int argc, char * * argv) {
    //file pointer.
    FILE * fptr;

    //Allegro display, timer, bitmap, key, bouncer size, and other variable initialization.
    ALLEGRO_DISPLAY * display = NULL;
    ALLEGRO_EVENT_QUEUE * event_queue = NULL;
    ALLEGRO_TIMER * timer = NULL;
    ALLEGRO_BITMAP * bouncer = NULL;
    ALLEGRO_BITMAP * bouncer2 = NULL;
    ALLEGRO_BITMAP * TRAIL1 = NULL;
    ALLEGRO_BITMAP * TRAIL2 = NULL;

    bool redraw = true;
    bool doexit = false;
    bool gamestart = false; //boolean to start the game.

    //Strings for printing to console and match history.
    char redwin[20] = "Player Red Wins! ";
    char bluwin[20] = "Player Blue Wins! ";

    //series of if statements if allegro init, keyboard install, and timer does not work.
    if (!al_init()) {
        fprintf(stderr, "failed to initialize allegro!\n");
        return -1;
    }

    if (!al_install_keyboard()) {
        fprintf(stderr, "failed to initialize the keyboard!\n");
        return -1;
    }

    //initializing timer.
    timer = al_create_timer(1.0 / FPS);
    if (!timer) {
        fprintf(stderr, "failed to create timer!\n");
        return -1;
    }

    //Allegro image addon.
    al_init_image_addon();

    //Intializing my bitmaps by linking them to my .png files.
    bouncer = al_load_bitmap("player_red.png"); //player red
    bouncer2 = al_load_bitmap("player_blue.png"); //player blue
    TRAIL1 = al_load_bitmap("player_red_trail.png"); //trail of red player
    TRAIL2 = al_load_bitmap("player_blue_trail.png"); //trail of blue player
    ALLEGRO_BITMAP * map = al_load_bitmap("map_game.png"); //map
    ALLEGRO_BITMAP * title = al_load_bitmap("title_screen.png"); //title screen
    ALLEGRO_BITMAP * bluewinner = al_load_bitmap("blue_wins.png"); //screen when blue wins
    ALLEGRO_BITMAP * redwinner = al_load_bitmap("red_wins.png"); //screen when red wins

    //Series of if statements if 'bouncer' (lightcycle bitmaps), display or event queue fail.
    if (!bouncer) {
        fprintf(stderr, "failed to create bouncer bitmap!\n");
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }

    //Creating display.
    display = al_create_display(SCREEN_W, SCREEN_H);

    if (!display) {
        fprintf(stderr, "failed to create display!\n");
        al_destroy_timer(timer);
        return -1;
    }

    al_clear_to_color(al_map_rgb(255, 0, 255));

    event_queue = al_create_event_queue();

    if (!event_queue) {
        fprintf(stderr, "failed to create event_queue!\n");
        al_destroy_bitmap(bouncer);
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }

    //Registering event sources for allegro and starting display.
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();
    al_start_timer(timer);

    //Title screen load in.
    if (!gamestart) {
        fflush(stdin);
        al_draw_bitmap(title, 0, 0, NULL);
        al_flip_display();
        printf("Press space to start...\n");
        ALLEGRO_EVENT ev;
        while (true) {
            al_wait_for_event(event_queue, & ev); //waits for user to press the space bar to start.
            if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
                if (ev.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                    break;
                }
            }
            if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { //closes the program.
                return 0;
            }
        }

    }

    gamestart = true;

    while (gamestart) {
        // Reset the game
        //Initializing variables for each of the  player structs.
        //make into function.
        //Player 1 begins at the bottom and moves upward initially.
        init_player1();
        //Player 2 begins at the top and moves downward initially.
        init_player2();

        //Initialize light trails for each player.
        init_trails();

        //Let the fun begin.
        doexit = false;
        while (!doexit) {
            ALLEGRO_EVENT ev;
            al_wait_for_event(event_queue, & ev);
            if (ev.type == ALLEGRO_EVENT_TIMER) {

                // Calling functions that assign directions based on key presses.
                setDir(player1);
                setDir(player2);

                //initalizing booleans for which player one.
                bool bluecollision = false;
                bool redcollision = false;

                //calling collision detection functions.
                collDetect(bluecollision, player1);
                collDetect(redcollision, player2);

                //when player blue collides with something player red wins.
                //prompts user to either play again or quit the game.
                if (bluecollision) {
                    printf("\n%s\n", redwin); //prints red won on console.
                    fptr = fopen("match_history.txt", "a");
                    fprintf(fptr, "%s\n", redwin); //prints red won on match history text file.
                    fclose(fptr);
                    al_draw_bitmap(redwinner, 0, 0, NULL); //displays red victory bitmap on screen.
                    al_flip_display();
                    ALLEGRO_EVENT ev;
                    while (true) {
                        al_wait_for_event(event_queue, & ev); //wait for user to press space to play again.
                        if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
                            if (ev.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                                doexit = true;
                                break;
                            }
                        }
                        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { //closes the program
                            doexit = true;
                            gamestart = false;
                            return 0;
                        }
                    }
                }
                //when player red collides with something player blue wins.
                //prompts user to either play again or quit the game.
                if (redcollision) {
                    printf("\n%s\n", bluwin);
                    fptr = fopen("match_history.txt", "a");
                    fprintf(fptr, "%s\n", bluwin);
                    fclose(fptr);
                    al_draw_bitmap(bluewinner, 0, 0, NULL);
                    al_flip_display();
                    ALLEGRO_EVENT ev;
                    while (true) {
                        al_wait_for_event(event_queue, & ev);
                        if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
                            if (ev.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                                doexit = true;
                                break;
                            }
                        }
                        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                            doexit = true;
                            gamestart = false;
                            return 0;
                        }
                    }

                }

                //Calling functions that spawn light trails for each player.
                lightcycle_Spawn1();
                lightcycle_Spawn2();

                redraw = true;

            } else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                return 0;
            } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
                switch (ev.keyboard.keycode) {
                case ALLEGRO_KEY_LEFT: //Turns player blue 90 degrees left from their current direction.  (decrementation)  
                    if (player1.dir >= 1) {
                        player1.dir--;

                    } else {
                        player1.dir = 3; //Sets the user to right direction.
                    }
                    break;

                case ALLEGRO_KEY_RIGHT: //Turns player blue 90 degrees right from their current direction (incrementation) 
                    if (player1.dir <= 2) {
                        player1.dir++;
                    } else {
                        player1.dir = 0; //sets the user to up direction
                    }
                    break;

                case ALLEGRO_KEY_D: //Turns player blue 90 degrees left from their current direction.  (decrementation)
                    if (player2.dir >= 1) {
                        player2.dir--;

                    } else {
                        player2.dir = 3; //Sets the user to right direction.
                    }
                    break;

                case ALLEGRO_KEY_A: //Turns player red 90 degrees right from their current direction (incrementation) 
                    if (player2.dir <= 2) {
                        player2.dir++;
                    } else {
                        player2.dir = 0; //sets the user to up direction
                    }
                    break;
                }
            }
            //Drawing Map, TRONS, and TRAILS in Allegro
            if (redraw && al_is_event_queue_empty(event_queue)) {

                redraw = false;

                al_clear_to_color(al_map_rgb(0, 0, 0));

                //drawing map.
                al_draw_bitmap(map, 0, 0, NULL);

                //Drawing light trails.
                for (int i = 0; i < MAX_BLOCKS; i++) {

                    if (p1Trail[i].draw) {
                        al_draw_bitmap(TRAIL2, p1Trail[i].x, p1Trail[i].y, NULL); //blue player
                    }
                    if (p2Trail[i].draw) {
                        al_draw_bitmap(TRAIL1, p2Trail[i].x, p2Trail[i].y, NULL); //red player
                    }
                }

                //Drawing light bikes.
                al_draw_bitmap(bouncer2, player1.x, player1.y, 0); //Blue player

                al_draw_bitmap(bouncer, player2.x, player2.y, 0); //Red player

                al_flip_display();
            }
        }
    }
    //series of allegro 5 commands that destroy bitmap, timer, display, and event queue.
    al_destroy_bitmap(bouncer);
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    return 0;
}

//Function that sets the player direction based on key presses.
void setDir(player &player) {
    switch (player.dir) {
    case 0: //UP direction
        player.xdir = 0;
        player.ydir = -1;
        break;
    case 1: //Right Direction
        player.xdir = 1;
        player.ydir = 0;
        break;
    case 2: //Down Direction
        player.xdir = 0;
        player.ydir = 1;
        break;
    case 3: //Left Direction
        player.xdir = -1;
        player.ydir = 0;
        break;
    }
}

//Function that spawns the light trail based on cooldown for player 1.
int lightcycle_Spawn1() {
    player1.x += PLAYER_SPEED * player1.xdir; //moves the player's coordinate based on x-axis direction.
    player1.y += PLAYER_SPEED * player1.ydir; //moves the player's coordinate based on y-axis direction.
    player1.blockCooldown++;

    if (player1.blockCooldown > cd) {
        player1.blockCooldown = 0;
        p1Trail[player1.blockCounter].draw = true; //sets the boolean to draw the trail to true.
        if (player1.blockCounter - 1 >= 0) {
            p1Trail[player1.blockCounter - 1].isActive = true; //sets the trail behind the player to active.
        }
        //Spawns the trail just behind the player.
        p1Trail[player1.blockCounter].x = player1.x - player1.xdir * 13;
        p1Trail[player1.blockCounter].y = player1.y - player1.ydir * 13;
        player1.blockCounter++;
    }
}

//Function that spawns the light bike and trail based on cooldown for player 1.
int lightcycle_Spawn2() {

    player2.x += PLAYER_SPEED * player2.xdir; //moves the player's coordinate based on x-axis direction.
    player2.y += PLAYER_SPEED * player2.ydir; //moves the player's coordinate based on y-axis direction.
    player2.blockCooldown++;

    if (player2.blockCooldown > cd) {
        player2.blockCooldown = 0;
        p2Trail[player2.blockCounter].draw = true; //sets the boolean to draw the trail to true.
        if (player2.blockCounter - 1 >= 0) {
            p2Trail[player2.blockCounter - 1].isActive = true; //sets the trail behind the player to active.
        }
        //Spawns the trail just behind the player.
        p2Trail[player2.blockCounter].x = player2.x - player2.xdir * 13;
        p2Trail[player2.blockCounter].y = player2.y - player2.ydir * 13;
        player2.blockCounter++;
    }
}

//Function that checks for collision detection for player blue.
//If a collision is detected it will set the boolean 'bluecollision' to true.
void collDetect(bool &collision, player &player) {
        int nextx;
        int nexty;
        int xcenter;
        bool xcollision;

        for (int i = 0; i < MAX_BLOCKS; i++) {
            // Check collision player 2's trail.
            //checking if x coordinate of the player is in the x coordinate of player 2's trail.
            if (p1Trail[i].isActive || p2Trail[i].isActive) {
                nextx = player.x + PLAYER_SPEED * player.xdir;
                xcenter = nextx + GRID_SIZE / 2;
                if (xcenter >= p2Trail[i].x && xcenter <= p2Trail[i].x + GRID_SIZE) {
                    xcollision = true;
                } else {
                    xcollision = false;
                }

                //The switch statement is for each directional scenario: Up, Right, Down, Left.
                switch (player.dir) {
                case 0:
                case 1:
                    //(xcollision) It's in the same x coordinate.
                    //After it checks if it is in the same y coordinate.
                    if (xcollision) {
                        nexty = player.y + PLAYER_SPEED * player.ydir;
                        if (nexty >= p2Trail[i].y && nexty <= p2Trail[i].y + GRID_SIZE) {
                            collision = true;
                        }
                    }
                    break;

                case 2:
                case 3:
                    //(xcollision) It's in the same x coordinate.
                    //After it checks if it is in the same y coordinate.
                    if (xcollision) {
                        nexty = player.y + PLAYER_SPEED * player.ydir + GRID_SIZE;
                        if (nexty >= p2Trail[i].y && nexty <= p2Trail[i].y + GRID_SIZE) {
                            collision = true;
                        }
                    }
                    break;
                }
            }
            //Checks collision with player 1's trail.
            //checking if x coordinate of the player is in the x coordinate of player 1's trail.
            if (p1Trail[i].isActive || p2Trail[i].isActive) {
                nextx = player.x + PLAYER_SPEED * player.xdir;
                xcenter = nextx + GRID_SIZE / 2;
                if (xcenter >= p1Trail[i].x && xcenter <= p1Trail[i].x + GRID_SIZE) {
                    xcollision = true;
                } else {
                    xcollision = false;
                }

                //The switch statement is for each directional scenario: Up, Right, Down, Left.
                switch (player.dir) {
                case 0:
                case 1:
                    //(xcollision) It's in the same x coordinate.
                    //After it checks if it is in the same y coordinate.
                    if (xcollision) {
                        nexty = player.y + PLAYER_SPEED * player.ydir;
                        if (nexty >= p1Trail[i].y && nexty <= p1Trail[i].y + GRID_SIZE) {
                            collision = true;
                        }
                    }
                    break;

                case 2:
                case 3:
                    if (xcollision) {
                        nexty = player.y + PLAYER_SPEED * player.ydir + GRID_SIZE;
                        if (nexty >= p1Trail[i].y && nexty <= p1Trail[i].y + GRID_SIZE) {
                            collision = true;
                        }
                    }
                    break;
                }

            }

        }
        //Calling wall collision detection function.
        collDetectWall(collision, player);
    }
    //Checks if the player collided into a wall.
void collDetectWall(bool &collision, player &player) {
    if (player.x < 0 || player.x > 1000 || player.y < 0 || player.y > 1000) {
        collision = true;
    }
}

//Initialize both trail boolean values to false for each player.
void init_trails() {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        p1Trail[i].isActive = false;
        p1Trail[i].draw = false;
        p2Trail[i].isActive = false;
        p2Trail[i].draw = false;
    }
}

//initializes player 1's coordinates, starting direction and block cooldowns and counters.
//player 1 (blue) starts on the bottom and moves upwards.
void init_player1() {
    player1.x = SCREEN_W / 2;
    player1.y = SCREEN_H - GRID_SIZE;
    player1.dir = 0;
    player1.blockCooldown = 0;
    player1.blockCounter = 0;
}

//initializes player 2's coordinates, starting direction and block cooldowns and counters.
//player 2 (red) starts on the top and moves downwards.
void init_player2() {
    player2.x = SCREEN_W / 2;
    player2.y = 0;
    player2.dir = 2;
    player2.blockCooldown = 0;
    player2.blockCounter = 0;
}
