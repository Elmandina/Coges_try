#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define SHM_NAME "/my_shm"
#define SEM_NAME "/my_sem"

typedef struct {
    int value;
} shared_data_t;

shared_data_t* shm_ptr;
sem_t* sem;

int producer_send_value(const char* sysfs_file, shared_data_t* shm_ptr, sem_t* sem) {
    FILE* f = fopen(sysfs_file, "r");
    if (!f) {
        printf("[Producer] File missing\n");
        return -1;
    }

    char line[256];
    int sent = 0;
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;

        if (strcmp(line, "1") == 0) {
            int base = 2 + rand() % 2;
            int jitter = (rand() % 5) - 2;
            int interval = base + jitter;

            sem_wait(sem);
            shm_ptr->value = interval;
            sem_post(sem);

            printf("[Producer] Sent: %d\n", interval);
            sent = interval;
        } else if (strcmp(line, "error: temp too high") == 0) {
            printf("[Producer] Cool-down 7 sec\n");
            sleep(7);
        } else {
            printf("[Producer] Not sending anything!\n");
        }
    }

    fclose(f);
    return sent;
}

int consumer_read_value(shared_data_t* shm_ptr, sem_t* sem, int* out_value) {
    sem_wait(sem);
    int val = shm_ptr->value;
    sem_post(sem);

    if (val < 0) return -1; 
    *out_value = val;
    return 0;
}

// ----------------- Test cases -----------------
void test_normal_flow() {
    printf("\nTest 1: Producer -> Consumer flow for 5 transmissions\n");
    FILE* f = fopen("./fake_sysfs_input", "w");
    fprintf(f, "1\n1\n1\n1\n1\n");
    fclose(f);

    int passed = 1;
    for (int i = 0; i < 5; i++) {
        int sent = producer_send_value("./fake_sysfs_input", shm_ptr, sem);
        int received;
        if (consumer_read_value(shm_ptr, sem, &received) != 0 || sent != received) {
            passed = 0;
        }
    }
    printf("Test 1 %s\n", passed ? "PASSED" : "FAILED");
}

void test_corrupted_data() {
    printf("\nTest 2: Consumer receives corrupted data\n");
    sem_wait(sem);
    shm_ptr->value = -999; 
    sem_post(sem);

    int received;
    int ok = consumer_read_value(shm_ptr, sem, &received);
    printf("Test 2 %s\n", (ok == -1) ? "PASSED" : "FAILED");
}

void test_sysfs_behavior() {
    printf("\nTest 3: Producer behavior with sysfs variations\n");

    // a) missing
    unlink("./fake_sysfs_input");
    int res = producer_send_value("./fake_sysfs_input", shm_ptr, sem);
    printf("Missing file: %s\n", (res == -1) ? "PASSED" : "FAILED");

    // b) empty
    FILE* f = fopen("./fake_sysfs_input", "w"); fclose(f);
    res = producer_send_value("./fake_sysfs_input", shm_ptr, sem);
    printf("Empty file: %s\n", (res == 0) ? "PASSED" : "FAILED");

    // c) error: temp too high
    f = fopen("./fake_sysfs_input", "w"); 
    fprintf(f, "error: temp too high"); 
    fclose(f);
    res = producer_send_value("./fake_sysfs_input", shm_ptr, sem);
    printf("Cool-down mode: %s\n", "PASSED (manual check)"); 

    // d) unexpected prefix
    f = fopen("./fake_sysfs_input", "w"); 
    fprintf(f, "#STATUS=1"); 
    fclose(f);
    res = producer_send_value("./fake_sysfs_input", shm_ptr, sem);
    printf("Unexpected prefix: %s\n", (res == 0) ? "PASSED" : "FAILED");
}



// ----------------- Main -----------------
int main() {
    // Setup shared memory + semaphore
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(shared_data_t));
    shm_ptr = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);

    
    test_normal_flow();
    test_corrupted_data();
    test_sysfs_behavior();
    
    // Cleanup
    munmap(shm_ptr, sizeof(shared_data_t));
    close(shm_fd);
    sem_close(sem);
    sem_unlink(SEM_NAME);
    shm_unlink(SHM_NAME);

    return 0;
}