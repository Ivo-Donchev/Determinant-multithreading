#include <iostream>
#include <fstream>
#include <thread>
#include <future>
#include <random>
#include <time.h>
#include <ctype.h>

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
int get_determinant(int** &matrix, int &size, int num_of_threads)
{
    int half_threads;
    if(num_of_threads > 2*size + 3){
        num_of_threads = size;
    }

    if(num_of_threads < 2)
        return get_minuend_multithreaded_recursively(matrix, size, 0, 0) - get_substrahend_multithreaded_recursively(matrix, size, 0, 0);

    if(num_of_threads == 2){
        future<long int> t1 = async(std::launch::async, get_substrahend_multithreaded_recursively, std::ref(matrix), size, 0, 0);
        return get_minuend_multithreaded_recursively(matrix, size, 0, 0) - t1.get();
    }
 
    // In case we have different threads for substrahend and minuend
    // num_of_threads > 2

    num_of_threads -= 3; // for the following two functions + the main thread
    half_threads = num_of_threads / 2;

    future<long int> t1 = async(std::launch::async, get_minuend_multithreaded_recursively, std::ref(matrix), size, half_threads, 0); // without the current thread
    future<long int> t2 = async(std::launch::async, get_substrahend_multithreaded_recursively, std::ref(matrix), size, num_of_threads - half_threads, 0); // without the current thread
    
    return t1.get() + t2.get();
}

double diffclock(clock_t clock1,clock_t clock2)
{
    double diffticks=clock1-clock2;
    double diffms=(diffticks)/(CLOCKS_PER_SEC/1000);
    return diffms;
}

int get_number(string msg)
{
    for(int i = 0; i < msg.length(); i++)
    {
        if(!(isdigit(msg[i])))
        {
            return -1;
        }
    }
    return stoi(msg);
}
int main(int argc, char* argv[])
{
    time_t begin = clock();    
    string fcommand, scommand;
    int num_of_threads;
    if(argc == 4){
        fcommand = argv[1];
        scommand = argv[2];
        num_of_threads = atoi(argv[3]);
        if(!num_of_threads){
            cout << "Error number of threads!!\n";
            return 1;
        }
    }
    else{
        cout << "Error invalid arguments!!\n";
        return 1;
    }

    
    
    if(fcommand == "-i"){ // file input 
        ifstream file;
        string size_str;
        file.open(scommand);
        file >> size_str;
        
        int size = stoi(size_str);
        int** matrix = allocMatrix(size);

        string number;
        for(int i = 0; i < size; i++)
        {
            for(int j = 0; j < size; j++)
            {
                file >> number;
                matrix[i][j] = stoi(number);

            }
        }

        time_t start = clock();
        cout << get_determinant(matrix, size, num_of_threads) << endl;
        time_t end = clock();
        cout << endl << "Total execution time for current run (millis): " << diffclock(end, start) << endl;

        file.close();
        deleteMatrix(matrix, size);
    }
    else if(fcommand == "-n"){ // random numbers
        int size = get_number(scommand);
        int **matrix = initMatrix(size);

        time_t start = clock();
        cout << get_determinant(matrix, size, num_of_threads) << endl;
        time_t end = clock();
        cout << endl << "Total execution time for current run (millis): " << diffclock(end, start) << endl;

        deleteMatrix(matrix, size);
    }
    else{
        cout << "Error: Invalid command!\n";
    }
    time_t finish = clock();
    cout << endl << "Execution time for the entire program: " << diffclock(finish, begin) << endl;

    return 0;
}