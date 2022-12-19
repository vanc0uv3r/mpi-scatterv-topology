#include <iostream>
#include <mpich/mpi.h>
#include <vector>
#include <cstring>

#define MATX_SIZE 4
#define PAYLOAD_SIZE 120

using namespace std;

void init_maps(vector<pair<int, int>> &map_d, int map_r[][MATX_SIZE])
{
    for (int i = 0; i < MATX_SIZE; i++)
    {
        for (int j = 0; j < MATX_SIZE; j++)
        {
            map_d[i * MATX_SIZE + j] = make_pair(i, j);
            map_r[i][j] = i * MATX_SIZE + j;
        }
    }
}


void print_found(int *payload, int my_rank)
{
    printf("[MPI process %d] GOT IT \n", my_rank);
    for (int i = 0; i < my_rank; i++)
        printf("%d ", payload[i]);
    printf("\n");
}


int main(int argc, char *argv[])
{
    int size;
    int dims[2] = {0, 0};
    int periods[2] = {false, false};
    int my_rank;
    int my_coords[2];
    pair<int, int> coords;
    int map_r[MATX_SIZE][MATX_SIZE];
    int *payload = new int[PAYLOAD_SIZE];

    vector<pair<int, int>> map_d(MATX_SIZE * MATX_SIZE);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Dims_create(size, 2, dims);
    MPI_Comm new_communicator;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, false, &new_communicator);

    MPI_Comm_rank(new_communicator, &my_rank);
    MPI_Cart_coords(new_communicator, my_rank, 2, my_coords);
    init_maps(map_d, map_r);

    if (my_rank == 0)
    {
        int k = 0;
        for (int i = 1; i < 16; i++)
        {
            for (int j = 1; j <= i; j++)
            {
                payload[k] = j * 10;
                k++;
            }
        }
        coords = map_d[my_rank];
        MPI_Isend(payload, PAYLOAD_SIZE , MPI_INT, map_r[coords.first][coords.second + 1], 1, new_communicator);
        MPI_Isend(payload, PAYLOAD_SIZE , MPI_INT, map_r[coords.first + 1][coords.second], 1, new_communicator);
    }
    else
    {
        int *my_buff = new int[my_rank];
        MPI_Status status;
        MPI_Recv(payload, PAYLOAD_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, new_communicator, &status);
        
        coords = map_d[my_rank];
        if (coords.second == 0 && coords.first < MATX_SIZE) 
            MPI_Isend(payload, PAYLOAD_SIZE, MPI_INT, map_r[coords.first + 1][coords.second], 1, new_communicator);
        
        if (coords.second < MATX_SIZE)
            MPI_Isend(payload, PAYLOAD_SIZE, MPI_INT, map_r[coords.first][coords.second + 1], 1, new_communicator);

        memcpy(my_buff, payload + ((my_rank - 1) * (my_rank) / 2), my_rank * 4);
        
        print_found(my_buff, my_rank);
    }

    MPI_Finalize();

    return EXIT_SUCCESS;
} 
