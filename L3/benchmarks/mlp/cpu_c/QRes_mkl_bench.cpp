/*
 * Copyright 2019 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define USE_DOUBLE_PRECISION
#include <fstream>
#include <cstdlib>
#include "QRes_mkl_helper.hpp"

// Two layer NN structure
struct TwoLayerNN {
    double NNinputs;
    double L1size;
    double L2size;
    double NNoutputs;
    XFBLAS_dataType* IW;
    XFBLAS_dataType* b1;
    XFBLAS_dataType* LW21;
    XFBLAS_dataType* b2;
    XFBLAS_dataType* LW32;
    XFBLAS_dataType* b3;
};

#define LOOP 4
using namespace std;

int main(int argc, char** argv) {
    if (argc < 4) {
        printf("Usage: QRes_mkl_bench2 DataFolder NumberOfTraces \n");
        return EXIT_FAILURE;
    }

    ofstream outFile;
    string data_dir(argv[1]), NNfile = "NN356.bin", InputsFile = "NN356inputs.bin", OutputsFile = "NN356outputs.bin";
    string ssNN, ssNNin, ssNNou;
    ssNN = data_dir + NNfile;
    ssNNin = data_dir + InputsFile;
    ssNNou = data_dir + OutputsFile;

    setenv("OMP_NUM_THREADS", argv[3], 1);

    int NumberOfTraces = atoi(argv[2]);
    int LoopNumberOfTraces[] = {39226,   74886,   110546,  146206,  181866,  217526,  253186,  288846,  324506,
                                360166,  395826,  431486,  467146,  502806,  538466,  574126,  609786,  645446,
                                681106,  716766,  752426,  788086,  823746,  859406,  895066,  930726,  966386,
                                1002046, 1037706, 1073366, 1109026, 1144686, 1180346, 1216006, 1251666, 1287326,
                                1322986, 1358646, 1394306, 1429966, 1465626, 1501286, 1536946, 1572606, 1608266,
                                1643926, 1679586, 1715246, 1750906, 1786566};
    int LoopCount = 1;
    // Execute Loop if number of traces is -1
    if (NumberOfTraces == -1)
        LoopCount = 50;
    else
        LoopNumberOfTraces[0] = NumberOfTraces;

    // XFBLAS_dataType *a, *b, *bt, *c, *d;
    XFBLAS_dataType *L1temp, *L2temp, *Otemp, *InputTable, *InputTable2, *OutputTable, *OnesTraces, alpha = 1.,
                                                                                                    beta = 1.;

    // Variables for NN models
    long long int NNnumber, NNinputs, L1size, L2size, NNoutputs, NNinputsData, NNtraces, NNOutputsData, NNtracesOut;
    struct TwoLayerNN* TwoLayerCase;

    streampos size;
    double *memblock, *IW, *b1, *LW12, *b2, *LW32, *b3;
    // ifstream inFile ("C:\\Users\\Orlando\\Documents\\MATLAB\\NN356.bin", ios::in | ios::binary | ios::ate);
    ifstream inFile(std::string(ssNN), ios::in | ios::binary | ios::ate);

    if (inFile.is_open()) {
        size = inFile.tellg();
        memblock = new double[size];
        inFile.seekg(0, ios::beg);
        inFile.read((char*)memblock, size);
        inFile.close();

        cout << "the entire NN file content is in memory\n";

        NNnumber = memblock[0];
        TwoLayerCase = new TwoLayerNN[NNnumber];
        NNinputs = memblock[1];
        L1size = memblock[2];
        L2size = memblock[3];
        NNoutputs = memblock[4];

        cout << "Two layer NN set," << NNinputs << " inputs, L1 size = " << L1size << ", L2 Size = " << L2size
             << ", Outputs = " << NNoutputs << "\n";
        cout << "Number of traces = " << NumberOfTraces << "\n";

        int kindex = 5;

        for (int kn = 0; kn <= NNnumber - 1; kn++) {
            TwoLayerCase[kn].NNinputs = NNinputs;
            TwoLayerCase[kn].L1size = L1size;
            TwoLayerCase[kn].L2size = L2size;
            TwoLayerCase[kn].NNoutputs = NNoutputs;
            TwoLayerCase[kn].IW = createMat(L1size, NNinputs);    // new double[NNinputs*L1size];
            TwoLayerCase[kn].b1 = createMat(L1size, 1);           // new double[L1size];
            TwoLayerCase[kn].LW21 = createMat(L2size, L1size);    // new double[L1size*L2size];
            TwoLayerCase[kn].b2 = createMat(L2size, 1);           // new double[L2size];
            TwoLayerCase[kn].LW32 = createMat(NNoutputs, L2size); // new double[L2size*NNoutputs];
            TwoLayerCase[kn].b3 = createMat(NNoutputs, 1);        // new double[NNoutputs];
            // Reading IW
            for (int k3 = 0; k3 < NNinputs; k3++)   // Columns
                for (int k2 = 0; k2 < L1size; k2++) // Rows
                {
                    TwoLayerCase[kn].IW[k2 * (int)NNinputs + k3] =
                        memblock[kindex++]; // matrices saved by column in Matlab, by row in Intel
                }
            // Reading b1
            for (int k2 = 0; k2 < L1size; k2++) {
                TwoLayerCase[kn].b1[k2] = memblock[kindex++]; // matrices saved by column
            }
            // Reading LW21
            for (int k3 = 0; k3 < L1size; k3++)     // Columns
                for (int k2 = 0; k2 < L2size; k2++) // Rows
                {
                    TwoLayerCase[kn].LW21[k2 * (int)L1size + k3] = memblock[kindex++]; // matrices saved by column
                }
            // Reading b2
            for (int k2 = 0; k2 < L2size; k2++) {
                TwoLayerCase[kn].b2[k2] = memblock[kindex++]; // matrices saved by column
            }
            // Reading LW32
            for (int k3 = 0; k3 < L2size; k3++) // Columns
                for (int k2 = 0; k2 < NNoutputs; k2++) {
                    TwoLayerCase[kn].LW32[k2 * (int)L2size + k3] = memblock[kindex++]; // matrices saved by column
                }
            // Reading b3
            for (int k2 = 0; k2 < NNoutputs; k2++) {
                TwoLayerCase[kn].b3[k2] = memblock[kindex++]; // matrices saved by column
            }
        }

        delete[] memblock;
    } else {
        cout << "Unable to open NN file\n";
        return EXIT_FAILURE;
    }

    for (int ll = 0; ll < LoopCount; ll++) {
        NumberOfTraces = LoopNumberOfTraces[ll];

        // ifstream inFilein("C:\\Users\\Orlando\\Documents\\MATLAB\\NN356inputs.bin", ios::in | ios::binary |
        // ios::ate);
        ifstream inFilein(ssNNin, ios::in | ios::binary | ios::ate);
        if (inFilein.is_open()) {
            size = inFilein.tellg();
            memblock = new double[2];
            inFilein.seekg(0, ios::beg);
            inFilein.read((char*)memblock, 16);
            // inFilein.close();

            cout << "the entire input file content is in memory\n";

            NNinputsData = memblock[0];
            NNtraces = memblock[1];

            delete[] memblock;
            if (NumberOfTraces < NNtraces) {
                memblock = new double[NumberOfTraces * NNinputsData];
                NNtraces = NumberOfTraces;
            } else
                memblock = new double[size / 8];

            // inFilein.seekg(0, ios::beg);
            inFilein.read((char*)memblock, NumberOfTraces * NNinputsData * 8);
            inFilein.close();

            cout << "Input Data," << NNinputsData << " inputs," << NNtraces << " traces\n";

            InputTable = createMat(NNtraces, NNinputsData);
            int kr = 0, kc = 0;
            // Reading Input Data. Saved by rows
            for (int k2 = 0, k4 = 0; k2 < NNtraces; k2++) // Rows
                for (int k3 = 0; k3 < NNinputsData; k3++) // Columns
                {
                    InputTable[k2 + (int)NNtraces * k3] = memblock[k4++];
                }

            if (DUMP_RESULT) {
                outFile.open(data_dir + "matInputTable_" + ".bin", ofstream::binary);
                outFile.write((char*)InputTable, sizeof(XFBLAS_dataType) * NNinputsData * NNtraces);
                outFile.close();
            }

            // row vector with ones to be used in calculation later
            OnesTraces = createMat(1, NNtraces, false, 1.0);

            if (DUMP_RESULT) {
                outFile.open(data_dir + "matOnesTraces_in_" + ".bin", ofstream::binary);
                outFile.write((char*)OnesTraces, sizeof(XFBLAS_dataType) * NNtraces * 1);
                outFile.close();
            }

            delete[] memblock;
        } else {
            cout << "Unable to open inputs file\n";
            return EXIT_FAILURE;
        }

        ifstream inFileout(ssNNou, ios::in | ios::binary | ios::ate);
        if (inFileout.is_open()) {
            size = inFileout.tellg();
            memblock = new double[2];
            inFileout.seekg(0, ios::beg);
            inFileout.read((char*)memblock, 16);
            // inFileout.close();

            cout << "the entire output file content is in memory\n";

            NNOutputsData = memblock[0];
            NNtracesOut = memblock[1];

            delete[] memblock;
            if (NumberOfTraces < NNtracesOut) {
                memblock = new double[NumberOfTraces * NNOutputsData];
                NNtracesOut = NumberOfTraces;
            } else
                memblock = new double[size / 8];

            // inFileout.seekg(0, ios::beg);
            inFileout.read((char*)memblock, NumberOfTraces * NNOutputsData * 8);
            inFileout.close();

            cout << "output Data," << NNOutputsData << " outputs," << NNtracesOut << " traces\n";

            OutputTable = createMat(NNtracesOut, NNOutputsData);
            int kr = 0, kc = 0;
            // Reading Input Data. Saved by rows
            for (int k2 = 0, k4 = 0; k2 < NNtracesOut; k2++) // Rows
                for (int k3 = 0; k3 < NNOutputsData; k3++)   // Columns
                {
                    OutputTable[k2 * (int)NNOutputsData + k3] = memblock[k4++];
                }

            if (DUMP_RESULT) {
                outFile.open(data_dir + "matOutputTable_" + ".bin", ofstream::binary);
                outFile.write((char*)OutputTable, sizeof(XFBLAS_dataType) * NNOutputsData * NNtracesOut);
                outFile.close();
            }

            delete[] memblock;
        } else {
            cout << "Unable to open inputs file\n";
            return EXIT_FAILURE;
        }

        // Create temp layer results
        L1temp = createMat(L1size, NNtraces);
        L2temp = createMat(L2size, NNtraces);
        Otemp = createMat(NNoutputs, NNtraces);

        TimePointType l_tp[3];

        // Cold Start
        l_tp[0] = chrono::high_resolution_clock::now();
        for (int kn = 0; kn <= NNnumber - 1; kn++) {
            // Set temporary variables to zero
            for (int kn2 = 0; kn2 < L1size * NNtraces; kn2++) L1temp[kn2] = 0.0;
            for (int kn2 = 0; kn2 < L2size * NNtraces; kn2++) L2temp[kn2] = 0.0;
            for (int kn2 = 0; kn2 < NNoutputs * NNtraces; kn2++) Otemp[kn2] = 0.0;

            // Create copies of b1 based on number of traces
            if (DUMP_RESULT) {
                outFile.open(data_dir + "matb1_in_" + to_string(kn) + ".bin", ofstream::binary);
                outFile.write((char*)TwoLayerCase[kn].b1, sizeof(XFBLAS_dataType) * L1size * 1);
                outFile.close();
            }

            GEMM_MKL(L1size, NNtraces, 1, alpha, beta, TwoLayerCase[kn].b1, OnesTraces,
                     L1temp); // Copy bias1 with number of traces

            if (DUMP_RESULT) {
                outFile.open(data_dir + "matb1_L1temp_in_" + to_string(kn) + ".bin", ofstream::binary);
                outFile.write((char*)L1temp, sizeof(XFBLAS_dataType) * L1size * NNtraces);
                outFile.close();

                // Execute IW * InputsTable' + b1
                outFile.open(data_dir + "matW1_" + to_string(kn) + ".bin", ofstream::binary);
                outFile.write((char*)TwoLayerCase[kn].IW, sizeof(XFBLAS_dataType) * L1size * NNinputs);
                outFile.close();
            }
            GEMM_MKL(L1size, NNtraces, NNinputs, alpha, beta, TwoLayerCase[kn].IW, InputTable,
                     L1temp); // Executes IW * inputs' + b1

            if (DUMP_RESULT) {
                outFile.open(data_dir + "matIW_L1temp_in_" + to_string(kn) + ".bin", ofstream::binary);
                outFile.write((char*)L1temp, sizeof(XFBLAS_dataType) * L1size * NNtraces);
                outFile.close();
            }

#if SIGMOID == 1
            sigmoid(L1size * NNtraces, L1temp, L1temp);
#else
            tansig(L1size * NNtraces, L1temp, L1temp);
#endif

            if (DUMP_RESULT) {
                outFile.open(data_dir + "matTansig_L1temp_in_" + to_string(kn) + ".bin", ofstream::binary);
                outFile.write((char*)L1temp, sizeof(XFBLAS_dataType) * L1size * NNtraces);
                outFile.close();

                // Create copies of b2 based on number of traces
                outFile.open(data_dir + "matb2_in_" + to_string(kn) + ".bin", ofstream::binary);
                outFile.write((char*)TwoLayerCase[kn].b2, sizeof(XFBLAS_dataType) * L2size * 1);
                outFile.close();
            }

            GEMM_MKL(L2size, NNtraces, 1, alpha, beta, TwoLayerCase[kn].b2, OnesTraces,
                     L2temp); // Copy bias2 with number of traces

            if (DUMP_RESULT) {
                outFile.open(data_dir + "matb2_L2temp_in_" + to_string(kn) + ".bin", ofstream::binary);
                outFile.write((char*)L2temp, sizeof(XFBLAS_dataType) * L2size * NNtraces);
                outFile.close();

                // Execute LW21 * n1 + b2
                outFile.open(data_dir + "matW2_" + to_string(kn) + ".bin", ofstream::binary);
                outFile.write((char*)TwoLayerCase[kn].LW21, sizeof(XFBLAS_dataType) * L2size * L1size);
                outFile.close();
            }

            GEMM_MKL(L2size, NNtraces, L1size, alpha, beta, TwoLayerCase[kn].LW21, L1temp,
                     L2temp); // Executes LW21 * L1out + b2

            if (DUMP_RESULT) {
                outFile.open(data_dir + "matLW21_L2temp_in_" + to_string(kn) + ".bin", ofstream::binary);
                outFile.write((char*)L2temp, sizeof(XFBLAS_dataType) * L2size * NNtraces);
                outFile.close();
            }

#if SIGMOID == 1
            sigmoid(L2size * NNtraces, L2temp, L2temp);
#else
            tansig(L2size * NNtraces, L2temp, L2temp);
#endif

            if (DUMP_RESULT) {
                outFile.open(data_dir + "matTansig_L2temp_in_" + to_string(kn) + ".bin", ofstream::binary);
                outFile.write((char*)L2temp, sizeof(XFBLAS_dataType) * L2size * NNtraces);
                outFile.close();

                // Create copies of b3 based on number of traces
                outFile.open(data_dir + "matb3_in_" + to_string(kn) + ".bin", ofstream::binary);
                outFile.write((char*)TwoLayerCase[kn].b3, sizeof(XFBLAS_dataType) * NNoutputs * 1);
                outFile.close();
            }

            GEMM_MKL(NNoutputs, NNtraces, 1, alpha, beta, TwoLayerCase[kn].b3, OnesTraces,
                     Otemp); // Copy bias3 with number of traces

            if (DUMP_RESULT) {
                outFile.open(data_dir + "matb3_Otemp_in_" + to_string(kn) + ".bin", ofstream::binary);
                outFile.write((char*)Otemp, sizeof(XFBLAS_dataType) * NNoutputs * NNtraces);
                outFile.close();

                // Execute LW32 * n2 + b3
                outFile.open(data_dir + "matW3_" + to_string(kn) + ".bin", ofstream::binary);
                outFile.write((char*)TwoLayerCase[kn].LW32, sizeof(XFBLAS_dataType) * NNoutputs * L2size);
                outFile.close();
            }

            GEMM_MKL(NNoutputs, NNtraces, L2size, alpha, beta, TwoLayerCase[kn].LW32, L2temp,
                     Otemp); // Executes LW32 * L2out + b3

            if (DUMP_RESULT) {
#if SIGMOID == 1
                outFile.open(data_dir + "mat_sigmoid_output_" + to_string(kn) + ".bin", ofstream::binary);
                outFile.write((char*)Otemp, sizeof(XFBLAS_dataType) * NNoutputs * NNtraces);
                outFile.close();
#else
                outFile.open(data_dir + "matOtemp_in_" + to_string(kn) + ".bin", ofstream::binary);
                outFile.write((char*)Otemp, sizeof(XFBLAS_dataType) * NNoutputs * NNtraces);
                outFile.close();

                double minDiff = 1e30, maxDiff = -1e30, tempDiff;
                // Calculate Maximum and Minumum errors
                for (int k3 = kn * 3, k4 = 0; k3 < (kn + 1) * 3; k3++) // Columns
                    for (int k2 = 0; k2 < NNtracesOut; k2++)           // Rows
                    {
                        tempDiff = OutputTable[k2 * (int)NNOutputsData + k3] - Otemp[k4++];
                        if (tempDiff < minDiff) minDiff = tempDiff;
                        if (tempDiff > maxDiff) maxDiff = tempDiff;
                    }
                cout << "Error NN " << kn << ": min = " << minDiff << "  max = " << maxDiff << "\n";
#endif
            }
        }

        /*
        COPY_MKL(m, n, alpha, c, d);
        EXP_MKL(n*m, d, d);
        GEMM_MKL(m, k, n, alpha, beta, a, b, d);
        */
        l_tp[1] = chrono::high_resolution_clock::now();

        // Hot benchmarking
        // for (int i = 0; i < LOOP; i++)
        //{
        // COPY_MKL(m, n, alpha, c, d);
        // GEMM_MKL(m, k, n, alpha, beta, a, b, d);
        //}
        // l_tp[2] = chrono::high_resolution_clock::now();

        chrono::duration<double> l_durationSec_cold = l_tp[1] - l_tp[0];
        // chrono::duration<double> l_durationSec_bench = l_tp[2] - l_tp[1];

        /* Calculation of Flops of Two layer NN
         Copy bias1 with number of traces =  2 * L1size * NNtraces
         IW * inputs' + b1 = 2 * L1size * NNtraces * NNinputs
         Tansig L1 = 102 * L1size * NNtraces
         Copy bias2 with number of traces =  2 * L2size * NNtraces
         LW21 * n1 + b2 = 2 * L2size * NNtraces * L1size
         Tansig L2 = 102 * L2size * NNtraces
         Copy bias3 with number of traces =  2 * NNoutputs * NNtraces
         LW21 * n1 + b2 = 2 * NNoutputs * NNtraces * L2size
        */
        double flops = (2 * L1size * NNtraces + 2. * L1size * NNtraces * NNinputs + 102. * L1size * NNtraces +
                        2. * L2size * NNtraces + 2. * L2size * NNtraces * L1size + 102. * L2size * NNtraces +
                        2. * NNoutputs * NNtraces + 2. * NNoutputs * NNtraces * L2size) *
                       NNnumber;

        cout << std::string("DATA_CSV:,Type,Thread,Func,Traces,Inputs,L1,L2,Outputs,") + "TimeApiSeconds," +
                    "EffApiPct,PerfApiTops\n";
        /*cout << std::string("DATA_CSV:,Type,Thread,Func,M,K,N,")
                 << "TimeApiMs,"
                 << "EffApiPct,PerfApiTops\n";*/
        cout << "DATA_CSV:,"
             << "Cold Start," << getenv("OMP_NUM_THREADS") << "," << DISPLAY_GEMM_FUNC << "," << NNtraces << ","
             << NNinputs << "," << L1size << "," << L2size << "," << NNoutputs
             /*<< "Cold Start," << "N/A" << "," << DISPLAY_GEMM_FUNC << "," << m << "," << k << "," << n*/
             << "," << ((double)l_durationSec_cold.count()) << ","
             << "N/A," << (flops / (double)l_durationSec_cold.count() * 1.e-12) << endl;
