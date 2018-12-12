#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Structures.h"

void loadStudentDetails(char *filename, char *binaryFilename){
	FILE *fp = fopen(filename, "r");
	FILE *bfp = fopen(binaryFilename, "wb");
	fclose(bfp);

	bfp = fopen(binaryFilename, "ab");

	char *line = (char *)malloc(sizeof(char)* PAGE_SIZE);

	int i = 0, p = 0;

	StudentDataPage dp;

	fgets(line, PAGE_SIZE, fp);

	while (fgets(line, PAGE_SIZE, fp) != '\0'){
		dp.dataType = '1';
		dp.tableID = '1';
		dp.data[i].id = atoi(strtok(line, ","));
		strcpy(dp.data[i].name, strtok(NULL, ","));
		i += 1;
		p += 1;
		if (i == STUDENT_PAGE_RECORDS){
			fwrite(&dp, sizeof(StudentDataPage), 1, bfp);
			memset(&dp, 0, sizeof(StudentDataPage));
			i = 0;
		}
		if (p == RECORDS){
			if (p % STUDENT_PAGE_RECORDS != 0)
				fwrite(&dp, sizeof(StudentDataPage), 1, bfp);
			break;
		}
	}

	fclose(bfp);
	fclose(fp);
}

void loadMarksDetails(char *filename, char *binaryFilename){
	FILE *fp = fopen(filename, "r");
	
	FILE *bfp = fopen(binaryFilename, "ab");

	char *line = (char *)malloc(sizeof(char)* PAGE_SIZE);

	int i = 0, p = 0;

	MarksDataPage dp;

	fgets(line, PAGE_SIZE, fp);

	while (fgets(line, PAGE_SIZE, fp) != '\0'){
		dp.dataType = '1';
		dp.tableID = '2';
		dp.data[i].studentID = atoi(strtok(line, ","));
		for (int j = 0; j < 4; ++j)
			dp.data[i].marks[j] = atoi(strtok(NULL, ","));
		i += 1;
		p += 1;
		if (i == MARKS_PAGE_RECORDS){
			fwrite(&dp, sizeof(StudentDataPage), 1, bfp);
			memset(&dp, 0, sizeof(StudentDataPage));
			i = 0;
		}
		if (p == RECORDS){
			if (p % MARKS_PAGE_RECORDS != 0)
				fwrite(&dp, sizeof(StudentDataPage), 1, bfp);
			break;
		}
	}

	fclose(bfp);
	fclose(fp);
}

void loadStudentIndexPage(char *filename){
	IndexPage ip;
	FILE *bfp = fopen(filename, "rb");

	for (int i = 0; i < STUDENT_INDEX_PAGE_SIZE; ++i){
		ip.pageID[i] = i * PAGE_SIZE;
		fseek(bfp, ip.pageID[i], SEEK_SET);
		if (i > 0)
			fread(&ip.keys[i-1], sizeof(int), 1, bfp);
	}

	fclose(bfp);

	bfp = fopen(filename, "ab");

	fwrite(&ip, sizeof(IndexPage), 1, bfp);

	fclose(bfp);
}

void loadMarksIndexPage(char *filename){
	IndexPage ip;
	FILE *bfp = fopen(filename, "rb");

	for (int i = 0; i < MARKS_INDEX_PAGE_SIZE; ++i){
		ip.pageID[i] = (i + STUDENT_INDEX_PAGE_SIZE + 1) * PAGE_SIZE ;
		fseek(bfp, ip.pageID[i], SEEK_SET);
		fread(&ip.keys[i], sizeof(int), 1, bfp);
	}

	for (int i = MARKS_INDEX_PAGE_SIZE; i < STUDENT_INDEX_PAGE_SIZE-1; ++i){
		ip.pageID[i] = -1;
		ip.keys[i] = -1;
	}
	ip.pageID[STUDENT_INDEX_PAGE_SIZE - 1] = -1;

	fclose(bfp);

	bfp = fopen(filename, "ab");

	fwrite(&ip, sizeof(IndexPage), 1, bfp);

	fclose(bfp);
}

void processStudentData(){
	loadStudentDetails("StudentsData.csv", "Data.bin");
	loadStudentIndexPage("Data.bin");
}

void processMarksData(){
	loadMarksDetails("MarksData.csv", "Data.bin");
	loadMarksIndexPage("Data.bin");
}

