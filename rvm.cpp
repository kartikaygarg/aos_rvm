#include "rvm.h"
#include <sys/stat.h>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string.h>

unsigned int tid_cnt = 0;
using namespace std;

//Creates A DIRECTORY with specified name where persistent memory space (extrenal data segment,log files) shall be maintained
rvm_t rvm_init(const char *directory){
	struct stat st;
	rvm_tt* rvm_dir = NULL;
	rvm_dir = (rvm_tt*)malloc(sizeof(rvm_tt));
	new (rvm_dir) rvm_tt();
	if(mkdir(directory, 0700) != 0){	//dir not created
		stat(directory,&st);
		//if((chmod(directory,(st.st_mode|700))== EPERM) || (chmod(directory,(st.st_mode|700))== EROFS)){
		if(chmod(directory,(st.st_mode|0700))== -1){	
			printf("Cannot create directory here. Check user permissions or Read-Only flag on memory space.");
			rvm_dir->dirname = NULL;
			return rvm_dir;
		}
	}
	rvm_dir->dirname = (char*) malloc (strlen(directory)+2);
	strcpy(rvm_dir->dirname,directory);
	strcat(rvm_dir->dirname,"/");
	rvm_dir->dirname[strlen(directory)+1] = '\0';
	rvm_dir->redo_file = (char*)malloc( sizeof(char) * ( strlen("redo1.txt") +1) );
	strcpy(rvm_dir->redo_file, "redo1.txt");
	rvm_dir->redo_file[strlen("redo1.txt")] = '\0';
	rvm_gb = rvm_dir;
	rvm_truncate_log(rvm_dir);
	return rvm_dir;	
}

char* dir_prefix(rvm_t rvm, char* segname){
	if(segname == NULL){
		return NULL;
	}
	char* prefix = NULL;
	prefix = (char*)malloc(sizeof(char) * ( strlen(rvm->dirname)+ (strlen(segname) +1 )  ) );
	strcpy(prefix, rvm->dirname);
	strcat(prefix,segname);
	prefix[strlen(rvm->dirname) + strlen(segname)] = '\0';
	return prefix;
}	

void rvm_truncate_log(rvm_t rvm){
	
	FILE *redo_seg = NULL;
	FILE *temp = NULL;
	char * file_name = NULL;
	unsigned int i = 0;
	file_name = dir_prefix(rvm,rvm->redo_file);
	redo_seg = fopen(file_name,"rb");
	if(redo_seg == NULL){
		free(file_name);
		file_name = NULL;
		return;
	}	
	redo_t redo1;
	mod_t mod1;
	while(fgetc(redo_seg)!= EOF){	
		fseek(redo_seg,-1,SEEK_CUR);
		++(redo1.num_updates);
		fread((void*) (&(mod1.segname_size)) , sizeof(unsigned int), 1, redo_seg );
		mod1.segname = (char*) malloc(sizeof(char) * mod1.segname_size);
		fread((void*) (mod1.segname) , sizeof(char), (mod1.segname_size), redo_seg );
		mod1.segname[mod1.segname_size -1] = '\0';
		fread((void*) (&(mod1.offset)) , sizeof(int), 1, redo_seg );
		fread((void*) (&(mod1.size)) , sizeof(int), 1, redo_seg );
		mod1.data_ptr = calloc((mod1.size+1),sizeof(char));
		fread( (mod1.data_ptr) , sizeof(char), ((mod1.size)), redo_seg );
		redo1.redo_log.push_back(mod1);
	 }	
	fclose(redo_seg);

	char* file_name2 = NULL;
	
	for(i=0;i<redo1.num_updates;++i){
		mod1 = redo1.redo_log.front();
		
		file_name2 = dir_prefix(rvm,mod1.segname);
		temp = fopen(file_name2,"rt+");
		fseek(temp,mod1.offset,SEEK_SET);
		fwrite(mod1.data_ptr,sizeof(char),mod1.size,temp);		
		fclose(temp);
		free(mod1.segname);
		free(mod1.data_ptr);
		redo1.redo_log.pop_front();
		free(file_name2);
	}
	file_name2 = NULL;
	//Delete redo1.txt file or clear out its contents
	//Easy way, just open the file in write mode, WILL OVERWRITE!!! and create a blank file
	
	remove(file_name);
	free(file_name);
	file_name = NULL;
	
}

