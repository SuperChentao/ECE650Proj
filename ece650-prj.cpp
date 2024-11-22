#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#include "EdgeManager.cpp"

#define RUNS 10

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;

void *input_thread(void *);
void *min_vc_thread(void *);
void *vc1_thread(void *);
void *vc2_thread(void *);

struct GraphData
{
    EdgeManager *EM;
    int *V;
    string *E;
    string *min_VC;
    string *VC1;
    string *VC2;
};

sem_t semaphore_read;
sem_t semaphore_write;
sem_t semaphore_calc;

int main(int argc, char **argv)
{
    sem_init(&semaphore_read, 0, 1);
    sem_init(&semaphore_calc, 0, 0);

    GraphData graph_data;
    graph_data.V = new int;
    graph_data.E = new string;
    graph_data.min_VC = new string;
    graph_data.VC1 = new string;
    graph_data.VC2 = new string;

    pthread_t thread[4];
    clockid_t clock_ids[3];
    struct timespec ts[3];

    pthread_create(&thread[0], NULL, input_thread, &graph_data);

    while (true)
    {
        sem_wait(&semaphore_calc);
        if (graph_data.V == nullptr)
            break;

        pthread_create(&thread[1], NULL, min_vc_thread, &graph_data);
        if (pthread_getcpuclockid(thread[1], &clock_ids[0]) != 0)
        {
            std::cerr << "Failed to get clock ID for thread 1\n";
            return -1;
        }

        pthread_create(&thread[2], NULL, vc1_thread, &graph_data);
        if (pthread_getcpuclockid(thread[2], &clock_ids[1]) != 0)
        {
            std::cerr << "Failed to get clock ID for thread 2\n";
            return -1;
        }

        pthread_create(&thread[3], NULL, vc2_thread, &graph_data);
        // if (pthread_getcpuclockid(thread[3], &clock_ids[2]) != 0)
        // {
        //     std::cerr << "Failed to get clock ID for thread 3\n";
        //     return -1;
        // }

        if (clock_gettime(clock_ids[0], &ts[0]) == 0)
        {
            std::cout << "Thread 1 for " << RUNS << "times calculation CPU time : "
                      << ts[0].tv_sec << " seconds and " << ts[0].tv_nsec << " nanoseconds\n";
        }
        else
        {
            std::cerr << "Failed to get CPU time for thread 1: " << strerror(errno) << "\n";
        }

        if (clock_gettime(clock_ids[1], &ts[1]) == 0)
        {
            std::cout << "Thread 1 for " << RUNS << "times calculation CPU time : "
                      << ts[1].tv_sec << " seconds and " << ts[1].tv_nsec << " nanoseconds\n";
        }
        else
        {
            std::cerr << "Failed to get CPU time for thread 2: " << strerror(errno) << "\n";
        }

        if (clock_gettime(clock_ids[2], &ts[2]) == 0)
        {
            std::cout << "Thread 1 for " << RUNS << "times calculation CPU time : "
                      << ts[2].tv_sec << " seconds and " << ts[2].tv_nsec << " nanoseconds\n";
        }
        else
        {
            std::cerr << "Failed to get CPU time for thread 3: " << strerror(errno) << "\n";
        }

        pthread_join(thread[1], NULL);
        pthread_join(thread[2], NULL);
        pthread_join(thread[3], NULL);

        sem_post(&semaphore_write);
        sem_post(&semaphore_read);
    }

    pthread_join(thread[0], NULL);

    // Clean up semaphores
    sem_destroy(&semaphore_read);
    sem_destroy(&semaphore_calc);

    return 0;
}

void *input_thread(void *arg)
{
    GraphData *data = static_cast<GraphData *>(arg);

    string line;
    char command;

    cout << "Start thread 1" << endl;

    while (true)
    {
        // wait until vertex cover for last input calculated
        sem_wait(&semaphore_read);
        if (cin.eof())
        {
            cout << "read eof" << endl;
            break;
        }

        std::getline(cin, line);
        if (line.empty())
        {
            cout << "read empty line" << endl;
            break;
        }

        // Assume all input is valid
        std::istringstream input(line);
        input >> command;
        input >> *(data->V);
        std::getline(cin, line);
        std::istringstream edge_input(line);
        edge_input >> command;
        edge_input >> *(data->E);

        // vertex cover calculation can start
        sem_post(&semaphore_calc);

        sem_wait(&semaphore_write);
        cout << "CNF-SAT-VC: " << *(data->min_VC) << endl;
        cout << "APPROX-VC-1: " << *(data->VC1) << endl;
        cout << "APPROX-VC-2: " << *(data->VC2) << endl;
    }

    // Free memory only if the thread terminates successfully
    delete data->V;
    delete data->E;
    delete data->min_VC;
    delete data->VC1;
    delete data->VC2;
    data->V = nullptr;
    data->E = nullptr;
    data->min_VC = nullptr;
    data->VC1 = nullptr;
    data->VC2 = nullptr;

    sem_post(&semaphore_calc);
    cout << "End of thread 1" << endl;
    pthread_exit(NULL);
}

void *min_vc_thread(void *arg)
{
    cout << "thread2 starts" << endl;
    GraphData *data = static_cast<GraphData *>(arg);

    for (int i = 0; i < RUNS; i++)
    {
        EdgeManager EM = EdgeManager(*(data->V), *(data->E));
        *(data->min_VC) = EM.findVertexCover();
    }

    cout << "thread2 ends" << endl;
    pthread_exit(NULL);
}

void *vc1_thread(void *arg)
{
    cout << "thread3 starts" << endl;
    GraphData *data = static_cast<GraphData *>(arg);

    for (int i = 0; i < RUNS; i++)
    {
        EdgeManager EM = EdgeManager(*(data->V), *(data->E));
        *(data->VC1) = EM.approxVC1();
    }

    cout << "thread3 ends" << endl;
    pthread_exit(NULL);
}

void *vc2_thread(void *arg)
{
    cout << "thread4 starts" << endl;
    GraphData *data = static_cast<GraphData *>(arg);
    for (int i = 0; i < RUNS; i++)
    {
        EdgeManager EM = EdgeManager(*(data->V), *(data->E));
        *(data->VC2) = EM.approxVC2();
    }

    cout << "thread4 ends" << endl;
    pthread_exit(NULL);
}

// ssh c65jin@eceubuntu.uwaterloo.ca
// cd c65jin/project/build