void loadTablePage(char *filename){
	TablePage tp;
	tp.pageType = '3';
	tp.tableID = '0';
	strcpy(tp.tableName[0], "Students");
	strcpy(tp.tableName[1], "Marks");
	tp.indexPageID[0] = STUDENT_INDEX_PAGE_SIZE * PAGE_SIZE;
	tp.indexPageID[1] = tp.indexPageID[0] + (MARKS_INDEX_PAGE_SIZE + 1) * PAGE_SIZE;

	FILE *bfp = fopen(filename, "ab");

	fwrite(&tp, sizeof(TablePage), 1, bfp);

	fclose(bfp);

}

void searchQuery(char *filename, int id){
	FILE *bfp = fopen(filename, "rb+");
	fseek(bfp, -PAGE_SIZE, SEEK_END);

	int location, locFlag = 1, queryFlag = 1;
	fread(&location, sizeof(int), 1, bfp);
	fseek(bfp, location, SEEK_SET);

	IndexPage ip;
	StudentDataPage dp;
	fread(&ip, sizeof(IndexPage), 1, bfp);

	for (int i = 0; i < STUDENT_INDEX_PAGE_SIZE-1; ++i){
		if (id < ip.keys[i]){
			fseek(bfp, ip.pageID[i], SEEK_SET);
			fread(&dp, sizeof(StudentDataPage), 1, bfp);
			locFlag = 0;
			break;
		}
	}

	if (locFlag){
		fseek(bfp, ip.pageID[STUDENT_INDEX_PAGE_SIZE - 1], SEEK_SET);
		fread(&dp, sizeof(StudentDataPage), 1, bfp);
	}

	for (int i = 0; i < STUDENT_PAGE_RECORDS; ++i){
		if (dp.data[i].id == id){
			printf("\nStudent found with name %s.\n\n", dp.data[i].name);
			queryFlag = 0;
			break;
		}
	}
	fclose(bfp);

	if (queryFlag)
		printf("\nStudent not found.\n\n");

}

void endRangeQuery(char *filename, int endID){
	FILE *bfp = fopen(filename, "rb+");
	fseek(bfp, -PAGE_SIZE, SEEK_END);

	int location, locFlag = -1, queryFlag = 1;
	fread(&location, sizeof(int), 1, bfp);
	fseek(bfp, location, SEEK_SET);

	IndexPage ip;
	StudentDataPage dp;
	fread(&ip, sizeof(IndexPage), 1, bfp);

	for (int i = 0; i < STUDENT_INDEX_PAGE_SIZE - 2; ++i){
		if (endID >= ip.keys[i]){
			fseek(bfp, ip.pageID[i], SEEK_SET);
			fread(&dp, sizeof(StudentDataPage), 1, bfp);
			for (int j = 0; j < STUDENT_PAGE_RECORDS; ++j){
				if (dp.data[j].id <= endID){
					printf("%3d %32s\n", dp.data[j].id, dp.data[j].name);
					queryFlag = 0;
				}
				else{
					break;
				}
			}
		}
		else{
			locFlag = i;
			break;
		}
	}
	if (locFlag >= 0){
		fseek(bfp, ip.pageID[locFlag], SEEK_SET);
		fread(&dp, sizeof(StudentDataPage), 1, bfp);
		for (int j = 0; j < STUDENT_PAGE_RECORDS; ++j){
			if (dp.data[j].id <= endID){
				printf("%3d %32s\n", dp.data[j].id, dp.data[j].name);
				queryFlag = 0;
			}
		}
	}

	if (endID >= ip.keys[STUDENT_INDEX_PAGE_SIZE - 2]){
		fseek(bfp, ip.pageID[STUDENT_INDEX_PAGE_SIZE - 1], SEEK_SET);
		fread(&dp, sizeof(StudentDataPage), 1, bfp);
		for (int j = 0; j < STUDENT_PAGE_RECORDS; ++j){
			if (dp.data[j].id <= endID){
				printf("%3d %32s\n", dp.data[j].id, dp.data[j].name);
				queryFlag = 0;
			}
		}
	}
	fclose(bfp);

	if (queryFlag)
		printf("\nNo IDs could be found with such index range.\n\n");
}