char* lookup_name(rvm_t rvm, const char* segname){
	std::map<char*,segment_t>::iterator seg_ptr;
	seg_ptr = rvm->seg_db.begin();
	for(;seg_ptr != rvm->seg_db.end();++seg_ptr){
		if(  strcmp(seg_ptr->second.segname,segname) == 0 ){
			return seg_ptr->second.segname;
		}
	}
	return NULL;
}

void *rvm_map(rvm_t rvm, const char *segname, int size_to_create){
	if(segname == NULL){
		return (void*) -1;		
	}	
	rvm_truncate_log(rvm);
	char* seg_name = NULL;
	seg_name = (char*)malloc((strlen(segname)+1)*sizeof(char));
	strcpy(seg_name,segname);
	seg_name[strlen(segname)] = '\0';
	FILE * fseg = NULL;
	char* file_name = NULL;
	char* seg_ptr = NULL;
	seg_ptr = lookup_name(rvm, segname);
	if(seg_ptr == NULL){				//not present in the map, install and opoen file (if exists), if not, then create an empty file 
		segment_t* new_seg = NULL;
		new_seg = (segment_t*)malloc(sizeof(segment_t));
		new (new_seg) segment_t();
		new_seg->segname = (char*)malloc((strlen(segname)+1)*sizeof(char));
		strcpy(new_seg->segname,segname);
		new_seg->segname[strlen(segname)] = '\0';
		new_seg->segbase = calloc(size_to_create,sizeof(char));
		file_name = dir_prefix(rvm,seg_name);
		fseg = fopen(file_name,"ab+");
		free(file_name);
		file_name = NULL;
		if(fseg == NULL){
			cout<<"ERROR. Segment file  was NOT found in dir. File unable to open!!!\n";
			fflush(stdout);
			return (void*) -1;
			
		}	
		// fseek(fseg,0,SEEK_SET);
		fread(new_seg->segbase,sizeof(char),size_to_create,fseg);
		fclose(fseg);
		new_seg->size = size_to_create;
		if(new_seg->undo_log != NULL){		//means currently under some TRANSACTION, since undo log is not NULL || ILLEGAL operation
			cout<<"\nRECHECK: UNDO_log ptr, is NOT NULL but segbase ptr is NULL!!!";		//Means a transaction was not cleared properly, as UNDO LOG was not cleared & freed
			fflush(stdout);
		}	
		else{		
			//cout<<"\nRECHECK: UNDO_log ptr, is NULL but segbase ptr is NOT!!!";

			// new_seg->undo_log = calloc(size_to_create,sizeof(char));
			// memcpy(new_seg->undo_log,new_seg->segbase,size_to_create);
		}
		rvm->seg_db.insert( std::pair<char*,segment_t>(new_seg->segname,*new_seg) );
		free(seg_name);
		seg_name = NULL;
		//free(new_seg);
		//new_seg = NULL;
		return new_seg->segbase;
	}
	else{		//present in the map
		
		if( rvm->seg_db[seg_ptr].segbase == NULL ){		//currently not mapped to memory (thus surely NO TRANSACTION)
			rvm->seg_db[seg_ptr].segbase = malloc(size_to_create*sizeof(char));
			file_name = dir_prefix(rvm,seg_ptr);
			fseg = fopen(file_name,"ab+");
			free(file_name);
			file_name = NULL;
			if(fseg == NULL){
				cout<<"ERROR. Segment file was NOT found in dir. File unable to open!!!\n";
				fflush(stdout);
				return (void*) -1;
			
			}		
			// fseek(fseg,0,SEEK_SET);
			fread(rvm->seg_db[seg_ptr].segbase,sizeof(char),size_to_create,fseg);
			fclose(fseg);
			rvm->seg_db[seg_ptr].size = size_to_create;
			
			
			if(rvm->seg_db[seg_ptr].tid != 0){
				cout<<"\nRECHECK: TID is NOT 0 but segbase ptr is NULL!!!";		//Means a transaction was not cleared properly, as UNDO LOG was not cleared & freed
			}
			if(rvm->seg_db[seg_ptr].trans != NULL){
				cout<<"\nRECHECK: TRANS is NOT NULL but segbase ptr is NULL!!!";		//Means a transaction was not cleared properly, as UNDO LOG was not cleared & freed
			}
			
			rvm->seg_db[seg_ptr].tid = 0;
			rvm->seg_db[seg_ptr].trans = NULL;
			
			if(rvm->seg_db[seg_ptr].undo_log != NULL){		//means currently under some TRANSACTION, since undo log is not NULL || ILLEGAL operation
				cout<<"\nRECHECK: UNDO_log ptr, is NOT NULL but segbase ptr is NULL!!!";		//Means a transaction was not cleared properly, as UNDO LOG was not cleared & freed
				free(rvm->seg_db[seg_ptr].undo_log);
				// rvm->seg_db[seg_ptr]->undo_log = calloc(size_to_create,sizeof(char));
				// memcpy(rvm->seg_db[seg_ptr]->undo_log,rvm->seg_db[seg_ptr]->segbase,size_to_create);
				
			}	
			else{		
				//cout<<"\nRECHECK: UNDO_log ptr, is NULL but segbase ptr is NOT!!!";
				// rvm->seg_db[seg_ptr]->undo_log = calloc(size_to_create,sizeof(char));
				// memcpy(rvm->seg_db[seg_ptr]->undo_log,rvm->seg_db[seg_ptr]->segbase,size_to_create);
			}	
			
			//CAN't DO here: since create UNDO logs in begin_trans() and not here in map
			// rvm->seg_db[seg_ptr].undo_log = calloc(size_to_create,sizeof(char));
			// memcpy(rvm->seg_db[seg_ptr].undo_log,rvm->seg_db[seg_ptr].segbase,size_to_create);
			free(seg_name);
			seg_name = NULL;
			return rvm->seg_db[seg_ptr].segbase;
		}
		else{		//presently mapped to memory, MAY/MAY NOT being operated on by some transactions
			
			// cout<<"Segment Mapping found in map, and SEGBASE was NOT NULL. Mapping already exists. NOthing to do!\n";
			/*
			//void* temp_ptr = NULL;
			//Create a new copy of the UNDO log on the mapping function call
			if(rvm->seg_db[seg_ptr].undo_log != NULL){		//means currently under some TRANSACTION, since undo log is not NULL || ILLEGAL operation
				//free(rvm->seg_db[seg_ptr].undo_log);	
			
			}	
			else{		
				//cout<<"\nRECHECK: UNDO_log ptr, is NULL but segbase ptr is NOT!!!";		//Was Possible, since memory could be mapped but not under any TRANSACTION
				//rvm->seg_db[seg_ptr].undo_log = calloc(size_to_create,sizeof(char));
			}
						
			//temp_ptr = calloc(size_to_create,sizeof(char));
			if(rvm->seg_db[seg_ptr].size < size_to_create){
				//memcpy(temp_ptr,rvm->seg_db[seg_ptr].segbase,rvm->seg_db[seg_ptr].size);
				//free(rvm->seg_db[seg_ptr].segbase);
			}
			else if(rvm->seg_db[seg_ptr].size > size_to_create){
				//memcpy(temp_ptr,rvm->seg_db[seg_ptr].segbase,size_to_create);
				//free(rvm->seg_db[seg_ptr].segbase);
			}
			else{		//equal to already what exists
				//no steps to be taken, simply return
				
			}
			
			//memcpy(rvm->seg_db[seg_ptr].undo_log,rvm->seg_db[seg_ptr].segbase,size_to_create);
			//ASK: re-create the UNDO log entry for every case, (RECREATE UNDO LOG/refresh it for an existing mapping??)
			//rvm->seg_db[seg_ptr].segbase = temp_ptr;
			//rvm->seg_db[seg_ptr].size = size_to_create;
			*/	
			free(seg_name);
			seg_name = NULL;
			return (void *) -1;		//ASK: if return -1, for all cases, if mapping exists, whether it may be same/small/bigger size
		}		
	}		
}

