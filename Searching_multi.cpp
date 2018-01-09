#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <climits>

using namespace std;

#define    MAX_DOCUMENTS_NUM        10
#define    MAX_DOCUMENTS_LENGTH    6000
#define    MAX_QUERY_LENGTH        6000

char DocName[MAX_DOCUMENTS_NUM][200];
int DocLength[MAX_DOCUMENTS_NUM], DocScore[MAX_DOCUMENTS_NUM], DocRankList[MAX_DOCUMENTS_NUM];
int *DocNote[MAX_DOCUMENTS_NUM][MAX_DOCUMENTS_LENGTH], *QueryNote[MAX_QUERY_LENGTH];
int DocNoteLength[MAX_DOCUMENTS_NUM][MAX_DOCUMENTS_LENGTH], QueryNoteLength[MAX_QUERY_LENGTH];
int QueryLength, QueryNonZeroLength;
int NumDocuments, sum, mean;

int dtw(int **, int *, int, int **, int *, int);

int dtw2(int **, int *, int, int **, int *, int);

int transp_dtw(int **, int *, int, int **, int *, int);

int compute_dist_all(int *, int, int *, int);

int compute_dist_simple(int *, int, int *, int);

void Sort(int *, int *, int);


int main(int argc, char *argv[]) {
    int i, j;
    fstream fs1, fs2;

    if (argc != 3) {
        printf("Usage: searching document_list query_note_sequence\n");
        exit(1);
    }
    /****************** load documents' note sequence *******************/

    NumDocuments = 0;
    fs1.open(argv[1], fstream::in);
    if (fs1.fail()) {
        cout << "can't open file " << argv[2];
        exit(1);
    }

    string document, line;
    stringstream parse;
    vector<int> buffer;

    while (getline(fs1, document)) {
        strcpy(DocName[NumDocuments], document.c_str());
        fs2.open(document, fstream::in);
        if (fs2.fail()) {
            cout << "can't open file " << DocName[NumDocuments];
            exit(1);
        }
        DocLength[NumDocuments] = 0;
        while (getline(fs2, line)) {
            parse << line;
            int n;
            while (parse >> n)
                buffer.push_back(n);
            sort(buffer.begin(), buffer.end());
            DocNoteLength[NumDocuments][DocLength[NumDocuments]] = int(buffer.size());
            DocNote[NumDocuments][DocLength[NumDocuments]] = new int[DocNoteLength[NumDocuments][DocLength[NumDocuments]]];
            copy(buffer.begin(), buffer.end(), DocNote[NumDocuments][DocLength[NumDocuments]]);
            DocLength[NumDocuments]++;
            parse.clear();
            buffer.clear();
            if (DocLength[NumDocuments] == MAX_DOCUMENTS_LENGTH) break;
        }
        NumDocuments++;
        fs2.close();
    }

    fs1.close();

    /****************** load query's note sequence *******************/
    fs1.open(argv[2], fstream::in);
    if (fs1.fail()) {
        cout << "can't open file " << argv[2];
        exit(1);
    }
    QueryLength = 0;
    while (getline(fs1, line)) {
        parse << line;
        int n;
        while (parse >> n)
            buffer.push_back(n);
        sort(buffer.begin(), buffer.end());
        QueryNoteLength[QueryLength] = int(buffer.size());
        QueryNote[QueryLength] = new int[QueryNoteLength[QueryLength]];
        copy(buffer.begin(), buffer.end(), QueryNote[QueryLength]);
        QueryLength++;
        parse.clear();
        buffer.clear();
        if (QueryLength == MAX_QUERY_LENGTH) break;
    }
    fs1.close();

    /*******************************************************************/

    sum = QueryNonZeroLength = 0;
    for (i = 0; i < QueryLength; i++) {
        for (j = 0; j < QueryNoteLength[i]; j++)
            if (QueryNote[i][j] != 0) {
                QueryNonZeroLength++;
                sum += QueryNote[i][j];
            }
    }
    mean = (int) round(float(sum) / QueryNonZeroLength);

    for (i = 0; i < QueryLength; i++)
        if (QueryNote[i][0] != 0)
            for (j = 0; j < QueryNoteLength[i]; j++)
                QueryNote[i][j] = QueryNote[i][j] - (mean - 60);

    //inspect here
    for (j = 0; j < NumDocuments; j++)
        DocScore[j] = transp_dtw(QueryNote, QueryNoteLength, QueryLength, DocNote[j], DocNoteLength[j], DocLength[j]);

    Sort(DocScore, DocRankList, NumDocuments);

    for (j = 0; j < NumDocuments; j++) {
        cout << DocName[DocRankList[j]] << " " << DocScore[DocRankList[j]] << endl;
    }

    return 0;
}

