#include <stdio.h>   // 화면 출력, 파일 읽기/쓰기 기능
#include <stdlib.h>  // 메모리 관리 (동적 할당)
#include <string.h>  // 문자열 복사, 길이 측정
#include <math.h>    // 수학 계산 (제곱근 등)

#define MAX_SENSORS 50   // 최대 저장 가능한 센서 개수
#define PAGE_SIZE 10     // 한 페이지에 보여줄 센서 수

// 센서 정보를 저장하는 구조체 (상자)
// name: 센서 이름 (예: "온도 센서")
// size: 데이터 개수 (예: 5개)
// data: 측정값 배열 (예: [10, 20, 30, 40, 50])
typedef struct {
    char* name; // 동적 할당으로 이름 저장
    int size;   // 데이터 개수
    int* data;  // 동적 할당으로 데이터 저장
} DataSource;

// 통계 결과를 저장하는 구조체 (결과 리포트)
typedef struct {
    double mean, variance, stddev; // 평균, 분산, 표준편차
    int min, max, range, count;    // 최소, 최대, 범위, 데이터 개수
    double median;                 // 중앙값
} Stats;

// 전체 시스템을 관리하는 구조체 (센서 보관함)
typedef struct {
    DataSource sensors[MAX_SENSORS]; // 센서 배열
    int count;                       // 현재 센서 개수
} SensorSystem;

// ==============================================
// [통계 계산기] - 센서 데이터를 분석하는 함수
// ==============================================
Stats calculate_stats(DataSource ds) {
    Stats s = {0}; // 결과 저장 변수 초기화

    if (ds.size == 0 || ds.data == NULL) {
        return s; // 데이터 없으면 계산 안함
    }

    // 1단계: 데이터 복사 후 정렬 (버블 정렬)
    int* sorted = malloc(ds.size * sizeof(int)); // 임시 저장소
    memcpy(sorted, ds.data, ds.size * sizeof(int)); // 원본 복사

    // 버블 정렬 (거품이 올라오듯 큰 수를 뒤로)
    for(int i=0; i<ds.size-1; i++){
        for(int j=0; j<ds.size-i-1; j++){
            if(sorted[j] > sorted[j+1]){ // 앞이 더 크면
                // 두 수 교환 (스왑)
                int temp = sorted[j];
                sorted[j] = sorted[j+1];
                sorted[j+1] = temp;
            }
        }
    }

    // 2단계: 기본 통계 계산
    s.min = sorted[0];          // 정렬 후 첫 번째 값 = 최소값
    s.max = sorted[ds.size-1];  // 마지막 값 = 최대값
    s.range = s.max - s.min;    // 범위
    s.count = ds.size;          // 데이터 개수

    double sum = 0;
    for(int i=0; i<ds.size; i++){
        sum += sorted[i]; // 모든 데이터 합계
    }
    s.mean = sum / ds.size; // 평균

    // 중앙값 계산 (가운데 값)
    if(ds.size % 2 == 0) { // 짝수일 때
        s.median = (sorted[ds.size/2-1] + sorted[ds.size/2])/2.0;
    } else { // 홀수일 때
        s.median = sorted[ds.size/2];
    }

    // 3단계: 분산과 표준편차
    double sum_sq = 0;
    for(int i=0; i<ds.size; i++){
        sum_sq += (sorted[i] - s.mean) * (sorted[i] - s.mean); // (값-평균)^2 합
    }
    s.variance = sum_sq / ds.size; // 분산
    s.stddev = sqrt(s.variance);   // 표준편차 (분산 제곱근)
    
    free(sorted); // 임시 저장소 정리
    return s;
}

// ==============================================
// [메뉴 기능] - 센서 추가/조회/삭제/통계 계산 등
// ==============================================

// 1. 센서 추가 기능
void add_sensor(SensorSystem* sys) {
    if(sys->count >= MAX_SENSORS) {
        printf("최대 %d개까지 저장 가능합니다!\n", MAX_SENSORS);
        return;
    }
    
    char name[50];
    int size;
    
    printf("센서 이름: ");
    scanf("%s", name);
    printf("데이터 개수: ");
    scanf("%d", &size);
    
    // 이름 저장 (동적 할당)
    sys->sensors[sys->count].name = malloc(strlen(name) + 1); // +1은 글자 끝 표시
    strcpy(sys->sensors[sys->count].name, name);
    
    // 데이터 공간 생성
    sys->sensors[sys->count].size = size;
    sys->sensors[sys->count].data = malloc(size * sizeof(int));
    
    // 데이터 입력 받기
    printf("데이터 입력 (%d개):\n", size);
    for(int i=0; i<size; i++){
        printf("[%d]: ", i+1);
        scanf("%d", &sys->sensors[sys->count].data[i]);
    }
    
    sys->count++; // 센서 개수 증가
    printf("센서 추가 완료!\n");
}

// 2. 센서 목록 보기 (페이징)
void show_sensors(SensorSystem* sys, int page) {
    if(sys->count == 0) {
        printf("등록된 센서가 없습니다.\n");
        return;
    }
    
    int start = (page-1) * PAGE_SIZE; // 시작 인덱스
    int end = start + PAGE_SIZE;
    if(end > sys->count) end = sys->count; // 끝 인덱스 조정

    printf("\n=== 센서 목록 (페이지 %d) ===\n", page);
    for(int i=start; i<end; i++){
        printf("%d. %s (%d개 데이터)\n", 
               i+1, sys->sensors[i].name, sys->sensors[i].size);
    }
    printf("\n");
}

