#ifndef __LIBRVM__
#define __LIBRVM__

#include "rvm_internal.h"
#include <map>
#include <list>
#include <stdlib.h>

using namespace std;
typedef int trans_t;

class mod_t
{
	public:
	unsigned int segname_size;
	char* segname;
	int offset;
	int size;
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
	redo_t();
};

redo_t::redo_t(){
	num_updates = 0;
}

class trans_tt{
	public:
	unsigned int tid;
	unsigned int num_seg;
	char** seg_names;
	void** seg_bases;
	redo_t redo_obj;
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
	std::map<unsigned int,trans_tt*> trans;
	rvm_tt();
};

rvm_tt::rvm_tt(){
	dirname = NULL;
	redo_file = NULL;
	num_segs = 0;
}

typedef rvm_tt* rvm_t;
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