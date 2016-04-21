#include "rvm.h"
#include <sys/stat.h>
#include <iostream>
#include <stdlib.h>
//#include <unistd.h>
#include <fstream>
//#include <conio.h>
#include <string.h>

unsigned long int tid_cnt = 0;

using namespace std;

rvm_t rvm_dir;

//Creates A DIRECTORY with specified name where persistent memory space (extrenal data segment,log files) shall be maintained
rvm_t rvm_init(const char *directory){
	struct stat st;
	rvm_t rvm_dir;
	if(mkdir(directory, 0700) != 0){	//dir not created
		stat(directory,&st);
		//if((chmod(directory,(st.st_mode|700))== EPERM) || (chmod(directory,(st.st_mode|700))== EROFS)){
		if(chmod(directory,(st.st_mode|0700))== -1){	
			printf("Cannot create directory here. Check user permissions or Read-Only flag on memory space.");
			rvm_dir.dirname = NULL;
			return rvm_dir;
		}
	}
	rvm_dir.dirname = (char*) malloc (strlen(directory)+2);
	strcpy(rvm_dir.dirname,directory);
	strcat(rvm_dir.dirname,"/");
	rvm_dir.dirname[strlen(directory)+1] = '\0';
	return rvm_dir;
}

char* dir_prefix(rvm_t rvm, char* segname){
	char* prefix = NULL;
	prefix = (char*)malloc(sizeof(char) * ( strlen(rvm.dirname)+ (strlen(segname) +1 )  ) );
	strcpy(prefix, rvm.dirname);
	strcat(prefix,segname);
	prefix[strlen(rvm.dirname) + strlen(segname)] = '\0';
	return prefix;
}	

// UNDER_DEVELOPMENT
void rvm_truncate_log(rvm_t rvm){
	
	FILE *redo_seg = NULL;
	FILE *temp = NULL;
	char * file_name = NULL;
	int i = 0;
	file_name = dir_prefix(rvm,"redo1.txt");
	// file_name = (char*)malloc(sizeof(char)* ( strlen(rvm.dirname)+ (strlen("redo1.txt") +1 )  ) );
	// strcpy(file_name, rvm.dirname);
	// strcat(file_name,"redo1.txt");
	// file_name[strlen(rvm.dirname) + strlen("redo1.txt")] = '\0';
	redo_seg = fopen(file_name,"rb");
	
	redo_t redo1;
	mod_t mod1;
	// std::list<mod_t>::iterator ptr;
	// ptr = redo1.redo_log.begin();
	
	// while(!feof(redo_seg)){
	while(fgetc(redo_seg)!= EOF){	
		fseek(redo_seg,-1,SEEK_CUR);
		++(redo1.num_updates);
		//fwrite((void*)(&(redo1.redo_log.front())),sizeof(mod_t),1,redo_seg);	
		fread((void*) (&(mod1.segname_size)) , sizeof(unsigned int), 1, redo_seg );
		cout<<"1 "<<mod1.segname_size<<endl;
		mod1.segname = (char*) malloc(sizeof(char) * mod1.segname_size);
		fread((void*) (mod1.segname) , sizeof(char), (mod1.segname_size), redo_seg );
		//fread((void*) (mod1.segname) , sizeof(char), (mod1.segname_size+1), redo_seg );
		mod1.segname[mod1.segname_size -1] = '\0';
		cout<<"2 ";
		puts((char*)mod1.segname);
		//cout<<endl;
		fread((void*) (&(mod1.offset)) , sizeof(int), 1, redo_seg );
		cout<<"3 "<<mod1.offset<<endl;
		fread((void*) (&(mod1.size)) , sizeof(int), 1, redo_seg );
		cout<<"4 "<<mod1.size<<endl;
		mod1.data_ptr = malloc(sizeof(char) * (mod1.size+1));
		// mod1.data_ptr = malloc(sizeof(char) * mod1.size);
		fread((void*) (mod1.data_ptr) , sizeof(char), ((mod1.size)+1), redo_seg );
		// fread( (mod1.data_ptr) , sizeof(char), ((mod1.size)), redo_seg );
		cout<<"5 ";
		puts((char*)mod1.data_ptr);
		// cout<<endl;
		redo1.redo_log.push_back(mod1);
		// if(feof(redo_seg)){
			// cout<<"reached end "<<endl;
			// break;
		// }
		//free(mod1.segname);
		//free(mod1.data_ptr);
	 }	
	fclose(redo_seg);
	free(file_name);
	file_name = NULL;
	
	for(i=0;i<redo1.num_updates;++i){
		mod1 = redo1.redo_log.front();
		
		file_name = dir_prefix(rvm,mod1.segname);
		// file_name = (char*)malloc(sizeof(char)* ( strlen(rvm.dirname)+ (mod1.segname_size +1 )  ) );
		// strcpy(file_name, rvm.dirname);
		// strcat(file_name,mod1.segname);
		// file_name[strlen(rvm.dirname) + mod1.segname_size] = '\0';
	
		temp = fopen(file_name,"rt+");

		fseek(temp,mod1.offset,SEEK_SET);
		fwrite(mod1.data_ptr,sizeof(char),mod1.size,temp);		
		fclose(temp);
		free(mod1.segname);
		free(mod1.data_ptr);
		redo1.redo_log.pop_front();
		free(file_name);
	}
	
	
	//free(file_name);
	
	
}

