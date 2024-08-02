/** AUTHOR
_______________________________

 * Name: Martin Mendl
 * Email: x247581@fit.vutbr.cz
 * Date: 26.4. 2024
 * file: main code src file for ski-bus
_______________________________
*/


#include "ski-bus.h"

/**
 * @brief Initializes the bus stops and the semaphore for the final stop.
*/
void init_bus_stops() {
    bus_stops = mmap(NULL, sizeof(sem_t)*Z, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    bus_stop_sign = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    skiers_waiting = mmap(NULL, sizeof(int)*Z, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    datafor = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    printafor = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);

    // check for errors
    if (
        bus_stops == MAP_FAILED ||
        bus_stop_sign == MAP_FAILED || 
        skiers_waiting == MAP_FAILED || 
        datafor == MAP_FAILED ||
        printafor == MAP_FAILED
        ) {

        perror("mapping of semaphores failed!\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the semaphores for the bus stops
    for (int i = 0; i < Z; i++) {
        if (sem_init(&bus_stops[i], 1, 1) < 0) {
            perror("sem_init failed");
            exit(EXIT_FAILURE);
        }
    }

    // Initialize the semaphore for the bus stop sign
    if (sem_init(bus_stop_sign, 1, 1) < 0) { 
        perror("faild to init bus_stop_sign");
        exit(EXIT_FAILURE);
    }

    // Initialize the semaphore for the shared data
    if (sem_init(datafor, 1, 1) < 0){
        perror("faild to inif datafor");
        exit(EXIT_FAILURE);
    }

    // Initialize the semaphore for the print
    if (sem_init(printafor, 1, 1) < 0){
        perror("faild to inif printafor");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Destroys the bus stops and the semaphore for the final stop.
*/
void destroy_bus_stops() {
    for (int i = 0; i < Z; i++) {
        if (sem_destroy(&bus_stops[i]) < 0) {
            perror("sem_destroy");
            exit(EXIT_FAILURE);
        }
    }
    // Unmap the memory region
    if (munmap(bus_stops, sizeof(sem_t)*Z) < 0) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    // Unmap the semaphore, for skier waiting for the bus to finish
    if (munmap(bus_stop_sign ,sizeof(sem_t)) < 0) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    if (munmap(datafor, sizeof(sem_t)) < 0) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    if (munmap(printafor, sizeof(sem_t)) < 0) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Initializes the shared memory.
*/
void init_shared_memory() {

    shared_memory = mmap(NULL, sizeof(shared_data), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);

    if (shared_memory == MAP_FAILED) {
        perror("mapping shared data failed\n");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }

    shared_memory->skiers_boarded = 0;
    shared_memory->occupancy = 0;
    shared_memory->amount_of_skiers_to_board = 0;
    shared_memory->out_file = fopen(out_file_name, "w"); // Open the file for writing
}

/**
 * @brief Waits for the shared data to be available.
*/
void wait_for_my_turn() {

    if (sem_wait(datafor) < 0) {
        perror("datafor faild to load\n");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Frees the shared data.
*/
void done_with_my_turn() {
    if (sem_post(datafor) < 0) {
        perror("datafor faild to free\n");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Destroys the shared memory.
*/
void destroy_shared_memory() {
    // Unmap the shared memory region
    if (munmap(shared_memory, sizeof(shared_data)) < 0) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief add a message to the log buffer
 * 
 * @param message The message to be added to the log buffer
*/
void add_log_message(const char *message) {
    if (shared_memory->num_messages < MAX_LOG_MESSAGES) {
        strncpy(shared_memory->log_messages[shared_memory->num_messages], message, MAX_MESSAGE_LENGTH - 1);
        shared_memory->log_messages[shared_memory->num_messages][MAX_MESSAGE_LENGTH - 1] = '\0'; // Ensure null termination
        shared_memory->num_messages++;
    }

    if (shared_memory->num_messages == MAX_LOG_MESSAGES) 
        write_logs_to_file();
}

/**
 * @brief Write the log messages to the output file.
*/
void write_logs_to_file() {
    if (shared_memory->out_file != NULL) {
        for (int i = 0; i < shared_memory->num_messages; i++) {
            fprintf(shared_memory->out_file, "%s\n", shared_memory->log_messages[i]);
            // Clear the message from the buffer
            shared_memory->log_messages[i][0] = '\0';
        }
        fflush(shared_memory->out_file); // Ensure data is written immediately
        shared_memory->num_messages = 0;
    }
}


/**
 * @brief Generates a random sleep time.
*/
void random_sleep(int max_value) {
    srand(getpid() + time(NULL));
    float sleep_time =  rand() % (max_value + 1);
    usleep(sleep_time);
}

/**
 * @brief Prints out the message that the bus has started.
*/
void bus_started() {
    if (sem_wait(printafor) < 0) {
        perror("faild to display bus started message\n");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
    shared_memory->ID++;

    snprintf(log_message, MAX_MESSAGE_LENGTH, "%d: BUS: started", shared_memory->ID);

    printf("%s\n", log_message);
    add_log_message(log_message);

    if (sem_post(printafor) < 0) {
        perror("faild to return from showing bus started message");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Prints out the message that the bus has arrived at a bus stop.
 * 
 * @param idZ The ID of the bus stop.
*/
void bus_arrived(int idZ) {
    if (sem_wait(printafor) < 0) {
        perror("faild to display bus started message\n");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
    shared_memory->ID++;

    snprintf(log_message, MAX_MESSAGE_LENGTH, "%d: BUS: arrived to %d", shared_memory->ID, idZ);

    printf("%s\n", log_message);
    add_log_message(log_message);

    if (sem_post(printafor) < 0) {
        perror("faild to return from showing bus started message");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Prints out the message that the bus has left a bus stop.
 * 
 * @param idZ The ID of the bus stop.
*/
void bus_leaving(int idZ) {
    if (sem_wait(printafor) < 0) {
        perror("faild to display bus started message\n");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
    shared_memory->ID++;

    snprintf(log_message, MAX_MESSAGE_LENGTH, "%d: BUS: leaving %d", shared_memory->ID, idZ);

    printf("%s\n", log_message);
    add_log_message(log_message);

    if (sem_post(printafor) < 0) {
        perror("faild to return from showing bus started message");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Prints out the message that the bus has arrived at the final bus stop.
*/
void bus_arrived_to_final() {
    if (sem_wait(printafor) < 0) {
        perror("faild to display bus started message\n");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
    shared_memory->ID++;

    snprintf(log_message, MAX_MESSAGE_LENGTH, "%d: BUS: arrived to final", shared_memory->ID);

    printf("%s\n", log_message);
    add_log_message(log_message);

    if (sem_post(printafor) < 0) {
        perror("faild to return from showing bus started message");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Prints out the message that the bus has left the final bus stop.
*/
void bus_leaving_final() {
    if (sem_wait(printafor) < 0) {
        perror("faild to display bus started message\n");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
    shared_memory->ID++;

    snprintf(log_message, MAX_MESSAGE_LENGTH, "%d: BUS: leaving final", shared_memory->ID);

    printf("%s\n", log_message);
    add_log_message(log_message);

    if (sem_post(printafor) < 0) {
        perror("faild to return from showing bus started message");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Prints out the message that the bus has finished its journey.
*/
void bus_finished() {
    if (sem_wait(printafor) < 0) {
        perror("faild to display bus started message\n");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
    shared_memory->ID++;

    snprintf(log_message, MAX_MESSAGE_LENGTH, "%d: BUS: finish", shared_memory->ID);
    
    printf("%s\n", log_message);    
    add_log_message(log_message);

    if (sem_post(printafor) < 0) {
        perror("faild to return from showing bus started message");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Prints out the message that the skier has started.
 * 
 * @param idL The ID of the skier.
*/
void skier_started(int idL) {
    if (sem_wait(printafor) < 0) {
        perror("faild to display bus started message\n");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
    shared_memory->ID++;

    snprintf(log_message, MAX_MESSAGE_LENGTH, "%d: L %d: started", shared_memory->ID, idL);

    printf("%s\n", log_message);
    add_log_message(log_message);

    if (sem_post(printafor) < 0) {
        perror("faild to return from showing bus started message");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Prints out the message that the skier has arrived at a bus stop.
 * 
 * @param idL The ID of the skier.
 * @param idZ The ID of the bus stop.
*/
void skier_arrived(int idL, int idZ) {
    if (sem_wait(printafor) < 0) {
        perror("faild to display bus started message\n");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
    shared_memory->ID++;

    snprintf(log_message, MAX_MESSAGE_LENGTH, "%d: L %d: arrived to %d", shared_memory->ID, idL, idZ);
    
    printf("%s\n", log_message);
    add_log_message(log_message);

    if (sem_post(printafor) < 0) {
        perror("faild to return from showing bus started message");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Prints out the message that the skier is boarding the bus.
 * 
 * @param idL The ID of the skier.
*/
void skier_boarding(int idL) {
    if (sem_wait(printafor) < 0) {
        perror("faild to display bus started message\n");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
    shared_memory->ID++;

    snprintf(log_message, MAX_MESSAGE_LENGTH, "%d: L %d: boarding", shared_memory->ID, idL);

    printf("%s\n", log_message);    
    add_log_message(log_message);

    if (sem_post(printafor) < 0) {
        perror("faild to return from showing bus started message");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Prints out the message that the skier has reached the sky.
 * 
 * @param idL The ID of the skier.
*/
void skier_sky(int idL) {
    if (sem_wait(printafor) < 0) {
        perror("printafor to display bus started message\n");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
    shared_memory->ID++;

    snprintf(log_message, MAX_MESSAGE_LENGTH, "%d: L %d: going to ski", shared_memory->ID, idL);

    printf("%s\n", log_message);
    add_log_message(log_message);

    if (sem_post(printafor) < 0) {
        perror("faild to return from showing bus started message");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Creates the skier processes.
*/
void create_skiers_processes() {

    int id, data_buffer;

    for (int idL = 0; idL < L; idL++) {

        id = fork();

        if (id < 0) {
            shared_memory->skiers_boarded = -1;        
            // let other skiers now, that they should quit as well   
            destroy_bus_stops();
            destroy_shared_memory();
            perror("failed to create a skiier\n");
            exit(EXIT_FAILURE);
        } else if (id == 0) {

            // select a ranodm destion the skier has to go to
            srand(getpid() + time(NULL)); // seed the random number generator
            int skier_destionation = rand() % (Z-1); // generate a random number in interval <0, Z-1>

            skier_started(idL+1);

            // wait for the skier to reach the destination
            random_sleep(TL);

            skier_arrived(idL+1, skier_destionation+1);
            
            // take note of how many skiers are present at each bus stop
            wait_for_my_turn();
            skiers_waiting[skier_destionation]++;
            done_with_my_turn();

            // skier need to wait for the bus stop to become available
            if (sem_wait(&bus_stops[skier_destionation]) < 0) {
                perror("skier faild to shop up to the bus stop\n");
                destroy_bus_stops();
                destroy_shared_memory();
                exit(EXIT_FAILURE);
            }

            // board the bus
            skier_boarding(idL+1);

            wait_for_my_turn();
            shared_memory->occupancy++;
            shared_memory->skiers_boarded++;
            shared_memory->amount_of_skiers_to_board--;
            skiers_waiting[skier_destionation]--;
            data_buffer = shared_memory->amount_of_skiers_to_board;
            done_with_my_turn();

            // I am the last one to board
            if (data_buffer == 0) {
                // tell the bus to leave
                if (sem_post(bus_stop_sign) < 0) {
                    perror("bus faild to leave the bus station\n");
                    destroy_bus_stops();
                    destroy_shared_memory();
                    exit(EXIT_FAILURE);
                }
            }

            // wait for the final stop
            if (sem_wait(&bus_stops[Z-1]) < 0) {
                perror("skier fiald to get of the bus at the final stop\n");
                destroy_bus_stops();
                destroy_shared_memory();
                exit(EXIT_FAILURE);
            }

            skier_sky(idL+1);
            
            // leave the bus
            wait_for_my_turn();
            shared_memory->occupancy--;
            data_buffer = shared_memory->occupancy;
            done_with_my_turn();

            // I am the last sub process
            if (data_buffer == 0) {

                // tell the bus to leave
                if (sem_post(bus_stop_sign) < 0) {
                    perror("bus faild to leave the bus station\n");
                    destroy_bus_stops();
                    destroy_shared_memory();
                    exit(EXIT_FAILURE);
                }
            }

            exit(EXIT_SUCCESS);
        }
    }
}

/**
 * @brief Creates the ski bus process.
*/
void craete_ski_bus_process() {

    int id = fork();

    if (id < 0) {
        perror("failed to create ski bus process!\n");
        destroy_bus_stops();
        destroy_shared_memory();
        exit(EXIT_FAILURE);
    } else if (id == 0) {
        
        // making all the bus stops unawailable
        for (int i = 0; i < Z; i++) {
            if (sem_wait(&bus_stops[i]) < 0) {  
                perror("failed to aquarie a bus stop\n");
                destroy_bus_stops();
                destroy_shared_memory();
                exit(EXIT_FAILURE);
            }
        }

        // create skiner processes
        create_skiers_processes();

        // move to the first bus stop
        int amount_of_skiers_to_board, available_space;

        bus_started();

        while (1) {
            
            for (int idZ = 0; idZ < Z-1; idZ++) {

                // travel to bus stop
                random_sleep(TB);
                bus_arrived(idZ+1);

                wait_for_my_turn();
                // Calculate the available space on the bus
                available_space = K - shared_memory->occupancy;

                // amount of peopole that will board the bus
                amount_of_skiers_to_board =  skiers_waiting[idZ] >= available_space ? available_space : skiers_waiting[idZ];

                // tell the skiers, how many can baord the bus
                shared_memory->amount_of_skiers_to_board = amount_of_skiers_to_board;
                done_with_my_turn();

                // the value of the semaphore is -1 at this point
                if (amount_of_skiers_to_board > 0) {
                    
                    // make the bus stop sign active
                    if (sem_wait(bus_stop_sign) < 0) {
                        perror("Bus faild to wait for skiers to baord\n");
                        destroy_bus_stops();
                        destroy_shared_memory();
                        exit(EXIT_FAILURE);
                    }

                    // allow n amount of passages to board                
                    for (int j = 0; j < amount_of_skiers_to_board; j++) {
                        if (sem_post(&bus_stops[idZ]) < 0) {
                            perror("faild to make space on the bus!\n");
                            destroy_bus_stops();
                            destroy_shared_memory();
                            exit(EXIT_FAILURE);
                        }
                    }

                    // the only way this will go throw, is when the last skiour would free the bus_stop_sign
                    // this means, that the last skier has boarded
                    if (sem_wait(bus_stop_sign) < 0) {
                        perror("Bus faild to wait for skiers to baord\n");
                        destroy_bus_stops();
                        destroy_shared_memory();
                        exit(EXIT_FAILURE);
                    }

                    // free the stop sign
                    if (sem_post(bus_stop_sign) < 0) {
                        perror("bus faild to leave the bus station\n");
                        destroy_bus_stops();
                        destroy_shared_memory();
                        exit(EXIT_FAILURE);
                    }
                }

                bus_leaving(idZ+1);
            }

            // travel to final bus stop
            random_sleep(TB);

            bus_arrived_to_final();

            wait_for_my_turn();
            int amount_of_pasagers = shared_memory->occupancy;
            done_with_my_turn();

            if ( amount_of_pasagers > 0) {
                
                // make the bus stop sign active
                if (sem_wait(bus_stop_sign) < 0) {
                    perror("Bus faild to wait for skiers to baord\n");
                    destroy_bus_stops();
                    destroy_shared_memory();
                    exit(EXIT_FAILURE);
                }

                // allow n amount of passages to board                
                for (int j = 0; j < amount_of_pasagers; j++) {
                    if (sem_post(&bus_stops[Z-1]) < 0) {
                        perror("faild to make space on the bus!\n");
                        destroy_bus_stops();
                        destroy_shared_memory();
                        exit(EXIT_FAILURE);
                    }
                }

                // the only way this will go throw, is when the last skiour would free the bus_stop_sign
                // this means, that the last skier has left
                if (sem_wait(bus_stop_sign) < 0) {
                    perror("Bus faild to wait for skiers to baord\n");
                    destroy_bus_stops();
                    destroy_shared_memory();
                    exit(EXIT_FAILURE);
                }

                // free the stop sign
                if (sem_post(bus_stop_sign) < 0) {
                    perror("bus faild to leave the bus station\n");
                    destroy_bus_stops();
                    destroy_shared_memory();
                    exit(EXIT_FAILURE);
                }
            }  

            bus_leaving_final();

            // if all the skiers have boarded, exit
            if (shared_memory->skiers_boarded == L) {
                bus_finished();
                exit(EXIT_SUCCESS);
            }
        }
    }
}

int main(int argc, char *argv[]) {

    // chekc for amoutn of arguments
    if (argc != 6) {
        printf("Invalid number of arguments!\n");
        printf("Usage: ./proj2 L Z K TL TB\n");
        return 1;
    }
    
    // check for the validity of the arguments
    char *endptr;
    L = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || L < 0 || L >= 20000) {
        printf("Invalid value for L!\n");
        return 1;
    }
    
    Z = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0' || Z <= 0 || Z > 10) {
        printf("Invalid value for Z!\n");
        return 1;
    }
    
    K = strtol(argv[3], &endptr, 10);
    if (*endptr != '\0' || K < 10 || K > 100) {
        printf("Invalid value for K!\n");
        return 1;
    }
    
    TL = strtol(argv[4], &endptr, 10);
    if (*endptr != '\0' || TL > 10000) {
        printf("Invalid value for TL!\n");
        return 1;
    }
    
    TB = strtol(argv[5], &endptr, 10);
    if (*endptr != '\0' || TB > 1000) {
        printf("Invalid value for TB!\n");
        return 1;
    }

    init_bus_stops();
    init_shared_memory();
    craete_ski_bus_process();

    // wait for all the processes to finish
    for (int i = 0; i < L + 2; i++) {
        wait(NULL);
    }

    // clear the buffer
    if (shared_memory->num_messages > 0) 
        write_logs_to_file();
        
    destroy_bus_stops();
    destroy_shared_memory();
    
    return 0;
}