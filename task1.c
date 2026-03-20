#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <syslog.h>
#include <gtk/gtk.h>



#define SHM_NAME "/my_shm"
#define SEM_NAME "/my_sem"

typedef struct {
    int value;
} shared_data_t;

shared_data_t* shm_ptr;
sem_t* sem;

GtkWidget* label;

void* ProducerThread(void* arg) {
    sem_t* sem = sem_open(SEM_NAME, 0);
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    shared_data_t* shm_ptr = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    char line[256];
    
    while (1) {
        int base = 2 + rand() % 2;       
        int jitter = (rand() % 5) - 2;   
        int interval = base + jitter;

        FILE *f = fopen("./fake_sysfs_input", "r");
        if (!f) {
            syslog(LOG_ERR, "File missing retrying in 5 sec");
            sleep(5);
            continue; 
        }

        while(fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;
        
        if (strcmp(line, "1") == 0) {
            shm_ptr->value = interval; // write value
            printf("[Producer] Sent: %d\n", interval);
        } else if(strcmp(line, "error: temp too high") == 0) {
            printf("Cool-down 7 sec\n");
        } else printf("Not sending anything!\n");

        }
    fclose(f);
    }
    
    return 0;
}

void* ConsumerThread(void* arg) {
    sem_t* sem = sem_open(SEM_NAME, 0);
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    shared_data_t* shm_ptr = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    while (1) {
        int received = shm_ptr->value;  
        printf("[Consumer] Received: %d\n", received);
    }

    return 0;
}

gboolean update_label(gpointer data) {
    int val;
    sem_wait(sem);
    val = shm_ptr->value;
    sem_post(sem);

    char buf[64];
    snprintf(buf, sizeof(buf), "Received: %d", val);
    gtk_label_set_text(GTK_LABEL(label), buf);
    return TRUE;
}
int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    // Shared memory + semaphore
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(shared_data_t));
    shm_ptr = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);

    // Create GUI
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Producer/Consumer");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 100);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    label = gtk_label_new("Waiting for data...");
    gtk_container_add(GTK_CONTAINER(window), label);

    gtk_widget_show_all(window);

    pthread_t producer;
    pthread_create(&producer, NULL, ProducerThread, NULL);

    g_timeout_add(500, update_label, NULL);

    gtk_main();

    munmap(shm_ptr, sizeof(shared_data_t));
    close(shm_fd);
    sem_close(sem);
    sem_unlink(SEM_NAME);
    shm_unlink(SHM_NAME);

    return 0;
}