char* base2name(rvm_t rvm,void* segbase){
	std::map<char*,segment_t>::iterator seg_ptr;
	seg_ptr = rvm->seg_db.begin();
	for(;seg_ptr != rvm->seg_db.end();++seg_ptr){
		if(seg_ptr->second.segbase == segbase ){
			return seg_ptr->second.segname;
		}
	}
	return NULL;	
}

void rvm_unmap(rvm_t rvm, void *segbase){
	if( (segbase == NULL) || (rvm == NULL) ){
		return;
	}
	char* temp_segname = NULL;
	temp_segname = base2name(rvm,segbase);
	if(temp_segname == NULL){		//Segment is unmapped
		return;
	}
	else{
		free(rvm->seg_db[temp_segname].segbase);
		rvm->seg_db[temp_segname].segbase = NULL;
		if(rvm->seg_db[temp_segname].undo_log != NULL){
			free(rvm->seg_db[temp_segname].undo_log);
			rvm->seg_db[temp_segname].undo_log = NULL;
		}		
		rvm->seg_db[temp_segname].size = 0;
		if(rvm->seg_db[temp_segname].trans != NULL){
			//Clear transactions operating on this segment
			//FREE up and abort and remove existing transaction
			
			for(unsigned int i=0; i< rvm->seg_db[temp_segname].trans->num_seg;++i){
				//Can't FREE segbase and segnames, since the pointers in transaction are simply pointers to those in the seg_db segment database. Thus freeing them will lead to unmapping of those segments.
				if(rvm->seg_db[rvm->seg_db[temp_segname].trans->seg_names[i]].segbase != segbase){		//delete properties of all other segments apart from the currently being unmapped segment
					rvm->seg_db[rvm->seg_db[temp_segname].trans->seg_names[i]].tid = 0;
					memcpy(rvm->seg_db[rvm->seg_db[temp_segname].trans->seg_names[i]].segbase,rvm->seg_db[rvm->seg_db[temp_segname].trans->seg_names[i]].undo_log,rvm->seg_db[rvm->seg_db[temp_segname].trans->seg_names[i]].size);
					free(rvm->seg_db[rvm->seg_db[temp_segname].trans->seg_names[i]].undo_log);
					rvm->seg_db[rvm->seg_db[temp_segname].trans->seg_names[i]].undo_log = NULL;
					rvm->seg_db[rvm->seg_db[temp_segname].trans->seg_names[i]].trans = NULL;
				}	
			}			
			rvm->trans.erase(rvm->seg_db[temp_segname].trans->tid);		//POP from trans_db
			rvm->seg_db[temp_segname].trans->tid = 0;
			rvm->seg_db[temp_segname].trans->num_seg = 0;
			rvm->seg_db[temp_segname].tid = 0;
			free(rvm->seg_db[temp_segname].trans);
			rvm->seg_db[temp_segname].trans = NULL;
		}	
	}
	rvm->seg_db.erase(temp_segname);		//Pop from seg_db
	free(temp_segname);
	temp_segname = NULL;
}

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases){
	if(segbases == NULL){
		return (trans_t) -1;
	}
	trans_tt* new_trans = NULL;
	new_trans = (trans_tt*)calloc(1,sizeof(trans_tt));
	new (new_trans) trans_tt();
	int i=0;
	new_trans->tid = ++(tid_cnt);
	new_trans->num_seg = numsegs;
	new_trans->seg_names = (char**)malloc(numsegs*sizeof(char*));
	new_trans->seg_bases = (void**)calloc(numsegs,sizeof(void*));
	for(i=0;i<numsegs;++i){
		if(segbases[i] == NULL){
			cout<<"\n Pls CHECK the testcase. One of the passed base pointers to begin_trans() was NULL.\n";
			return (trans_t) -1;
		}
		new_trans->seg_names[i] = base2name(rvm,segbases[i]);
		if(rvm->seg_db[new_trans->seg_names[i]].undo_log == NULL){
			if(rvm->seg_db[new_trans->seg_names[i]].tid != 0){
				cout<<"\nRECHECK: UNDO_log ptr, is NULL but segment.TID is NOT 0!!!\n";		//Means a transaction was not cleared properly, as UNDO LOG was not cleared & freed
			}
			if(rvm->seg_db[new_trans->seg_names[i]].trans != NULL){
				cout<<"\nRECHECK: UNDO_log ptr, is NULL but TRANS ptr is NOT NULL!!!\n";		//Means a transaction was not cleared properly, as UNDO LOG was not cleared & freed
			}
			rvm->seg_db[new_trans->seg_names[i]].undo_log = calloc( rvm->seg_db[new_trans->seg_names[i]].size , sizeof(char) );
			memcpy(rvm->seg_db[new_trans->seg_names[i]].undo_log,segbases[i],rvm->seg_db[new_trans->seg_names[i]].size);
		}
		else{		//ALREADY BEING OPERATED BY SOME OTHER TRANSACTION
			cout<<"Segment is already under operation by SOME OTHER TRANSaction.\n";
			return (trans_t) -1;
			break;
		}
		rvm->seg_db[new_trans->seg_names[i]].tid = tid_cnt;
		rvm->seg_db[new_trans->seg_names[i]].trans = new_trans;
		new_trans->seg_bases[i] = segbases[i];
	}	
	rvm->trans.insert( std::pair<unsigned int,trans_tt*>(tid_cnt,new_trans) );
	return new_trans->tid;
}

