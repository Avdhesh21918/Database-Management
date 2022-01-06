#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "hash.h"

#define SIZE 10000

int get_fileSize(FILE *file){
	fseek( file, 0, SEEK_END);
	int size = ftell(file) / sizeof(long);          /* getting file size*/
	return size;
}

int get_index(char *key, FILE *txt_file, FILE *idx_file, FILE *hash_file){
	long hash_table[SIZE], val;
	int size = 0;
	while(fread(&val, sizeof(long), 1, hash_file)){                      /* getting file index*/
		hash_table[size++] = val;
	}

	char buffer[BUFFER];
	for(int i=0; i<size; i++){
		if(hash_table[i]<0)
			continue;

		fseek(idx_file, hash_table[i]*sizeof(long), SEEK_SET);
		fread(&val, sizeof(long), 1, idx_file);

		fseek(txt_file, val, SEEK_SET);
		fgets(buffer, BUFFER, txt_file);

		buffer[ strlen(buffer)-1 ] = '\0';

		if(strcmp(buffer, key) == 0)
			return hash_table[i];
	}
	return -1;
}

char* get_code_index(FILE *code_file, int idx, int code_size, int file_size){
	char* codes = malloc( code_size );

	char* array = malloc( code_size*file_size );
	fseek(code_file, 0, SEEK_SET);
	fread(array, 1, code_size*file_size, code_file);

	for(int i=0; i<code_size; i++){
		int index = i*file_size + idx;
		codes[i] = array[index];
	}
	return codes;
}

int code_to_index(FILE *code_file, int* indexes, int code_idx, int code_size, int file_size){
	char* array = malloc( code_size* file_size );
	fseek(code_file, 0, SEEK_SET);
	fread(array, 1, code_size*file_size, code_file);

	int size = 0;
	for(int i=0; i<file_size; i++){
		int index = code_idx*file_size + i;                     
		if(array[index])
			indexes[size++] = i;
	}
	return size;
}

int get_indexes_from_codes(char* codes, int* indexes, int max_size){
	int size = 0;
	for(int i=0; i<max_size; i++){
		if(codes[i])
			indexes[size++] = i;                                  /* course code*/
	}
	return size;
}

int get_code_intersection(int* common_codes, char* codes1, char* codes2, int size){
	int index = 0;

	for(int i=0; i<size; i++){
		char c = codes1[i]&&codes2[i];
		if(c)
			common_codes[index++] = i;
	}
	return index;
}

char* get_string(FILE *txt_file, FILE *idx_file, int index){
	char* buffer = malloc( BUFFER );
	long val;

	fseek(idx_file, sizeof(long)*index, SEEK_SET);                /* getting the string*/
	fread(&val, sizeof(long), 1, idx_file);

	fseek(txt_file, val, SEEK_SET);
	fgets(buffer, BUFFER, txt_file);
	buffer[ strlen(buffer)-1 ] = '\0';

	return buffer;
}

