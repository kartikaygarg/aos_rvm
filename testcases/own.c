#include "../rvm.cpp"

void redo_cr(rvm_t rvm){
	FILE *redo_seg = NULL;
	int i=0;
	char * file_name = NULL;
	// file_name = dir_prefix(rvm,"redo1.txt");
	file_name = dir_prefix(rvm,rvm.redo_file);
	// file_name = (char*)malloc(sizeof(char)* ( strlen(rvm.dirname)+ (strlen("redo1.txt") +1 )  ) );
	// strcpy(file_name, rvm.dirname);
	// strcat(file_name,"redo1.txt");
	// file_name[strlen(rvm.dirname)+ strlen("redo1.txt")] = '\0';
	redo_seg = fopen(file_name,"ab");
	
	redo_t redo1;
	++(redo1.num_updates);
	mod_t mod1;
	mod1.segname_size = strlen("new1.txt") +1;
	cout<<"TEST: "<<mod1.segname_size<<endl;
	mod1.segname = (char*)malloc(mod1.segname_size);
	strcpy(mod1.segname,"new1.txt");
	mod1.segname[strlen("new1.txt")] = '\0';
	cout<<"TEST2: ";
	puts(mod1.segname);
	cout<<endl;
	mod1.offset = 3;
	mod1.size = 3;
	mod1.data_ptr = malloc((mod1.size+1) * sizeof(char));
	// mod1.data_ptr = malloc(mod1.size * sizeof(char));
	strcpy((char*)mod1.data_ptr,"xyz");
	strcat((char*)mod1.data_ptr,"\0");
	puts((char*)(mod1.data_ptr));
	cout<<endl;
	
	redo1.redo_log.push_back(mod1);
	std::list<mod_t>::iterator ptr;
	ptr = redo1.redo_log.begin();
	
	//for(;ptr != redo1.redo_log.end(); ++ptr){
		// fwrite((void*)(&(redo1.redo_log.front())),sizeof(mod_t),1,redo_seg);
	//	fwrite((void*)(&mod1),sizeof(mod_t),1,redo_seg);
	//}
	
	//i=0;
	// while(ptr != redo1.redo_log.end()){
	while(redo1.num_updates){	
		--(redo1.num_updates);
		// mod1 = redo1.redo_log[ptr];
		mod1 = redo1.redo_log.front();
		//mod1 = redo1.redo_log[i];
		//fwrite((void*)(&(redo1.redo_log.front())),sizeof(mod_t),1,redo_seg);	
		fwrite((void*) (&(mod1.segname_size)) , sizeof(unsigned int), 1, redo_seg );
		fwrite((void*) (mod1.segname) , sizeof(char), (mod1.segname_size), redo_seg );
		//fread((void*) (mod1.segname) , sizeof(char), (mod1.segname_size+1), redo_seg );
		fwrite((void*) (&(mod1.offset)) , sizeof(int), 1, redo_seg );
		fwrite((void*) (&(mod1.size)) , sizeof(int), 1, redo_seg );
		// fwrite((void*) (mod1.data_ptr) , sizeof(char), (mod1.size), redo_seg );
		fwrite((void*) (mod1.data_ptr) , sizeof(char), ((mod1.size)+1), redo_seg );
		//redo1.redo_log.pop_front();
		free(mod1.segname);
		free(mod1.data_ptr);
		//++i;
		++ptr;
		redo1.redo_log.pop_front();
	}	
	
	fclose(redo_seg);
	
	free(file_name);
	file_name = NULL;
	
}

void redo_cr2(rvm_t rvm){
	FILE *redo_seg = NULL;
	int i=0;
	char * file_name = NULL;
	// file_name = dir_prefix(rvm,"redo1.txt");
	file_name = dir_prefix(rvm,rvm.redo_file);
	// file_name = (char*)malloc(sizeof(char)* ( strlen(rvm.dirname)+ (strlen("redo1.txt") +1 )  ) );
	// strcpy(file_name, rvm.dirname);
	// strcat(file_name,"redo1.txt");
	// file_name[strlen(rvm.dirname)+ strlen("redo1.txt")] = '\0';
	redo_seg = fopen(file_name,"ab");
	
	redo_t redo1;
	++(redo1.num_updates);
	mod_t mod1;
	mod1.segname_size = strlen("new1.txt") +1;
	cout<<"TEST: "<<mod1.segname_size<<endl;
	mod1.segname = (char*)malloc(mod1.segname_size);
	strcpy(mod1.segname,"new1.txt");
	mod1.segname[strlen("new1.txt")] = '\0';
	cout<<"TEST2: ";
	puts(mod1.segname);
	cout<<endl;
	mod1.offset = 3;
	mod1.size = 3;
	mod1.data_ptr = malloc((mod1.size+1) * sizeof(char));
	// mod1.data_ptr = malloc(mod1.size * sizeof(char));
	strcpy((char*)mod1.data_ptr,"xyz");
	strcat((char*)mod1.data_ptr,"\0");
	puts((char*)(mod1.data_ptr));
	cout<<endl;
	
	redo1.redo_log.push_back(mod1);
	std::list<mod_t>::iterator ptr;
	ptr = redo1.redo_log.begin();
	
	//for(;ptr != redo1.redo_log.end(); ++ptr){
		// fwrite((void*)(&(redo1.redo_log.front())),sizeof(mod_t),1,redo_seg);
	//	fwrite((void*)(&mod1),sizeof(mod_t),1,redo_seg);
	//}
	
	//i=0;
	// while(ptr != redo1.redo_log.end()){
	while(redo1.num_updates){	
		--(redo1.num_updates);
		// mod1 = redo1.redo_log[ptr];
		mod1 = redo1.redo_log.front();
		//mod1 = redo1.redo_log[i];
		//fwrite((void*)(&(redo1.redo_log.front())),sizeof(mod_t),1,redo_seg);	
		fwrite((void*) (&(mod1.segname_size)) , sizeof(unsigned int), 1, redo_seg );
		fwrite((void*) (mod1.segname) , sizeof(char), (mod1.segname_size), redo_seg );
		//fread((void*) (mod1.segname) , sizeof(char), (mod1.segname_size+1), redo_seg );
		fwrite((void*) (&(mod1.offset)) , sizeof(int), 1, redo_seg );
		fwrite((void*) (&(mod1.size)) , sizeof(int), 1, redo_seg );
		// fwrite((void*) (mod1.data_ptr) , sizeof(char), (mod1.size), redo_seg );
		fwrite((void*) (mod1.data_ptr) , sizeof(char), ((mod1.size)+1), redo_seg );
		//redo1.redo_log.pop_front();
		free(mod1.segname);
		free(mod1.data_ptr);
		//++i;
		++ptr;
		redo1.redo_log.pop_front();
	}	
	
	fclose(redo_seg);
	
	free(file_name);
	file_name = NULL;
	
}

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
	
	redo_cr(rvm);
	rvm_truncate_log(rvm);
	void* ptr = NULL;
	// ptr = map
	// umap()
	
	
	return 0;
}