//MAKE SURE THAT IN ABORT AS WELL AS IN COMMIT, I AM CLEARING TID AND TRANS OF SEGMENTS, IT WAS OPERATING ON, AND ALSO CLEAR THE UNDO_LOG POINTER AND 
void rvm_abort_trans(trans_t tid){		//Just like receiving an object
	if(rvm_gb == NULL){
		return;
	}
	trans_tt* temp = NULL;
	temp = rvm_gb->trans[tid];
	
	if(temp != NULL){
		//Clear transactions operating on this segment
		//FREE up and abort and remove existing transaction
		
		for(unsigned int i=0; i< temp->num_seg;++i){
			//Can't FREE segbase and segnames, since the pointers in transaction are simply pointers to those in the seg_db segment database. Thus freeing them will lead to unmapping of those segments.
			//if(rvm_gb->seg_db[temp->seg_names[i]].segbase != segbase){		//delete properties of all other segments apart from the currently being unmapped segment
				rvm_gb->seg_db[temp->seg_names[i]].tid = 0;
				memcpy(rvm_gb->seg_db[temp->seg_names[i]].segbase,rvm_gb->seg_db[temp->seg_names[i]].undo_log,rvm_gb->seg_db[temp->seg_names[i]].size);
				free(rvm_gb->seg_db[temp->seg_names[i]].undo_log);
				rvm_gb->seg_db[temp->seg_names[i]].undo_log = NULL;
				rvm_gb->seg_db[temp->seg_names[i]].trans = NULL;
			//}
			
		}		
		//TO-DO: See if have to free the mod_t elements (data_ptr) and pop_front() from the redo_log list.
		//Also see if want to change, the mod_t to mod_t* and then delete those pointers. (FREE them)
		// std::list<mod_t>::iterator ptr_redo;
		// for(ptr_redo = temp->redo_obj.redo_log.begin();  ptr_redo != temp->redo_obj.redo_log.end() ; ++ptr_redo){
			// free(temp->redo_obj.redo_log.front()->data_ptr);
			
		// }
		
		temp->redo_obj.num_updates = 0;
		rvm_gb->trans.erase(temp->tid);			//POP from trans_db
		temp->tid = 0;
		temp->num_seg = 0;		
		free(temp);
		temp = NULL;
	}
	else{
		cout<<"Invalid operation. The transaction doesn't exist anymore. Perhaps you unmapped a segment belonging to the transaction, which led to automatic deletion of the transaction.\n";
		return;
	}	
}

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size){
	if( (rvm_gb == NULL) || (segbase == NULL) || (tid == 0) ){
		return;
	}
	trans_tt* temp = NULL;
	temp = rvm_gb->trans[tid];
	
	if(temp != NULL){
		mod_t* new_mod = NULL;
		new_mod = (mod_t*)calloc(1,sizeof(mod_t));
		new (new_mod) mod_t();
		new_mod->segname = base2name(rvm_gb,segbase);
		if(new_mod->segname == NULL){
			cout<<"ERROR. Segment doesn't exist in the segment database!\n";
			fflush(stdout);
		}
		new_mod->segname_size = strlen(new_mod->segname)+1;
		new_mod->offset = offset;
		new_mod->size = size;
		++(temp->redo_obj.num_updates);
		temp->redo_obj.redo_log.push_back(*new_mod);
		
		//TO-DO: See if i can delete the pointer and free it right away!?
		free(new_mod);
		new_mod = NULL;
	}
	else{
		cout<<"Invalid operation. The transaction doesn't exist anymore. Perhaps you unmapped a segment belonging to the transaction, which led to automatic deletion of the transaction.\n";
		return;
	}
}

