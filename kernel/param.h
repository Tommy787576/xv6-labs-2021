#define NPROC        64  // maximum number of processes
#define NCPU          8  // maximum number of CPUs
#define NOFILE       16  // open files per process
#define NFILE       100  // open files per system
#define NINODE       50  // maximum number of active i-nodes
#define NDEV         10  // maximum major device number
#define ROOTDEV       1  // device number of file system root disk
#define MAXARG       32  // max exec arguments
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGSIZE      (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache
// https://github.com/mit-pdos/xv6-riscv/issues/59
// to resolve test writebig: panic: balloc: out of blocks issue
#ifdef LAB_FS
#define FSSIZE       200000  // size of file system in blocks
#else
#ifdef LAB_LOCK
#define FSSIZE       10000  // size of file system in blocks
#else
#define FSSIZE       2000   // size of file system in blocks
#endif
#endif
// #define FSSIZE       1000  // size of file system in blocks
#define MAXPATH      128   // maximum file path name
#define BUCKETCNT    13    // bucket count of disk block cache
#define BUCKETNBUF   (MAXOPBLOCKS*2)  // size of disk block cache per each bucket 