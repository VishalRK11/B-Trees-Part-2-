#define PAGE_SIZE 128
#define RECORDS 48
#define STUDENT_INDEX_PAGE_SIZE 16
#define MARKS_INDEX_PAGE_SIZE 8
#define STUDENT_PAGE_RECORDS 3
#define MARKS_PAGE_RECORDS 6

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Student{
	int id;
	char name[32];
}Student;

typedef struct Marks{
	int studentID;
	int marks[4];
}Marks;

typedef struct StudentDataPage{
	Student data[STUDENT_PAGE_RECORDS];
	char unused[18];
	unsigned char dataType;
	unsigned char tableID;
}StudentDataPage;

typedef struct MarksDataPage{
	Marks data[MARKS_PAGE_RECORDS];
	char unused[6];
	unsigned char dataType;
	unsigned char tableID;
}MarksDataPage;

typedef struct IndexPage{
	int pageID[16];
	int keys[15];
	char unused[2];
	unsigned char pageType;
	unsigned char tableID;
}IndexPage;

typedef struct TablePage{
	int indexPageID[10];
	char tableName[10][8];
	char unused[6];
	unsigned char pageType;
	unsigned char tableID;
}TablePage;