void rangeQuery(char *filename, int startID, int endID){
	FILE *bfp = fopen(filename, "rb+");
	fseek(bfp, -PAGE_SIZE, SEEK_END);

	int location, locFlag = -1, queryFlag = 1;
	fread(&location, sizeof(int), 1, bfp);
	fseek(bfp, location, SEEK_SET);

	IndexPage ip;
	StudentDataPage dp;
	fread(&ip, sizeof(IndexPage), 1, bfp);

	for (int i = 0; i < STUDENT_INDEX_PAGE_SIZE - 2; ++i){
		if (startID <= ip.keys[i] || endID >= ip.keys[i]){
			if (startID <= ip.keys[i] && endID >= ip.keys[i]){
				fseek(bfp, ip.pageID[i], SEEK_SET);
				fread(&dp, sizeof(StudentDataPage), 1, bfp);
				for (int j = 0; j < STUDENT_PAGE_RECORDS; ++j){
					if (dp.data[j].id >= startID && dp.data[j].id <= endID){
						printf("%3d %32s\n", dp.data[j].id, dp.data[j].name);
						locFlag = i + 1;
						queryFlag = 0;
					}
				}
			}
		}
		else
			break;
	}
	if (locFlag >= 0){
		fseek(bfp, ip.pageID[locFlag], SEEK_SET);
		fread(&dp, sizeof(StudentDataPage), 1, bfp);
		for (int j = 0; j < STUDENT_PAGE_RECORDS; ++j){
			if (dp.data[j].id >= startID && dp.data[j].id <= endID){
				printf("%3d %32s\n", dp.data[j].id, dp.data[j].name);
				queryFlag = 0;
			}
		}
	}

	if (endID >= ip.keys[STUDENT_INDEX_PAGE_SIZE - 2]){
		fseek(bfp, ip.pageID[STUDENT_INDEX_PAGE_SIZE - 1], SEEK_SET);
		fread(&dp, sizeof(StudentDataPage), 1, bfp);
		for (int j = 0; j < STUDENT_PAGE_RECORDS; ++j){
			if (dp.data[j].id >= startID && dp.data[j].id <= endID){
				printf("%3d %32s\n", dp.data[j].id, dp.data[j].name);
				queryFlag = 0;
			}
		}
	}
	fclose(bfp);

	if (queryFlag)
		printf("\nNo IDs could be found with such index range.\n\n");
}

void stringRangeQuery(char *filename, int startID, int endID, char *string){
	FILE *bfp = fopen(filename, "rb+");
	fseek(bfp, -PAGE_SIZE, SEEK_END);

	int location, locFlag = -1, queryFlag = 1;
	fread(&location, sizeof(int), 1, bfp);
	fseek(bfp, location, SEEK_SET);

	IndexPage ip;
	StudentDataPage dp;
	fread(&ip, sizeof(IndexPage), 1, bfp);

	for (int i = 0; i < STUDENT_INDEX_PAGE_SIZE - 2; ++i){
		if (startID <= ip.keys[i] || endID >= ip.keys[i]){
			if (startID <= ip.keys[i] && endID >= ip.keys[i]){
				fseek(bfp, ip.pageID[i], SEEK_SET);
				fread(&dp, sizeof(StudentDataPage), 1, bfp);
				for (int j = 0; j < STUDENT_PAGE_RECORDS; ++j){
					if (dp.data[j].id >= startID && dp.data[j].id <= endID && (strstr(dp.data[j].name, string) != NULL)){
						printf("%3d %32s\n", dp.data[j].id, dp.data[j].name);
						locFlag = i + 1;
						queryFlag = 0;
					}
				}
			}
		}
		else
			break;
	}
	if (locFlag >= 0){
		fseek(bfp, ip.pageID[locFlag], SEEK_SET);
		fread(&dp, sizeof(StudentDataPage), 1, bfp);
		for (int j = 0; j < STUDENT_PAGE_RECORDS; ++j){
			if (dp.data[j].id >= startID && dp.data[j].id <= endID && (strstr(dp.data[j].name, string) != NULL)){
				printf("%3d %32s\n", dp.data[j].id, dp.data[j].name);
				queryFlag = 0;
			}
		}
	}

	if (endID >= ip.keys[STUDENT_INDEX_PAGE_SIZE - 2]){
		fseek(bfp, ip.pageID[STUDENT_INDEX_PAGE_SIZE - 1], SEEK_SET);
		fread(&dp, sizeof(StudentDataPage), 1, bfp);
		for (int j = 0; j < STUDENT_PAGE_RECORDS; ++j){
			if (dp.data[j].id >= startID && dp.data[j].id <= endID && (strstr(dp.data[j].name, string) != NULL)){
				printf("%3d %32s\n", dp.data[j].id, dp.data[j].name);
				queryFlag = 0;
			}
		}
	}
	fclose(bfp);

	if (queryFlag)
		printf("\nNo IDs could be found with such index range.\n\n");
}

