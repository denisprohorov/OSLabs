#include <iostream>
#include <ctime>
#include <chrono>
#include <pthread.h>
#include <queue>
#include <mutex>

const int COUNT_THREADS = 4;

struct Block{
    int** matrix;
    int startN;
    int endN;
    int startM;
    int endM;
    int startK;
    int endK;

    Block() {}

    Block(int **matrix, int startN, int endN, int startK, int endK) :
    matrix(matrix), startN(startN), endN(endN), startK(startK), endK(endK) {}

    Block(int **matrix, int startM, int endM) : matrix(matrix), startM(startM), endM(endM) {}

};

struct MulData{
public:
    Block blockA;
    Block blockB;
    int** resultMatrix;
    void (*writeFncPtr)(MulData*, int**);

    MulData() {}

    MulData(const Block &blockA, const Block &blockB, int **resultMatrix) :
    blockA(blockA), blockB(blockB), resultMatrix(resultMatrix) {}

    MulData(const Block &blockA, const Block &blockB, int **resultMatrix, void (*writeFncPtr)(MulData *, int **))
            : blockA(blockA), blockB(blockB), resultMatrix(resultMatrix), writeFncPtr(writeFncPtr) {}
};

class BufferedChannel {
private:
    std::queue<MulData*> queue;
    bool isOpen;
    pthread_mutex_t mutex;

public:
    explicit BufferedChannel() : isOpen(true) {pthread_mutex_init(&mutex, NULL);}

    void Send(MulData* value){
        queue.push(value);
    }

    std::pair<MulData*, bool> Recv() {
        pthread_mutex_lock(&mutex);
        if(!this->isOpen || queue.empty()){
            this->Close();
            std::pair<MulData*, bool> pair;
            pair.first = NULL;
            pair.second = false;
            pthread_mutex_unlock(&mutex);
            return pair;
        }
        //queue is not empty
        std::pair<MulData*, bool> pair(queue.front(), true);
        queue.pop();
        pthread_mutex_unlock(&mutex);
        return pair;
    }

    void Close() {
        isOpen = false;
    }
};

struct ThirdData : MulData{
    pthread_mutex_t mutex;

    ThirdData(const Block &blockA, const Block &blockB, int **resultMatrix, void (*writeFncPtr)(MulData *, int **),
              const pthread_mutex_t &mutex) :
    MulData(blockA, blockB, resultMatrix, writeFncPtr), mutex(mutex) {}

    ThirdData() {}
};

int getRand(){
    int size = 300;
    return rand() % size - size / 2;
}

void deleteMatrix(void** matrix, int n){
    for (int i = 0; i < n; ++i) {
        delete[] matrix[i];
    }
    delete[] matrix;
}

void fillMatrix(int** A, int n, int m){
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            A[i][j] = getRand();
        }
    }
}

int** createMatrix(int n, int m){
    int** A = new int*[n];
    for (int i = 0; i < n; ++i) {
        A[i] = new int[m];
        for (int j = 0; j < m; ++j) {
            A[i][j] = 0;
        }
    }
    return A;
}

bool isEquals(int** A, int** B, int n, int m){
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if(A[i][j] != B[i][j]) return false;
        }
    }
    return true;
}

int** mul(void* lpParam){
    MulData* data = (MulData*) lpParam;
    int** C = createMatrix(data->blockA.endN - data->blockA.startN, data->blockB.endM - data->blockB.startM);
    for (int i = data->blockA.startN; i < data->blockA.endN; ++i) {
        for (int j = data->blockB.startM; j < data->blockB.endM; ++j) {
            for (int l = data->blockA.startK; l < data->blockA.endK; ++l) {
                C[i - data->blockA.startN][j - data->blockB.startM] +=
                        (data->blockA.matrix[i][l] * data->blockB.matrix[l][j]);
            }
        }
    }
    return C;
}

