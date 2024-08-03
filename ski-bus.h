/** AUTHOR
_______________________________

 * Name: Martin Mendl
 * Email: x247581@fit.vutbr.cz
 * Date: 26.4. 2024
 * file: header file for ski-bus.c
_______________________________
*/


#ifndef PROJECT_H
#define PROJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <time.h>

/**
 * Number of skiers.
 */
long L;

/**
 * Number of bus stops.
 */
long Z;

/**
 * Maximum bus capacity.
 */
long K;

/**
 * Max time in microseconds for skier to sleep.
 */
long TL;

/**
 * Max time in microseconds for bus to sleep.
 */
long TB;

/**
 * Maximum amount of log messages to strore.
*/
#define MAX_LOG_MESSAGES 10

/**
 * Max amount of characters in the log message
*/
#define MAX_MESSAGE_LENGTH 30

/**
 * @brief Struct for shared data among processes.
 */
typedef struct {
    int ID; /**< ID of the shared data. */
    int skiers_boarded; /**< Amount of skiers that have boarded the bus combined. If -1, error occurred. */
    int occupancy; /**< The amount of people on the bus. */
    int amount_of_skiers_to_board; /**< Amount of people waiting for the next ride. */
    FILE *out_file; /**< File pointer to store the logs from the program. */
    char log_messages[MAX_LOG_MESSAGES][MAX_MESSAGE_LENGTH]; /**< Array to store log messages. */
    int num_messages; /**< Number of log messages currently stored. */
} shared_data;

/**
 * Output file name.
 */
char out_file_name[] = "ski-bus.out";

/**
 * Pointer to shared memory segment.
 */
shared_data* shared_memory;

/**
 * Array representing the skier amount that are waiting at each stop.
 */
int* skiers_waiting;

/**
 * Array of semaphores for bus stops, accounting for the last stop as well.
 */
sem_t* bus_stops;

/**
 * Semaphore where skiers will wait until the bus finishes the route.
 */
sem_t* bus_stop_sign;

/**
 * Semaphore for accessing shared data.
 */
sem_t* datafor;

/**
 * Semaphore for printing out data.
 */
sem_t* printafor;

/**
 * the string, that will be written to the log file
*/
char log_message[MAX_MESSAGE_LENGTH];

/**
 * @brief Adds a log message to the buffer
 * 
 * @param message The message to be added.
*/
void add_log_message(const char *message);

/**
 * @brief dumps the buffer to the file
 * 
*/
void write_logs_to_file();

/**
 * @brief Initializes the bus stops.
 */
void init_bus_stops();

/**
 * @brief Destroys the bus stops.
 */
void destroy_bus_stops();

/**
 * @brief Initializes the shared memory.
 */
void init_shared_memory();

/**
 * @brief Destroys the shared memory.
 */
void destroy_shared_memory();

/**
 * @brief Waits for my turn to access shared data.
 */
void wait_for_my_turn();

/**
 * @brief Releases my turn to access shared data.
 */
void done_with_my_turn();

/**
 * @brief Sleeps for a random time.
 * 
 * @param max_value The maximum value for the random sleep time.
 */
void random_sleep(int max_value);

/**
 * @brief Function called when the bus starts its journey.
 */
void bus_started();

/**
 * @brief Function called when the bus arrives at a bus stop.
 * 
 * @param idZ The ID of the bus stop.
 */
void bus_arrived(int idZ);

/**
 * @brief Function called when the bus leaves a bus stop.
 * 
 * @param idZ The ID of the bus stop.
 */
void bus_leaving(int idZ);

/**
 * @brief Function called when the bus arrives at the final stop.
 */
void bus_arrived_to_final();

/**
 * @brief Function called when the bus leaves the final stop.
 */
void bus_leaving_final();

/**
 * @brief Function called when the bus finishes its journey.
 */
void bus_finished();

/**
 * @brief Function called when a skier starts skiing.
 * 
 * @param idL The ID of the skier.
 */
void skier_started(int idL);

/**
 * @brief Function called when a skier arrives at a bus stop.
 * 
 * @param idL The ID of the skier.
 * @param idZ The ID of the bus stop.
 */
void skier_arrived(int idL, int idZ);

/**
 * @brief Function called when a skier is boarding the bus.
 * 
 * @param idL The ID of the skier.
 */
void skier_boarding(int idL);

/**
 * @brief Function called when a skier reaches the sky.
 * 
 * @param idL The ID of the skier.
 */
void skier_sky(int idL);

/**
 * @brief Creates processes for skiers.
 */
void create_skiers_processes();

/**
 * @brief Creates a process for the ski bus.
 */
void create_ski_bus_process();

#endif