// 3. 센서 삭제 기능
void delete_sensor(SensorSystem* sys) {
    if(sys->count == 0) {
        printf("삭제할 센서가 없습니다.\n");
        return;
    }
    
    show_sensors(sys, 1); // 목록 보여주기
    int idx;
    char confirm;
    
    printf("삭제할 센서 번호: ");
    scanf("%d", &idx);
    idx--; // 0부터 시작하도록 조정
    
    if(idx < 0 || idx >= sys->count) {
        printf("잘못된 번호입니다.\n");
        return;
    }
    
    printf("정말 '%s' 센서를 삭제할까요? (y/n): ", sys->sensors[idx].name);
    scanf(" %c", &confirm);
    
    if(confirm == 'y' || confirm == 'Y') {
        // 메모리 정리
        free(sys->sensors[idx].name);
        free(sys->sensors[idx].data);
        
        // 삭제 후 뒤의 센서들을 앞으로 이동
        for(int i=idx; i<sys->count-1; i++){
            sys->sensors[i] = sys->sensors[i+1];
        }
        sys->count--;
        printf("센서 삭제 완료!\n");
    }
}

// 4. 통계 계산 기능
void compute_statistics(SensorSystem* sys) {
    if(sys->count == 0) {
        printf("계산할 센서가 없습니다.\n");
        return;
    }
    
    show_sensors(sys, 1);
    int idx;
    
    printf("통계를 계산할 센서 번호: ");
    scanf("%d", &idx);
    idx--; // 0부터 시작하도록 조정
    
    if(idx < 0 || idx >= sys->count) {
        printf("잘못된 센서 번호입니다.\n");
        return;
    }
    
    Stats stats = calculate_stats(sys->sensors[idx]);
    
    printf("\n[%s 통계 결과] \n", sys->sensors[idx].name);
    printf("평균     : %.2f\n", stats.mean);
    printf("중앙값   : %.2f\n", stats.median);
    printf("최솟값   : %d\n", stats.min);
    printf("최댓값   : %d\n", stats.max);
    printf("범위     : %d\n", stats.range);
    printf("분산     : %.2f\n", stats.variance);
    printf("표준편차 : %.2f\n", stats.stddev);
    printf("데이터수 : %d\n\n", stats.count);
}

// 5. 센서 비교 기능(추후 구현)
void compare_sensors(SensorSystem* sys) {
    if(sys->count < 2) {
        printf("비교할 센서가 부족합니다 (최소 2개 필요).\n");
        return;
    }
    
    show_sensors(sys, 1);
    int idx1, idx2;
    
    printf("첫 번째 센서 번호: ");
    scanf("%d", &idx1);
    printf("두 번째 센서 번호: ");
    scanf("%d", &idx2);
    
    idx1--; idx2--; // 0부터 시작하도록 조정
    
    if(idx1 < 0 || idx1 >= sys->count || idx2 < 0 || idx2 >= sys->count) {
        printf("잘못된 센서 번호입니다.\n");
        return;
    }
    
    printf("\n[%s vs %s 비교 결과]\n", 
           sys->sensors[idx1].name, sys->sensors[idx2].name);
    printf("(비교 기능은 추후 구현 예정입니다)\n\n");
}

// 6. 도움말 기능
void show_help() {
    printf("\n=== 센서 데이터 관리 시스템 도움말 ===\n");
    printf("1. 추가    : 새로운 센서 정보 입력\n");
    printf("2. 조회    : 등록된 센서 목록 표시\n");
    printf("3. 수정    : 센서 정보 변경 (추후 구현)\n");
    printf("4. 삭제    : 센서 정보 제거\n");
    printf("5. 통계    : 선택한 센서의 통계값 계산\n");
    printf("6. 비교    : 두 센서 데이터 비교 분석\n");
    printf("7. 저장    : 현재 데이터를 파일로 저장 (추후 구현)\n");
    printf("8. 로드    : 파일에서 데이터 불러오기 (추후 구현)\n");
    printf("9. 도움말  : 이 화면을 표시\n");
    printf("0. 종료    : 프로그램 종료\n\n");
}

// ==============================================
// [메인 함수] - 프로그램 시작점
// ==============================================
int main() {
    SensorSystem sys = {0}; // 센서 시스템 초기화
    int choice;
    
    printf("센서 데이터 관리 시스템 시작!\n");
    
    while(1) {
        // 메뉴 출력
        printf("\n=== 메인 메뉴 ===\n");
        printf("1.추가 2.조회 3.수정 4.삭제 5.통계\n");
        printf("6.비교 7.저장 8.로드 9.도움말 0.종료\n");
        printf("선택 >> ");
        scanf("%d", &choice);
        
        switch(choice) {
            case 1: add_sensor(&sys); break;
            case 2: show_sensors(&sys, 1); break;
            case 3: printf("수정 기능은 준비 중입니다.\n"); break;
            case 4: delete_sensor(&sys); break;
            case 5: compute_statistics(&sys); break;
            case 6: compare_sensors(&sys); break;
            case 7: printf("저장 기능은 준비 중입니다.\n"); break;
            case 8: printf("로드 기능은 준비 중입니다.\n"); break;
            case 9: show_help(); break;
            case 0: 
                printf("프로그램을 종료합니다.\n");
                // 메모리 정리
                for(int i=0; i<sys.count; i++){
                    free(sys.sensors[i].name);
                    free(sys.sensors[i].data);
                }
                exit(0);
            default: printf("잘못된 선택입니다.\n");
        }
    }
    return 0;
}