void writeMatrix(MulData* data, int** matrix){
    for (int i = 0; i < data->blockA.endN - data->blockA.startN; ++i) {
        for (int j = 0; j < data->blockB.endM - data->blockB.startM; ++j) {
            data->resultMatrix[i + data->blockA.startN][j + data->blockB.startM] += matrix[i][j];
        }
    }
}

void* threadProc(void* lpParam){
    BufferedChannel* channel = (BufferedChannel*) lpParam;
    std::pair<MulData*, bool> param = channel->Recv();
    while(param.second) {
        MulData* data = param.first;
        int** C = mul((void*) data);
        //write block result
        data->writeFncPtr(data, C);
        deleteMatrix((void**)C, data->blockA.endN - data->blockA.startN);
        delete data;
        param = channel->Recv();
    }
    return 0;
}

int** secondMul(void* lpParam){
    BufferedChannel channel;
    MulData* data = (MulData*) lpParam;
    auto pThreadArray = new pthread_t[COUNT_THREADS];
    for (int i = 0; i < COUNT_THREADS; ++i) {
        for (int j = 0; j < COUNT_THREADS; ++j) {
            channel.Send(new MulData(Block(data->blockA.matrix, i * data->blockA.endN / COUNT_THREADS,
                                       (i + 1) * data->blockA.endN / COUNT_THREADS,
                                       data->blockA.startK, data->blockA.endK),
                                Block(data->blockB.matrix, j * data->blockB.endM / COUNT_THREADS,
                                       (j + 1) * data->blockB.endM / COUNT_THREADS),
                                       data->resultMatrix, writeMatrix));
        }
    }

    for (int i = 0; i < COUNT_THREADS; ++i) {
        pthread_create(&pThreadArray[i], NULL, threadProc, (void*)&channel);
    }
    for (int i = 0; i < COUNT_THREADS; ++i) {
        pthread_join(pThreadArray[i], NULL);
    }
    delete[] pThreadArray;
    return data->resultMatrix;
}

void writeMatrixUsingMutex(MulData* data, int** matrix){
    ThirdData* thirdData = (ThirdData*) data;
    pthread_mutex_lock(&thirdData->mutex);
    writeMatrix(data, matrix);
    pthread_mutex_unlock(&thirdData->mutex);
}