void rvm_commit_trans(trans_t tid){
	if( (rvm_gb == NULL) || (tid == 0) ){
		return;
	}
	trans_tt* temp = NULL;
	temp = rvm_gb->trans[tid];
	
	if(temp != NULL){
		FILE *redo_seg = NULL;
		//unsigned int i=0;
		char * file_name = NULL;
		file_name = dir_prefix(rvm_gb,rvm_gb->redo_file);
		redo_seg = fopen(file_name,"ab");

		while(temp->redo_obj.num_updates){	
			--(temp->redo_obj.num_updates);
			fwrite((void*) (&(temp->redo_obj.redo_log.front().segname_size)) , sizeof(unsigned int), 1, redo_seg );
			fwrite((void*) (temp->redo_obj.redo_log.front().segname) , sizeof(char), (temp->redo_obj.redo_log.front().segname_size), redo_seg );
			fwrite((void*) (&(temp->redo_obj.redo_log.front().offset)) , sizeof(int), 1, redo_seg );
			fwrite((void*) (&(temp->redo_obj.redo_log.front().size)) , sizeof(int), 1, redo_seg );
			// char* temp_data = (char*)(rvm_gb->seg_db[temp->redo_obj.redo_log.front().segname].segbase);
			// for(int i=0; i < (temp->redo_obj.redo_log.front().offset) ;++i,++temp_data){				
			// }
			// fwrite( (void*)temp_data , sizeof(char), (temp->redo_obj.redo_log.front().size), redo_seg );
			// fwrite( &(rvm_gb->seg_db[temp->redo_obj.redo_log.front().segname].segbase[temp->redo_obj.redo_log.front().offset]  ) , sizeof(char), (temp->redo_obj.redo_log.front().size), redo_seg );
			fwrite( ( (rvm_gb->seg_db[temp->redo_obj.redo_log.front().segname].segbase) + temp->redo_obj.redo_log.front().offset  ) , sizeof(char), temp->redo_obj.redo_log.front().size, redo_seg );
			temp->redo_obj.redo_log.pop_front();
		}	
		
		fclose(redo_seg);	
		free(file_name);
		file_name = NULL;	
		
		for(unsigned int i=0; i< temp->num_seg;++i){
			//Can't FREE segbase and segnames, since the pointers in transaction are simply pointers to those in the seg_db segment database. Thus freeing them will lead to unmapping of those segments.
			
			//Not applicable for this case, as want to clear out all transactional properties
			//if(rvm_gb->seg_db[temp->seg_names[i]].segbase != segbase){		//delete properties of all other segments apart from the currently being unmapped segment
				rvm_gb->seg_db[temp->seg_names[i]].tid = 0;
				
				free(rvm_gb->seg_db[temp->seg_names[i]].undo_log);
				rvm_gb->seg_db[temp->seg_names[i]].undo_log = NULL;
				
				rvm_gb->seg_db[temp->seg_names[i]].trans = NULL;
			//}
			
		}
		
		//TO-DO: See if have to free the mod_t elements (data_ptr) and pop_front() from the redo_log list.
		//Also see if want to change, the mod_t to mod_t* and then delete those pointers. (FREE them)
		
		temp->redo_obj.num_updates = 0;
		rvm_gb->trans.erase(temp->tid);		//Pop from the transaction database
		temp->tid = 0;
		temp->num_seg = 0;
		
		free(temp);
		temp = NULL;		
	}
	else{
		cout<<"Invalid operation. The transaction doesn't exist anymore. Perhaps you unmapped a segment belonging to the transaction, which led to automatic deletion of the transaction.\n";
		return;
	}
}

void rvm_destroy(rvm_t rvm, const char *segname){
	if(segname == NULL){
		return;
	}
	rvm_truncate_log(rvm);
	char* index_name = NULL;
	char* file_name = NULL;
	file_name = dir_prefix(rvm,(char*)segname);
	remove(file_name);
	free(file_name);
	file_name = NULL;
	index_name = lookup_name(rvm, segname);
	if(index_name != NULL){
		rvm_unmap(rvm,rvm->seg_db[index_name].segbase);
		index_name = NULL;	
	}	
}

void rvm_exit(rvm_t rvm){
	if(rvm == NULL){
		return;
	}
	rvm->~rvm_tt();
	free(rvm);
	rvm = NULL;
}