// cout << "DATA_CSV:,"
//     << "Benchmark," << getenv("OMP_NUM_THREADS") << "," << DISPLAY_GEMM_FUNC << "," << m << "," << k << "," << n
//	 /*<< "Benchmark," << "N/A" << "," << DISPLAY_GEMM_FUNC << "," << m << "," << k << "," << n*/
//	 << "," << ((double)l_durationSec_bench.count() / (double)LOOP * 1e3) << ","
//     << "N/A," << (flops / (double)l_durationSec_bench.count() * (double)LOOP * 1.e-12) << endl;

#ifdef WINDOWS_SYSTEM
        _aligned_free(InputTable);
        _aligned_free(OutputTable);
        _aligned_free(OnesTraces);
        _aligned_free(L1temp);
        _aligned_free(L2temp);
        _aligned_free(Otemp);
#else
        free(InputTable);
        free(OutputTable);
        free(OnesTraces);
        free(L1temp);
        free(L2temp);
        free(Otemp);
#endif
    }

    // Release memory of NNs
    for (int kn = 0; kn <= NNnumber - 1; kn++) {
#ifdef WINDOWS_SYSTEM
        _aligned_free(TwoLayerCase[kn].IW);
        _aligned_free(TwoLayerCase[kn].b1);
        _aligned_free(TwoLayerCase[kn].LW21);
        _aligned_free(TwoLayerCase[kn].b2);
        _aligned_free(TwoLayerCase[kn].LW32);
        _aligned_free(TwoLayerCase[kn].b3);
#else
        free(TwoLayerCase[kn].IW);
        free(TwoLayerCase[kn].b1);
        free(TwoLayerCase[kn].LW21);
        free(TwoLayerCase[kn].b2);
        free(TwoLayerCase[kn].LW32);
        free(TwoLayerCase[kn].b3);
#endif
    }

    return 0;
}
