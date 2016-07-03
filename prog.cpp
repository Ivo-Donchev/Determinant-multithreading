#include <iostream>
#include <thread>
#include <future>
#include <random>
#include <time.h>

using namespace std;

int** allocMatrix(int &size)
{
    int** matrix = new int*[size];

    for(int i = 0; i < size;i++)
    {
        matrix[i] = new int[size];
    }

    return matrix;
}


int** initMatrix(int &size)
{
    int** matrix = allocMatrix(size);
    srand(0);

    for(int i = 0; i < size; i++)
    {
        for(int j = 0; j < size; j++)
        {
            matrix[i][j] = rand() % 9 + 1;    
        }
    }

    return matrix;
}
void deleteMatrix(int** &matrix, int &size)
{
    for(int i = 0; i < size; i++)
        delete [] matrix[i];
    delete [] matrix;
}
void printMatrix(int** &matrix, int size)
{
    for(int i = 0; i < size; i++)
    {
        for(int j = 0; j < size; j++)
            cout << matrix[i][j] << " ";
        cout<<endl;
    }
}
int get_index(int index, int &size)
{
    if(index < 0)
        return get_index(index+size, size);
    if(index >= size)
        return index % size;
    return index;
}

long int get_substrahend(int** &matrix, int size)
{
    long int substrahend = 0;
    for(int i = 0; i < size; i++)
    {
        long int curr = 1;
        for(int j = 0; j < size; j++)
        {
            int row_index = get_index(j, size);
            int col_index = get_index(i - j, size);
            curr *= matrix[row_index][col_index];
        }
        substrahend += curr;
    }

    return substrahend;
}

long int get_minuend(int** &matrix, int size)
{
    long int minuend = 0;
    for(int i = 0; i < size; i++)
    {
        long int curr = 1;
        for(int j = 0; j < size; j++)
        {
            int row_index = get_index(j, size);
            int col_index = get_index(i + j, size);
            curr *= matrix[row_index][col_index];
        }
        minuend += curr;
    }

    return minuend;
}
long int get_substrahend_multithreaded_recursively(int** &matrix, int size, int subthreads_number, int start=0)
{
    if(subthreads_number == -1) return 0;
    
    long int substrahend = 0;
    int moves = size / (subthreads_number + 1);
    if(!start && (size / (subthreads_number + 1)))
        moves++;

    future<long int> subthread = async(std::launch::async, get_substrahend_multithreaded_recursively, std::ref(matrix), size, subthreads_number - 1, start + moves);

    for(int i = start; i < moves; i++)
    {
        long int curr = 1;
        for(int j = 0; j < size; j++)
        {
            int row_index = get_index(j, size);
            int col_index = get_index(i - j, size);
            curr *= matrix[row_index][col_index];
        }
        substrahend += curr;
    }
    long int subthread_val = subthread.get();
    return substrahend + subthread_val;
}

long int get_minuend_multithreaded_recursively(int** &matrix, int size, int subthreads_number, int start=0)
{

    if(subthreads_number == -1) return 0;

    long int minuend = 0;
    int moves = size / (subthreads_number + 1);
    if(!start && (size / (subthreads_number + 1)))
        moves++;


    future<long int> subthread = async(std::launch::async, get_minuend_multithreaded_recursively, std::ref(matrix), size, subthreads_number - 1, start + moves);

    for(int i = start; i < moves; i++)
    {
        long int curr = 1;
        for(int j = 0; j < size; j++)
        {
            int row_index = get_index(j, size);
            int col_index = get_index(i + j, size);
            curr *= matrix[row_index][col_index];
        }
        minuend += curr;
    }
    long int minuend_val = subthread.get();
    return minuend + minuend_val;

}




long int get_substrahend_multithreaded(int** &matrix, int start, int end)
{
    long int substrahend = 0;

    for(int i = start; i < start + end; i++)
    {
        long int curr = 1;
        for(int j = 0; j < end; j++)
        {
            int row_index = get_index(j, end);
            int col_index = get_index(i - j, end);
            curr *= matrix[row_index][col_index];
        }
        substrahend += curr;
    }

    return substrahend;
}