int transp_dtw(int **test, int *test_vl, int test_length, int **reference, int *ref_vl, int ref_length) {
    int i, j, ref_sum = 0, ref_mean = 0;
    int return_dist, min_dist = INT_MAX/2;

    int NonZeroLength = 0;
    for (i = 0; i < ref_length; i++) {
        for (j = 0; j < ref_vl[i]; j++) {
            if (reference[i][j] != 0) {
                NonZeroLength++;
                ref_sum += reference[i][j];
            }
        }
    }
    ref_mean = (int) round(float(ref_sum) / NonZeroLength);

    for (i = 0; i < ref_length; i++)
        for (j = 0; j < ref_vl[i]; j++) {
            if (reference[i][j] != 0) {
                reference[i][j] = reference[i][j] - (ref_mean - 60);
            }
        }
    return_dist = dtw(test, test_vl, test_length, reference, ref_vl, ref_length);

    if (return_dist < min_dist)
        min_dist = return_dist;

    return min_dist;

}

int dtw(int **test, int *test_vl, int test_length, int **reference, int *ref_vl, int ref_length) {
    int i, j;
    int **local_dist, **Distance;
    int min_dist = INT_MAX/2;

    local_dist = new int *[ref_length];
    for (i = 0; i < ref_length; i++)
        local_dist[i] = new int[test_length];
    Distance = new int *[ref_length];
    for (i = 0; i < ref_length; i++)
        Distance[i] = new int[test_length];
    for (i = 0; i < ref_length; i++) {
        for (j = 0; j < test_length; j++) {
            local_dist[i][j] = INT_MAX/2;
            Distance[i][j] = INT_MAX/2;
        }
    }

    //here
    for (i = 0; i < ref_length / 2; i++) {
        local_dist[i][0] = compute_dist_simple(test[0], test_vl[0], reference[i], ref_vl[i]);
        Distance[i][0] = local_dist[i][0];
    }

    for (j = 0; j < test_length / 2; j++) {
        local_dist[0][j] = compute_dist_simple(test[j], test_vl[j], reference[0], ref_vl[0]);
        Distance[0][j] = local_dist[0][j];
    }

    for (i = 1; i < ref_length; i++) {
        for (j = 1; j < test_length; j++) {
            local_dist[i][j] = compute_dist_simple(reference[i], ref_vl[i], test[j], test_vl[j]);
        }
    }

    for (i = 2; i < ref_length / 2; i++)
        Distance[i][1] = min(Distance[i - 2][0], Distance[i - 1][0]) + local_dist[i][1];
    for (j = 2; j < test_length / 2; j++)
        Distance[1][j] = min(Distance[0][j - 2], Distance[0][j - 1]) + local_dist[1][j];

    Distance[1][1] = Distance[0][0] + local_dist[1][1];

    for (i = 2; i < ref_length; i++) {
        for (j = 2; j < test_length; j++) {
            if ((Distance[i - 1][j - 2] < Distance[i - 1][j - 1]) && (Distance[i - 1][j - 2] < Distance[i - 2][j - 1]))
                Distance[i][j] = Distance[i - 1][j - 2] + local_dist[i][j];
            else if ((Distance[i - 1][j - 1] < Distance[i - 1][j - 2]) &&
                     (Distance[i - 1][j - 1] < Distance[i - 2][j - 1]))
                Distance[i][j] = Distance[i - 1][j - 1] + local_dist[i][j];
            else
                Distance[i][j] = Distance[i - 2][j - 1] + local_dist[i][j];
        }
    }

    for (i = (ref_length / 2); i < ref_length; i++)
        min_dist = min(min_dist, Distance[i][test_length - 1]);
    for (j = (test_length / 2); j < test_length; j++)
        min_dist = min(min_dist, Distance[ref_length - 1][j]);

    for (i = 0; i < ref_length; i++) {
        delete[] local_dist[i];
        delete[] Distance[i];
    }
    delete local_dist;
    delete Distance;

    return min_dist;
}

