//
// Created by ycy on 1/9/18.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/dir.h>
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

int dtw_2(int **, int *, int, int **, int *, int);

int transp_dtw(int **, int *, int, int **, int *, int, bool);

int compute_dist_simple(int *, int, int *, int);

void Sort(int *, int *, int);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: document_dir query_dir [type]\n");
        exit(1);
    }

    bool type1 = true;
    if (argc == 4){
        if(atoi(argv[3]) == 2)
            type1 = false;
    }

    string doc_dir(argv[1]);
    string query_dir(argv[2]);

    //open document directory
    DIR* files = opendir(doc_dir.c_str());
    vector<string> doc_list;
    struct dirent* dp;
    while ((dp = readdir(files)) != nullptr) {
        size_t len = strlen(dp->d_name);
        if(len > 4 && strcmp(dp->d_name+len-4, ".seq") == 0)
            doc_list.emplace_back(string(dp->d_name));
    }
    closedir(files);

    int i, j;
    fstream fs;

    NumDocuments = min(MAX_DOCUMENTS_NUM, (int)doc_list.size());

    string document, line;
    stringstream parse;
    vector<int> buffer;

    for(i = 0; i < NumDocuments; i++){
        strcpy(DocName[i], doc_list[i].c_str());
        fs.open(doc_dir + "/" + doc_list[i], fstream::in);
        if (fs.fail()) {
            cout << "can't open file " << DocName[i];
            exit(1);
        }
        DocLength[i] = 0;
        while (getline(fs, line)) {
            parse << line;
            int n;
            while (parse >> n)
                buffer.push_back(n);
            sort(buffer.begin(), buffer.end());

            DocNoteLength[i][DocLength[i]] = int(buffer.size());
            DocNote[i][DocLength[i]] = new int[DocNoteLength[i][DocLength[i]]];
            copy(buffer.begin(), buffer.end(), DocNote[i][DocLength[i]]);
            DocLength[i]++;
            parse.clear();
            buffer.clear();
            if (DocLength[i] == MAX_DOCUMENTS_LENGTH) break;
        }
        fs.close();
    }

    //get query directory
    files = opendir(query_dir.c_str());
    vector<string> query_list;
    while ((dp = readdir(files)) != nullptr) {
        size_t len = strlen(dp->d_name);
        if(len > 4 && strcmp(dp->d_name+len-4, ".seq") == 0)
            query_list.emplace_back(string(dp->d_name));
    }
    closedir(files);

    /****************** load query's note sequence *******************/

    float MRR = 0;

    for(const auto &query : query_list){
        fs.open(query_dir + "/" + query, fstream::in);
        if (fs.fail()) {
            cout << "can't open file " << query;
            exit(1);
        }
        QueryLength = 0;
        while (getline(fs, line)) {
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
        fs.close();

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

        for (j = 0; j < NumDocuments; j++)
            DocScore[j] = transp_dtw(QueryNote, QueryNoteLength, QueryLength, DocNote[j], DocNoteLength[j], DocLength[j]
                    , type1);

        Sort(DocScore, DocRankList, NumDocuments);

        auto found = query.find(".seq");
        int number = stoi(query.substr(found-2, 2));
        for (j = 0; j < NumDocuments; j++) {
            //cout << DocName[DocRankList[j]] << " " << DocScore[DocRankList[j]] << endl;
            string rank(DocName[DocRankList[j]]);
            found = rank.find(".seq");
            if(number == stoi(rank.substr(found-2, 2))){
                cout << "For " << query << " rank is " << j+1 << endl;
                MRR += 1./(j+1);
                break;
            }
        }
    }

    cout << "Mean Reciprocal Rank is " << MRR/query_list.size() << endl;

    return 0;
}


int transp_dtw(int **test, int *test_vl, int test_length, int **reference, int *ref_vl, int ref_length, bool type1) {
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
    if(type1)
        return_dist = dtw(test, test_vl, test_length, reference, ref_vl, ref_length);
    else
        return_dist = dtw_2(test, test_vl, test_length, reference, ref_vl, ref_length);

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
            local_dist[i][j] = compute_dist_simple(reference[i], ref_vl[i], test[j], test_vl[j]);
            Distance[i][j] = INT_MAX/2;
        }
    }

    //initialize distance
    Distance[0][0] = local_dist[0][0];
    Distance[1][1] = Distance[0][0] + local_dist[1][1];
    Distance[1][2] = Distance[0][0] + local_dist[1][2];
    Distance[2][1] = Distance[0][0] + local_dist[2][1];

    //compute distance
    for (i = 2; i < ref_length; i++) {
        for (j = 2; j < test_length; j++) {
            Distance[i][j] = min(Distance[i-1][j-1], min(Distance[i-1][j-2], Distance[i-2][j-1])) + local_dist[i][j];
        }
    }

    //find minimum distance
    min_dist = Distance[ref_length-1][test_length-1];
    for (i = 0; i < ref_length; i++) {
        delete[] local_dist[i];
        delete[] Distance[i];
    }
    delete local_dist;
    delete Distance;

    return min_dist;
}

int dtw_2(int **test, int *test_vl, int test_length, int **reference, int *ref_vl, int ref_length) {
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

    //initialize distance
    Distance[0][0] = local_dist[0][0];
    for(i = 1; i < ref_length; i++)
        Distance[i][0] = Distance[i-1][0] + local_dist[i][0];
    for(j = 1; j < test_length; j++)
        Distance[0][j] = Distance[0][j-1] + local_dist[0][j];

    for (i = 1; i < ref_length; i++) {
        for (j = 1; j < test_length; j++) {
            Distance[i][j] = min(Distance[i-1][j-1], min(Distance[i][j-1], Distance[i-1][j])) + local_dist[i][j];
        }
    }

    min_dist = Distance[ref_length-1][test_length-1];
    for (i = 0; i < ref_length; i++) {
        delete[] local_dist[i];
        delete[] Distance[i];
    }
    delete local_dist;
    delete Distance;

    return min_dist;
}

int compute_dist_simple(int *value1, int l1, int *value2, int l2) {
    int minl = min(l1, l2), sum=0;
    for(int i = 0; i < minl; i++)
        sum += abs(value1[i] - value2[i]);
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
