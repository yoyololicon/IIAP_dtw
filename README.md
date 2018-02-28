# M-DTW : Multi-component Dynamic Time Warping

This is the final team project when I took the course ["Introduction to Interactive Audio Processing"](http://140.113.13.61/interactive/) at NCTU last semester, 
which is based on one of the course lab work. 
In the course we use DTW to compare the similarity between two pitch sequence.
The restriction is that the pitch should be monophonic, so we thought improving the system to adapt polyphonic pitch sequence would be a good topic.

## How to Build

```
cmake [where you want to put the file]
make
```

## Pitch Sequence Files
Before using the program, you should have prepared some sequence files.
We have already made some in the [sequence_file](sequence_file) directory, which are the pitch sequences estimated using _Deep Fourier Network_ (written as DDFT in the [report](final_report.pdf)) on a extended dataset we made from [bach10 dataset](http://music.cs.northwestern.edu/data/Bach10.html) 
.

_Deep Fourier Network_ is still being developed, but you can use other multi pitch estimation algorithm as well.
The format should look like this:
```
0
0
0
72
72
72
72
48 72
48 64 72
48 60 64 72
```
Each line represet one frame, each frame can have multiple pitch value (in midi number), or none, written as '0'.

Besides, file name should be ended with number, like 'myfile_01.seq', because our program will look at this number as ground truth to find the rank.
Same number represent same song.

## How to Use

```
./mdtw document_dir query_dir [search type]
```

* document_dir : where the document sequence files (.seq) are.
* query_dir : where the query sequence files (.seq) are.
* search type : 1 for Type-1 and 2 for Type-2. Default is 1.

The program will output each query's rank value and total MRR (Mean Reciprocal Rank).
```
For extract06.seq rank is 1
For extract05.seq rank is 1
For extract03.seq rank is 1
For extract10.seq rank is 1
For extract07.seq rank is 1
For extract08.seq rank is 1
For extract01.seq rank is 1
For extract02.seq rank is 1
For extract09.seq rank is 1
For extract04.seq rank is 1
Mean Reciprocal Rank is 1
```

For detail instructions and experiment result,  checkout our [report](final_report.pdf)(in Traditional Chinese).


## Environment

Only tested on ubuntu 16.04 LTS.