long int get_minuend_multithreaded(int** &matrix, int start, int end)
{
    long int minuend = 0;
    for(int i = start; i < start + end; i++)
    {
        long int curr = 1;
        for(int j = 0; j < end; j++)
        {
            int row_index = get_index(j, end);
            int col_index = get_index(i + j, end);
            curr *= matrix[row_index][col_index];
        }
        minuend += curr;
    }
    
    return minuend;

}



int get_determinant(int** &matrix, int &size, int num_of_threads)
{
    int half_threads;
    if(num_of_threads > size){
        num_of_threads = size;
    }

    if(num_of_threads < 2)
        return get_minuend_multithreaded_recursively(matrix, size, 0, 0) - get_substrahend_multithreaded_recursively(matrix, size, 0, 0);

    if(num_of_threads == 2){
        future<long int> t1 = async(std::launch::async, get_substrahend_multithreaded_recursively, std::ref(matrix), size, 0, 0);
        return get_minuend(matrix, size) - t1.get();
    }
 
    // In case we have different threads for substrahend and minuend
    // num_of_threads > 2

    num_of_threads -= 3; // for the following two functions + the main thread
    half_threads = num_of_threads / 2;

    future<long int> t1 = async(std::launch::async, get_minuend_multithreaded_recursively, std::ref(matrix), size, half_threads, 0); // without the current thread
    future<long int> t2 = async(std::launch::async, get_substrahend_multithreaded_recursively, std::ref(matrix), size, num_of_threads - half_threads, 0); // without the current thread
    return t1.get() + t2.get();

    // future<long int> minuend_parts_threads[num_of_threads];
    // future<long int> substrahend_parts_threads[num_of_threads - half_threads];
    // int minuend_moves;
    // int substrahend_moves = size / (num_of_threads - half_threads);

    // if(half_threads)
    //     minuend_moves = size / half_threads;
    // else
    //     minuend_moves = 0;

    // cout << half_threads << endl;
    // cout << size << endl;
    // cout << minuend_moves << endl;
    // cout << substrahend_moves << endl;

    // if(half_threads){
    //     for(int i = 0; i < half_threads; i++)
    //     {
    //         if(i == half_threads-1){
    //             minuend_parts_threads[i] = async(std::launch::async, get_minuend_multithreaded, std::ref(matrix), i*minuend_moves, size);
    //             break;
    //         }

    //         minuend_parts_threads[i] = async(std::launch::async, get_minuend_multithreaded, std::ref(matrix), i*minuend_moves, (i + 1)*minuend_moves);
    //     }
    // }

    // if(num_of_threads - half_threads)
    // {
    //     for(int i = 0; i < num_of_threads - half_threads; i++)
    //     {
    //         if(i == half_threads-1){
    //             substrahend_parts_threads[i] = async(std::launch::async, get_substrahend_multithreaded, std::ref(matrix), i*substrahend_moves, size); 
    //             break;
    //         }

    //         substrahend_parts_threads[i] = async(std::launch::async, get_substrahend_multithreaded, std::ref(matrix), i*substrahend_moves, (i+1)*substrahend_moves); 
    //     }
    // }

    // int determinant = 0;
    
    // if(half_threads){
    //     for(int i = 0; i < half_threads; i++)
    //         determinant += minuend_parts_threads[i].get();
    // }
    // else{
    //     determinant += get_minuend(matrix, size);
    // }
    
    // if(num_of_threads - half_threads)
    // {
    //     for(int i = 0; i < num_of_threads - half_threads; i++)
    //         determinant -= substrahend_parts_threads[i].get();
    // }
    // else{
    //     get_substrahend(matrix, size);
    // }

    
    // return determinant;

}

double diffclock(clock_t clock1,clock_t clock2)
{
    double diffticks=clock1-clock2;
    double diffms=(diffticks)/(CLOCKS_PER_SEC/1000);
    return diffms;
}
int main()
{
    const int num_of_threads = 3;
    int size = 20;
    int **matrix = initMatrix(size);
    int minuend, substrahend;


    time_t start = clock();
    cout << get_determinant(matrix, size, num_of_threads) << endl;
    time_t end = clock();

    deleteMatrix(matrix, size);

    cout << endl << "Time: " << diffclock(end, start) << endl;
    return 0;
}