int main(int argc, char** argv){

	if(argc != 3){
		fprintf( stderr, "Usage: %s basename value\n", argv[0]);
		exit(-1);
	}

	char* building = argv[1];
	char* room = argv[2];

	FILE *building_txt_file = fopen("building.txt", "r");
	FILE *room_txt_file = fopen("room.txt", "r");                                 /* opening the building .txt,room.txt,subject.txt,courseno,days,from,to.txt file*/
	FILE *subject_txt_file = fopen("subject.txt", "r");
	FILE *courseno_txt_file = fopen("courseno.txt", "r");
	FILE *days_txt_file = fopen("days.txt", "r");
	FILE *from_txt_file = fopen("from.txt", "r");
	FILE *to_txt_file = fopen("to.txt", "r");

	FILE *code_idx_file = fopen("code.idx", "r");
	FILE *building_idx_file = fopen("building.idx", "r");                          /* opening the building .idx,room.idx,subject.idx,courseno,days,from,to.idx file*/
	FILE *room_idx_file = fopen("room.idx", "r");
	FILE *subject_idx_file = fopen("subject.idx", "r");
	FILE *courseno_idx_file = fopen("courseno.idx", "r");
	FILE *days_idx_file = fopen("days.idx", "r");
	FILE *from_idx_file = fopen("from.idx", "r");
	FILE *to_idx_file = fopen("to.idx", "r");

	FILE *building_hash_file = fopen("building.hash", "rb");
	FILE *room_hash_file = fopen("room.hash", "rb");


	FILE *code_building_file = fopen("code_building.rel", "rb");
	FILE *code_room_file = fopen("code_room.rel", "rb");
	FILE *code_subject_file = fopen("code_subject.rel", "rb");
	FILE *code_courseno_file = fopen("code_courseno.rel", "rb");                      /* opening the building .rel,room.txt,subject.rel,courseno,days,from,to.rel file*/
	FILE *code_days_file = fopen("code_days.rel", "rb");
	FILE *code_from_file = fopen("code_from.rel", "rb");
	FILE *code_to_file = fopen("code_to.rel", "rb");

	//Getting index.
	int building_idx = get_index(building, building_txt_file, building_idx_file, building_hash_file);
	int room_idx = get_index(room, room_txt_file, room_idx_file, room_hash_file);

	if(building_idx==-1 || room_idx==-1)
	{
		fprintf( stderr, "Invalid values!! - %s, %s\n", building, room);
		exit(-1);
	}

	// printf("%d %d\n", building_idx, room_idx);
	
	char* codes_building = get_code_index(code_building_file, building_idx,
			get_fileSize(code_idx_file), get_fileSize(building_idx_file));

	char* codes_room = get_code_index(code_room_file, room_idx, 
			get_fileSize(code_idx_file), get_fileSize(room_idx_file));

	int common_codes[SIZE],
	common_codes_size = get_code_intersection(common_codes, codes_building, codes_room,
		get_fileSize(code_idx_file));

	for(int i=0; i<common_codes_size; i++){
		int subjects[SIZE], courseno[SIZE], days[SIZE], from[SIZE], to[SIZE];
		int max_size = 0;

		int subject_size = code_to_index(
				code_subject_file, subjects, common_codes[i],
				get_fileSize(code_idx_file),
				get_fileSize(subject_idx_file)
			);
		max_size = (max_size < subject_size) ? subject_size : max_size;      /* printing the subjectsize*/

		//---------------------------------------------------

		int courseno_size = code_to_index(
				code_courseno_file, courseno, common_codes[i],
				get_fileSize(code_idx_file),
				get_fileSize(courseno_idx_file)
			);
		max_size = (max_size < courseno_size) ? courseno_size : max_size;       /* course number*/

		//----------------------------------------------------

		int days_size = code_to_index(
				code_days_file, days, common_codes[i],
				get_fileSize(code_idx_file),
				get_fileSize(days_idx_file)
			);
		max_size = (max_size < days_size) ? days_size : max_size;             // printing the days

		//-----------------------------------------------------

		int from_size = code_to_index(
				code_subject_file, from, common_codes[i],
				get_fileSize(code_idx_file),
				get_fileSize(from_idx_file)
			);                                     
		max_size = (max_size < from_size) ? from_size : max_size;        /* ternatory operator */

		//------------------------------------------------------

		int to_size = code_to_index(
				code_subject_file, to, common_codes[i],
				get_fileSize(code_idx_file),
				get_fileSize(to_idx_file)
			);
		max_size = (max_size < to_size) ? to_size : max_size;

		//-------------------------------------------------------
		for(int j=0; j<max_size; j++){
			char* subject_txt = (j<subject_size) 
				? get_string(subject_txt_file, subject_idx_file, subjects[j]) 
				: "NA";
			char* courseno_txt = (j<courseno_size) 
				? get_string(courseno_txt_file, courseno_idx_file, courseno[j]) 
				: "NA";
			char* day_txt = (j<days_size) 
				? get_string(days_txt_file, days_idx_file, days[j]) 
				: "NA";
			char* from_txt = (j<from_size) 
				? get_string(from_txt_file, from_idx_file, from[j]) 
				: "NA";
			char* to_txt = (j<to_size) 
				? get_string(to_txt_file, to_idx_file, to[j]) 
				: "NA";

			printf("%s*%s %s %s - %s\n", subject_txt, courseno_txt, day_txt, from_txt, to_txt);
		}
	}

	//Closing all the file pointers.
	fclose(building_txt_file);
	fclose(building_idx_file);
	fclose(building_hash_file);                         /* closing all the files*/

	fclose(room_txt_file);
	fclose(room_idx_file);
	fclose(room_hash_file);
	
	fclose(code_building_file);
	fclose(code_room_file);
	fclose(code_idx_file);
	fclose(code_subject_file);
	fclose(code_courseno_file);
	fclose(code_days_file);
	fclose(code_from_file);
	fclose(code_to_file);

	fclose(subject_idx_file);
	fclose(courseno_idx_file);
	fclose(days_idx_file);
	fclose(from_idx_file);
	fclose(to_idx_file);

	fclose(subject_txt_file);
	fclose(courseno_txt_file);
	fclose(days_txt_file);
	fclose(from_txt_file);
	fclose(to_txt_file);

	return 0;
}