void *rvm_map(rvm_t rvm, const char *segname, int size_to_create){
	if(segname == NULL){
		return (void*) -1;		
	}
	char* seg_name = NULL;
	seg_name = (char*)malloc((strlen(segname)+1)*sizeof(char));
	strcpy(seg_name,segname);
	seg_name[strlen(segname)] = '\0';
	std::map<char*,segment_t>::iterator seg_ptr;
	FILE * fseg = NULL;
	seg_ptr = rvm.seg_db.find(seg_name);
	if( seg_ptr == rvm.seg_db.end()){	//not present in the map, install and opoen file (if exists), if not, then create an empty file 
		segment_t new_seg;
		new_seg.segname = (char*)malloc((strlen(segname)+1)*sizeof(char));
		strcpy(new_seg.segname,segname);
		// strcat(new_seg.segname,"\0");
		new_seg.segname[strlen(segname)] = '\0';
		new_seg.segbase = malloc(size_to_create);
		
		fseg = fopen(dir_prefix(rvm,seg_name),"rb");
		fread(new_seg.segbase,sizeof(char),size_to_create,fseg);
		fclose(fseg);
		new_seg.size = size_to_create;
		if(new_seg.undo_log != NULL){		//means currently under some TRANSACTION, since undo log is not NULL || ILLEGAL operation
			cout<<"\nRECHECK: UNDO_log ptr, is NOT NULL but segbase ptr is NULL!!!";		//Means a transaction was not cleared properly, as UNDO LOG was not cleared & freed
			
		}	
		else{		
			//cout<<"\nRECHECK: UNDO_log ptr, is NULL but segbase ptr is NOT!!!";

			// new_seg.undo_log = malloc(size_to_create);
			// memcpy(new_seg.undo_log,new_seg.segbase,size_to_create);
		}
		rvm.seg_db.insert( std::pair<char*,segment_t>(seg_name,new_seg) );
	}
	else{		//present in the map 
		if( rvm.seg_db[seg_name].segbase == NULL ){		//currently not mapped to memory (thus surely NO TRANSACTION)
			rvm.seg_db[seg_name].segbase = malloc(size_to_create);
			
			fseg = fopen(dir_prefix(rvm,seg_name),"rb");
			fread(rvm.seg_db[seg_name].segbase,sizeof(char),size_to_create,fseg);
			fclose(fseg);
			rvm.seg_db[seg_name].size = size_to_create;
			
			
			if(rvm.seg_db[seg_name].tid != 0){
				cout<<"\nRECHECK: TID is NOT 0 but segbase ptr is NULL!!!";		//Means a transaction was not cleared properly, as UNDO LOG was not cleared & freed
			}
			if(rvm.seg_db[seg_name].trans != NULL){
				cout<<"\nRECHECK: TRANS is NOT NULL but segbase ptr is NULL!!!";		//Means a transaction was not cleared properly, as UNDO LOG was not cleared & freed
			}
			
			rvm.seg_db[seg_name].tid = 0;
			rvm.seg_db[seg_name].trans = NULL;
			
			if(rvm.seg_db[seg_name].undo_log != NULL){		//means currently under some TRANSACTION, since undo log is not NULL || ILLEGAL operation
				cout<<"\nRECHECK: UNDO_log ptr, is NOT NULL but segbase ptr is NULL!!!";		//Means a transaction was not cleared properly, as UNDO LOG was not cleared & freed
				free(rvm.seg_db[seg_name].undo_log);
				
				// rvm.seg_db[seg_name]->undo_log = malloc(size_to_create);
				// memcpy(rvm.seg_db[seg_name]->undo_log,rvm.seg_db[seg_name]->segbase,size_to_create);
				
			}	
			else{		
				//cout<<"\nRECHECK: UNDO_log ptr, is NULL but segbase ptr is NOT!!!";
				
				// rvm.seg_db[seg_name]->undo_log = malloc(size_to_create);
				// memcpy(rvm.seg_db[seg_name]->undo_log,rvm.seg_db[seg_name]->segbase,size_to_create);
			}	
			
			//CAN't DO here: since create UNDO logs in begin_trans() and not here in map
			// rvm.seg_db[seg_name].undo_log = malloc(size_to_create);
			// memcpy(rvm.seg_db[seg_name].undo_log,rvm.seg_db[seg_name].segbase,size_to_create);
		}
		else{		//presently mapped to memory, MAY/MAY NOT being operated on by some transactions
			
			
			
			/*
			//void* temp_ptr = NULL;
			
			//Create a new copy of the UNDO log on the mapping function call
			
			if(rvm.seg_db[seg_name].undo_log != NULL){		//means currently under some TRANSACTION, since undo log is not NULL || ILLEGAL operation
				//free(rvm.seg_db[seg_name].undo_log);	
			
			}	
			else{		
				//cout<<"\nRECHECK: UNDO_log ptr, is NULL but segbase ptr is NOT!!!";		//Was Possible, since memory could be mapped but not under any TRANSACTION
				//rvm.seg_db[seg_name].undo_log = malloc(size_to_create);
			}
				
			
			//temp_ptr = malloc(size_to_create);
			if(rvm.seg_db[seg_name].size < size_to_create){
				//memcpy(temp_ptr,rvm.seg_db[seg_name].segbase,rvm.seg_db[seg_name].size);
				//free(rvm.seg_db[seg_name].segbase);
			}
			else if(rvm.seg_db[seg_name].size > size_to_create){
				//memcpy(temp_ptr,rvm.seg_db[seg_name].segbase,size_to_create);
				//free(rvm.seg_db[seg_name].segbase);
			}
			else{		//equal to already what exists
				//no steps to be taken, simply return
				
			}
			
			//memcpy(rvm.seg_db[seg_name].undo_log,rvm.seg_db[seg_name].segbase,size_to_create);
			//ASK: re-create the UNDO log entry for every case, (RECREATE UNDO LOG/refresh it for an existing mapping??)
			
			//rvm.seg_db[seg_name].segbase = temp_ptr;
			//rvm.seg_db[seg_name].size = size_to_create;
			*/
			
			return (void *) -1;		//ASK: if return -1, for all cases, if mapping exists, whether it may be same/small/bigger size
		}		
	}		
}