int dtw2(int **test, int *test_vl, int test_length, int **reference, int *ref_vl, int ref_length) {
    int i, j;
    int **local_dist, **Distance;
    int min_dist = INT_MAX/2;

    local_dist = new int *[ref_length];
    for (i = 0; i < ref_length; i++)
        local_dist[i] = new int[test_length];
    Distance = new int *[ref_length];
    for (i = 0; i < ref_length; i++)
        Distance[i] = new int[test_length];
    for (i = 0; i < ref_length; i++) {
        for (j = 0; j < test_length; j++) {
            local_dist[i][j] = compute_dist_simple(reference[i], ref_vl[i], test[j], test_vl[j]);
            Distance[i][j] = INT_MAX/2;
        }
    }

    //here
    for (i = 0; i < ref_length / 2; i++) {
        Distance[i][0] = local_dist[i][0];
    }

    for (j = 1; j < test_length / 2; j++) {
        Distance[0][j] = local_dist[0][j];
    }

    for (i = 1; i < ref_length; i++) {
        for (j = 1; j < test_length; j++) {
            if (test[j][0] == 0)
                local_dist[i][j] = 1;
            else
                local_dist[i][j] = compute_dist_simple(reference[i], ref_vl[i], test[j], test_vl[j]);
        }
    }

    for (i = 2; i < ref_length / 2; i++)
        Distance[i][1] = min(Distance[i - 2][0], Distance[i - 1][0]) + local_dist[i][1];
    for (j = 2; j < test_length / 2; j++)
        Distance[1][j] = min(Distance[0][j - 2], Distance[0][j - 1]) + local_dist[1][j];

    Distance[1][1] = Distance[0][0] + local_dist[1][1];

    for (i = 2; i < ref_length; i++) {
        for (j = 2; j < test_length; j++) {
            if ((Distance[i - 1][j - 2] < Distance[i - 1][j - 1]) && (Distance[i - 1][j - 2] < Distance[i - 2][j - 1]))
                Distance[i][j] = Distance[i - 1][j - 2] + local_dist[i][j];
            else if ((Distance[i - 1][j - 1] < Distance[i - 1][j - 2]) &&
                     (Distance[i - 1][j - 1] < Distance[i - 2][j - 1]))
                Distance[i][j] = Distance[i - 1][j - 1] + local_dist[i][j];
            else
                Distance[i][j] = Distance[i - 2][j - 1] + local_dist[i][j];
        }
    }

    for (i = (ref_length / 2); i < ref_length; i++)
        min_dist = min(min_dist, Distance[i][test_length - 1]);
    for (j = (test_length / 2); j < test_length; j++)
        min_dist = min(min_dist, Distance[ref_length - 1][j]);

    for (i = 0; i < ref_length; i++) {
        delete[] local_dist[i];
        delete[] Distance[i];
    }
    delete local_dist;
    delete Distance;

    return min_dist;
}

int compute_dist_all(int *value1, int l1, int *value2, int l2) {
    vector<int> v1(value1, value1 + l1);
    vector<int> v2(value2, value2 + l2);;
    int maxl = max(l1, l2);
    if (l1 < maxl)
        for (int i = 0; i < maxl - l1; i++)
            v1.push_back(0);
    else if (l2 < maxl)
        for (int i = 0; i < maxl - l2; i++)
            v2.push_back(0);
    int min = INT_MAX/2, sum = 0;

    do {
        for (int i = 0; i < maxl; i++)
            sum += abs(v1[i] - v2[i]);
        if (sum < min)
            min = sum;
        sum = 0;
    } while (next_permutation(v2.begin(), v2.end()));

    return min;
}

int compute_dist_simple(int *value1, int l1, int *value2, int l2) {
    int maxl = max(l1, l2), sum=0;
    for(int i = 0; i < maxl; i++){
        if (i >= l1)
            sum += value2[i];
        else if( i >= l2)
            sum += value1[i];
        else
            sum += abs(value1[i] - value2[i]);
    }
    return sum;
}

void Sort(int *A, int *rank, int No) {
    int B[50000], temp;
    int i, j, temp2;

    for (i = 0; i < No; i++) {
        B[i] = A[i];
        rank[i] = i;
    }
    for (i = 0; i < No; i++) {
        for (j = 0; j < No; j++) {
            if (B[i] <= B[j]) {
                temp = B[i];
                B[i] = B[j];
                B[j] = temp;
                temp2 = rank[i];
                rank[i] = rank[j];
                rank[j] = temp2;
            }
        }
    }
}