void joinQuery(char *filename){
	FILE *bfp = fopen(filename, "rb+");

	int location[2];
	fseek(bfp, -PAGE_SIZE, SEEK_END);
	fread(&location, sizeof(int), 2, bfp);

	IndexPage studentIndexPage, marksIndexPage;
	fseek(bfp, location[0], SEEK_SET);
	fread(&studentIndexPage, sizeof(IndexPage), 1, bfp);
	fseek(bfp, location[1], SEEK_SET);
	fread(&marksIndexPage, sizeof(IndexPage), 1, bfp);

	StudentDataPage studentDataPage;
	MarksDataPage marksDataPage;

	int i = 0, j = 0, stdIndex = STUDENT_PAGE_RECORDS, marksIndex = MARKS_PAGE_RECORDS;
	
	printf("%5s %32s %15s\n", "ID", "NAME", "TOTAL");
	while (studentIndexPage.pageID[i] != -1 || marksIndexPage.pageID[j] != -1){
		if (stdIndex == STUDENT_PAGE_RECORDS){
			stdIndex = 0;
			fseek(bfp, studentIndexPage.pageID[i], SEEK_SET);
			fread(&studentDataPage, sizeof(StudentDataPage), 1, bfp);
			i += 1;
		}
		if (marksIndex == MARKS_PAGE_RECORDS){
			marksIndex = 0;
			fseek(bfp, marksIndexPage.pageID[j], SEEK_SET);
			fread(&marksDataPage, sizeof(MarksDataPage), 1, bfp);
			j += 1;
		}
		if (studentDataPage.dataType != '1' || marksDataPage.dataType != '1')
			break;
		if (studentDataPage.data[stdIndex].id < marksDataPage.data[marksIndex].studentID)
			stdIndex = stdIndex + 1;
		else if ((studentDataPage.data[stdIndex].id > marksDataPage.data[marksIndex].studentID))
			marksIndex = marksIndex + 1;
		else if ((studentDataPage.data[stdIndex].id == marksDataPage.data[marksIndex].studentID)){
			int total = 0;
			for (int k = 0; k < 4; ++k)
				total += marksDataPage.data[marksIndex].marks[k];
			printf("%5d %32s %15d\n", studentDataPage.data[stdIndex].id, studentDataPage.data[stdIndex].name, total);
			stdIndex += 1;
			marksIndex += 1;
		}
	}
	fclose(bfp);
}

void processQueries(){
	int choice, id, startID, endID;

	char string[32];

	while (1){
		printf("\n1. Search by ID");
		printf("\n2. Search by range (1 to n)");
		printf("\n3. Search by range (m to n)");
		printf("\n4. Search by name");
		printf("\n5. Joins");
		printf("\n6. Exit");

		printf("\n\nEnter your choice: ");
		scanf("%d", &choice);

		switch (choice){
		case 1: printf("\nEnter id: ");
				scanf("%d", &id);
				searchQuery("Data.bin", id);
				break;
		case 2: printf("\nEnter the end range: ");
				scanf("%d", &endID);
				endRangeQuery("Data.bin", endID);
				break;
		case 3: printf("\nEnter the start range: ");
				scanf("%d", &startID);
				printf("\nEnter the end range: ");
				scanf("%d", &endID);
				rangeQuery("Data.bin", startID, endID);
				break;
		case 4: printf("\nEnter the start range: ");
				scanf("%d", &startID);
				printf("\nEnter the end range: ");
				scanf("%d", &endID);
				printf("\nEnter string: ");
				fflush(stdin);
				gets(string);
				stringRangeQuery("Data.bin", startID, endID, string);
				break;
		case 5: joinQuery("Data.bin"); break;
		case 6: system("pause");
				exit(1);
		default: printf("\n\nInvalid Option entered.\n\n");
		}
	}
}

int main(){

	processStudentData();
	processMarksData();
	loadTablePage("Data.bin");
	processQueries();

	system("pause");
	return 0;
}