char* base2name(rvm_t rvm,void* segbase){
	std::map<char*,segment_t>::iterator seg_ptr;
	seg_ptr = rvm.seg_db.begin();
	for(;seg_ptr != rvm.seg_db.end();++seg_ptr){
		if(seg_ptr->second.segbase == segbase ){
			return seg_ptr->second.segname;
		}
	}
	return NULL;	
}

void rvm_unmap(rvm_t rvm, void *segbase){
	char* temp_segname = NULL;
	// temp_segname = (char*)malloc(strlen(base2name(rvm,segbase))+1);
	temp_segname = base2name(rvm,segbase);
	// strcpy(temp_segname,base2name(rvm,segbase));
	// temp_segname[strlen(base2name(rvm,segbase))] = '\0';
	if(temp_segname == NULL){		//Segment is unmapped
		return;
	}
	else{
		free(rvm.seg_db[temp_segname].segbase);
		rvm.seg_db[temp_segname].segbase = NULL;
		free(rvm.seg_db[temp_segname].undo_log);
		rvm.seg_db[temp_segname].undo_log = NULL;
		rvm.seg_db[temp_segname].size = 0;
		rvm.seg_db[temp_segname].tid = 0;
		rvm.seg_db[temp_segname].trans = NULL;
	}
	
	//TO-DO: Clear transactions operating on this segment
	
	// free(temp_segname);
	temp_segname = NULL;
}

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases){
	trans_t new_trans;
	int i=0;
	new_trans.tid = ++(tid_cnt);
	new_trans.num_seg = numsegs;
	new_trans.seg_names = (char**)malloc(numsegs*sizeof(char*));
	for(i=0;i<numsegs;++i){
		new_trans.seg_names[i] = base2name(rvm,segbases[i]);
	}
	
	
	rvm.trans.insert( std::pair<unsigned int,trans_t>(tid_cnt,new_trans) );
}

/*
int main(){
	char* dirname;
	dirname = (char*)malloc(8*sizeof(char));
	strcpy(dirname,"rvm_dir");
	rvm_t rvm;
	
	rvm = rvm_init(dirname);
	if(rvm.dirname == NULL){
		cout<<"rvm_init FAILED!!!\n";
	}
	
	//getch();
	
	
	return 0;
}
*/