#include <iostream>
#include <ctime>
#include <chrono>
#include <windows.h>
#include <queue>
#include <mutex>
#include <condition_variable>

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

    MulData() {}

    MulData(const Block &blockA, const Block &blockB, int **resultMatrix) : blockA(blockA), blockB(blockB),
                                                                            resultMatrix(resultMatrix) {}
};

class BufferedChannel {
private:
    std::queue<MulData*> queue;
    bool isOpen;
    HANDLE mutex;

public:
    explicit BufferedChannel() : isOpen(true) {mutex = CreateMutex(NULL, FALSE, NULL);}

    void Send(MulData* value){
        queue.push(value);
    }

    std::pair<MulData*, bool> Recv() {
        if(mutex != NULL) WaitForSingleObject(mutex, INFINITE);
        if(!this->isOpen || queue.empty()){
            this->Close();
            std::pair<MulData*, bool> pair;
            pair.first = NULL;
            pair.second = false;
            return pair;
        }
        //queue is not empty
        std::pair<MulData*, bool> pair(queue.front(), true);
        queue.pop();
        ReleaseMutex(mutex);
        return pair;
    }

    void Close() {
        isOpen = false;
    }
};

struct ThirdData : MulData{
    HANDLE mutex;

    ThirdData(const Block &blockA, const Block &blockB, int **resultMatrix, const void *mutex) :
    MulData(blockA, blockB, resultMatrix) {mutex = CreateMutex(NULL, FALSE, NULL);}

    ThirdData() {}
};

int getRand(){
    int size = 300;
    return rand() % size - size / 2;
}

void deleteMatrix(int** matrix, int n){
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

int** mul(LPVOID lpParam){
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

DWORD secondMulProc(LPVOID lpParam){
    BufferedChannel* channel = (BufferedChannel*) lpParam;
    std::pair<MulData*, bool> param = channel->Recv();
    int **matrix;
    if(param.first != NULL) {
        matrix = param.first->resultMatrix;
    }
    while(param.second) {
        MulData* data = param.first;
        int** C = mul((LPVOID) data);
        for (int i = 0; i < data->blockA.endN - data->blockA.startN; ++i) {
            for (int j = 0; j < data->blockB.endM - data->blockB.startM; ++j) {
                matrix[i + data->blockA.startN][j + data->blockB.startM] = C[i][j];
            }
        }
        deleteMatrix(C, data->blockA.endN - data->blockA.startN);
        delete data;
        param = channel->Recv();
    }
    return 0;
}

int** secondMul(LPVOID lpParam){
    BufferedChannel channel;
    MulData* data = (MulData*) lpParam;
    auto pThreadArray = new HANDLE[COUNT_THREADS];
    for (int i = 0; i < COUNT_THREADS; ++i) {
        for (int j = 0; j < COUNT_THREADS; ++j) {
            channel.Send(new MulData(Block(data->blockA.matrix, i * data->blockA.endN / COUNT_THREADS,
                                       (i + 1) * data->blockA.endN / COUNT_THREADS,
                                       data->blockA.startK, data->blockA.endK),
                                Block(data->blockB.matrix, j * data->blockB.endM / COUNT_THREADS,
                                       (j + 1) * data->blockB.endM / COUNT_THREADS),
                                       data->resultMatrix));
        }
    }

    for (int i = 0; i < COUNT_THREADS; ++i) {
        pThreadArray[i] = CreateThread(NULL, 0, secondMulProc,
                                       (LPVOID)&channel, 0, 0);

    }
    WaitForMultipleObjects(COUNT_THREADS, pThreadArray, TRUE, INFINITE);
    for(int i = 0; i < COUNT_THREADS; i++){
        CloseHandle(pThreadArray[i]);
    }
    return data->resultMatrix;
}

DWORD thirdMulProc(LPVOID lpParam){
    BufferedChannel* channel = (BufferedChannel*) lpParam;
    std::pair<MulData*, bool> param = channel->Recv();
    int **matrix;
    if(param.first != NULL) {
        matrix = param.first->resultMatrix;
    }
    while(param.second) {
        ThirdData* data = (ThirdData*) param.first;
        int** C = mul((LPVOID) data);
        if (data->mutex != NULL) WaitForSingleObject(data->mutex, INFINITE);
        for (int i = 0; i < data->blockA.endN; ++i) {
            for (int j = 0; j < data->blockB.endM; ++j) {
                matrix[i][j] += C[i][j];
            }
        }
        ReleaseMutex(data->mutex);
        deleteMatrix(C, data->blockA.endN);
        delete data;
        param = channel->Recv();
    }
    return 0;
}

int** thirdMul(LPVOID lpParam){
    BufferedChannel channel;
    MulData* data = (MulData*) lpParam;
    HANDLE mutex = CreateMutex(NULL, FALSE, NULL);
    auto pThreadArray = new HANDLE[COUNT_THREADS];
    for (int i = 0; i < COUNT_THREADS; ++i) {
        channel.Send(new ThirdData(Block(data->blockA.matrix, data->blockA.startN, data->blockA.endN,
                                         i * data->blockA.endK / COUNT_THREADS,
                                         (i + 1) * data->blockA.endK / COUNT_THREADS),
                                   Block(data->blockB.matrix, data->blockB.startM, data->blockB.endM),
                                   data->resultMatrix, &mutex));
    }
    for (int i = 0; i < COUNT_THREADS; ++i) {
                pThreadArray[i] = CreateThread(NULL, 0, thirdMulProc,
                                       (LPVOID) &channel, 0, 0);

    }
    WaitForMultipleObjects(COUNT_THREADS, pThreadArray, TRUE, INFINITE);
    for(int i = 0; i < COUNT_THREADS; i++){
        CloseHandle(pThreadArray[i]);
    }
    return data->resultMatrix;
}

int callMul(int** (*fncPtr)(LPVOID), MulData* data, int** &resultMatrix){
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
//    std::ios_base::sync_with_stdio(false);
//    std::cin.tie(nullptr);
    freopen("input.txt", "r", stdin);
//    freopen("output.txt", "w", stdout);
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

//    printMatrix(A, n, k);
//    printMatrix(B, k, m);
//    printMatrix(C0, n, m);
//    printMatrix(C1, n, m);
//    printMatrix(C2, n, m);

    std::cout << "mul0 : " << time0 << '\n';
    std::cout << "mul1 : " << time1 << '\n';
    std::cout << "mul2 : " << time2 << '\n';
    std::cout << std::boolalpha << (isEquals(C0, C1, n, m) && isEquals(C1, C2, n, m)) << '\n';
    deleteMatrix(A, n);
    deleteMatrix(B, k);
    deleteMatrix(C0, n);

}