#ifndef __LIBRVM__
#define __LIBRVM__

#include "rvm_internal.h"
#include <map>
#include <list>
#include <stdlib.h>

using namespace std;

typedef int trans_t;

//class mod_t: public _mod_read
class mod_t
{
	public:
	unsigned int segname_size;
	char* segname;
	// unsigned int offset;
	// unsigned int size;
	
	int offset;
	int size;
	
	//_mod_read modif;
	void* data_ptr;
	mod_t();
};

mod_t::mod_t(){
	segname = NULL;
	data_ptr = NULL;
	segname_size = 0;
	offset = 0;
	size=0;
}

class redo_t{
	public:
	unsigned int num_updates;
	std::list<mod_t> redo_log;
	//std::list<mod_t>::iterator ptr;
	
	redo_t();
};

redo_t::redo_t(){
	num_updates = 0;
	//ptr = redo_log.begin();
}

class trans_tt{
	public:
	unsigned int tid;
	unsigned int num_seg;
	char** seg_names;
	void** seg_bases;
	redo_t redo_obj;
	//std::list<segment_t> seg_cur;
	
	//std::map<char*,segment_t>::iterator seg_ptr;
	//bool valid;
	
	trans_tt();
};

trans_tt::trans_tt(){
	seg_names = NULL;
	seg_bases = NULL;
	tid = 0;
	num_seg = 0;
	
}

class segment_t{
	public:
	char* segname;
	unsigned int size;
	void* segbase;
	void* undo_log;			//Clear on transaction Commit/abort
	unsigned int tid;		//Clear on transaction Commit/abort
	trans_tt* trans;			//Clear to NULL on transaction COmmit/abort
	
	segment_t();
};

segment_t::segment_t(){
	segname = NULL;
	size = 0;
	segbase = NULL;
	undo_log = NULL;
	tid = 0;
	trans = NULL;
}

class rvm_tt{
	public:
	char* dirname;
	char* redo_file;
	unsigned int num_segs;
	std::map<char*,segment_t> seg_db;
	//std::map<char*,segment_t>::iterator seg_ptr;
	
	// std::map<unsigned int,trans_tt> trans;
	std::map<unsigned int,trans_tt*> trans;
	
	rvm_tt();
};

rvm_tt::rvm_tt(){
	dirname = NULL;
	redo_file = NULL;
	//dirname = malloc();
	//dirname = "rvm_dir";
	num_segs = 0;
}

typedef rvm_tt* rvm_t;

/*
typedef struct trans_tt{
	char** segnames;
	unsigned int id;
	void** segbases;
	int* seg_size;
	int* offsets;
	int* mod_size;
	int numseg;
	rvm_t dir;
	
}trans_tt;

struct _segment_t{
  char* segname;
  void* segbase;
  unsigned int size;
  trans_tt cur_trans;
  mod_t* mods;
};

struct _trans_tt{
  rvm_t rvm;          //The rvm to which the transaction belongs
  int numsegs;        //The number of segments involved in the transaction
  segment_t* segments;//The array of segments
};

//For redo//
struct segentry_t{
  char* segname;
  unsigned int segsize;
  unsigned int updatesize;
  unsigned int numupdates;
  unsigned int* offsets;
  unsigned int* sizes;
  void* data;
};

// The redo log //
struct _redo_t{
  unsigned int numentries;
  segentry_t* entries;
};

typedef struct rvm_t{
	char* dirname;
	unsigned int redo_cnt;
	seg
}rvm_t;


// rvm //
struct _rvm_t{
  char* prefix;   //The path to the directory holding the segments //
  int redofd;         //File descriptor for the redo-log //
  seqsrchst_t segst;  //A sequential search dictionary mapping base pointers to segment names // 
};
*/

rvm_tt* rvm_gb = NULL;

rvm_t rvm_init(const char *directory);
void *rvm_map(rvm_t rvm, const char *segname, int size_to_create);
void rvm_unmap(rvm_t rvm, void *segbase);
void rvm_destroy(rvm_t rvm, const char *segname);
trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases);
void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size);
void rvm_commit_trans(trans_t tid);
void rvm_abort_trans(trans_t tid);
void rvm_truncate_log(rvm_t rvm);

#endif