int** thirdMul(void* lpParam){
    BufferedChannel channel;
    MulData* data = (MulData*) lpParam;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    auto pThreadArray = new pthread_t[COUNT_THREADS];
    for (int i = 0; i < COUNT_THREADS; ++i) {
        channel.Send(new ThirdData(Block(data->blockA.matrix, data->blockA.startN, data->blockA.endN,
                                         i * data->blockA.endK / COUNT_THREADS,
                                         (i + 1) * data->blockA.endK / COUNT_THREADS),
                                   Block(data->blockB.matrix, data->blockB.startM, data->blockB.endM),
                                   data->resultMatrix, writeMatrixUsingMutex, mutex));
    }
    for (int i = 0; i < COUNT_THREADS; ++i) {
                pthread_create(&pThreadArray[i],NULL, threadProc, (void*) &channel);

    }
   for(int i = 0; i < COUNT_THREADS; i++){
        pthread_join(pThreadArray[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
    delete[] pThreadArray;
    return data->resultMatrix;
}

int** fourMul(void* lpParam){
    BufferedChannel channel;
    MulData* data = (MulData*) lpParam;
    auto pThreadArray = new pthread_t[COUNT_THREADS];
    auto pMutexMatrix = new pthread_mutex_t*[COUNT_THREADS];
    for (int i = 0; i < COUNT_THREADS; ++i) {
        pMutexMatrix[i] = new pthread_mutex_t[COUNT_THREADS];
        for (int j = 0; j < COUNT_THREADS; ++j) {
            pthread_mutex_init(&pMutexMatrix[i][j], NULL);
        }
    }
    for (int i = 0; i < COUNT_THREADS; ++i) {
            for (int k = 0; k < COUNT_THREADS; ++k) {
        for (int j = 0; j < COUNT_THREADS; ++j) {
                channel.Send(new ThirdData(Block(data->blockA.matrix,
                                                 i * data->blockA.endN / COUNT_THREADS,
                                                 (i + 1) * data->blockA.endN / COUNT_THREADS,
                                                 k * data->blockA.endK / COUNT_THREADS,
                                                 (k + 1) * data->blockA.endK / COUNT_THREADS),
                                           Block(data->blockB.matrix,
                                                 j * data->blockB.endM / COUNT_THREADS,
                                                 (j + 1) * data->blockB.endM / COUNT_THREADS),
                                           data->resultMatrix, writeMatrixUsingMutex, pMutexMatrix[i][j]));
            }
        }
    }
    for (int i = 0; i < COUNT_THREADS; ++i) {
                pthread_create(&pThreadArray[i], NULL, threadProc, (void*) &channel);

    }
    for(int i = 0; i < COUNT_THREADS; i++){
        pthread_join(pThreadArray[i], NULL);
        for (int j = 0; j < COUNT_THREADS; ++j) {
            pthread_mutex_destroy(&pMutexMatrix[i][j]);
        }
    }
    deleteMatrix((void**)pMutexMatrix, COUNT_THREADS);
    delete[] pThreadArray;
    return data->resultMatrix;
}

int callMul(int** (*fncPtr)(void*), MulData* data, int** &resultMatrix){
    auto startTime = std::chrono::system_clock::now();
    resultMatrix = fncPtr(data);
    auto endTime = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
}

void printMatrix(int** A, int n, int m){
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            std::cout << A[i][j] << " ";
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}


int main(){
//    freopen("cmake-build-debug/input.txt", "r", stdin);
    freopen("input.txt", "r", stdin);
    srand(time(0));
    int n, k, m;
    std::cin >> n >> k >> m;
    int** A = createMatrix(n, k);
    fillMatrix(A, n, k);
    int** B = createMatrix(k, m);
    fillMatrix(B, k, m);
    int** C0 = createMatrix(n, m);
    MulData* parameters = new MulData(Block(A, 0, n, 0, k), Block(B, 0, m), C0);

    int time0 = callMul(mul, parameters, C0);

    int** C1 = createMatrix(n, m);
    parameters->resultMatrix = C1;
    int time1 = callMul(secondMul, parameters, C1);

    int** C2 = createMatrix(n, m);
    parameters->resultMatrix = C2;
    int time2 = callMul(thirdMul, parameters, C2);

    int** C3 = createMatrix(n, m);
    parameters->resultMatrix = C3;
    int time3 = callMul(fourMul, parameters, C3);

//    printMatrix(A, n, k);
//    printMatrix(B, k, m);
//    printMatrix(C0, n, m);
//    printMatrix(C1, n, m);
//    printMatrix(C2, n, m);
//    printMatrix(C3, n, m);

    std::cout << "mul0 : " << time0 << "ms" << '\n';
    std::cout << "mul1 : " << time1 << "ms" << '\n';
    std::cout << "mul2 : " << time2 << "ms" << '\n';
    std::cout << "mul3 : " << time3 << "ms" << '\n';
    std::cout << "C0, C1 is equals? " << std::boolalpha << isEquals(C0, C1, n, m) << '\n';
    std::cout << "C1, C2 is equals? " << std::boolalpha << isEquals(C1, C2, n, m) << '\n';
    std::cout << "C2, C3 is equals? " << std::boolalpha << isEquals(C2, C3, n, m) << '\n';
    delete parameters;
    deleteMatrix((void**)A, n);
    deleteMatrix((void**)B, k);
    deleteMatrix((void**)C0, n);
    deleteMatrix((void**)C1, n);
    deleteMatrix((void**)C2, n);
    deleteMatrix((void**)C3, n);

}
