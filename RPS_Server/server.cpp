#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SERVERPORT 9000
#define BUFSIZE    512

// 소켓 함수 오류 출력 후 종료
void err_quit(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// 소켓 함수 오류 출력
void err_display(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

const char* Print_win_rate() {
    // 몇판 이겼는지에 대한 변수
    // 몇판 졌는지에 대한 변수 계산해야함
    char buffer[BUFSIZE];
    int match_num = 10;
    int win_num = 6;
    int lose_num = 4;
    float win_ratio = (float)win_num / lose_num;

    const char win_info1[] = "판 플레이 / ";
    const char win_info2[] = "판 승리, ";
    const char win_info3[] = "판 패배 / ";
    const char win_info4[] = "현재 승률 : ";
    sprintf_s(buffer, "%d%s%d%s%d%s%s%f%%", match_num, win_info1, win_num, win_info2, lose_num, win_info3, win_info4, win_ratio);

    return buffer;
}

typedef struct {
    int RPS_flag=0;
    char RPS_result[BUFSIZE];
} RPS_result;

RPS_result RPS(char client_RPS[]) {
    RPS_result result;
    char scissors[5] = "가위";
    char rock[5] = "바위";
    char paper[3] = "보";

    // 서버가 랜덤으로 가위 바위 보를 내게 하기 위해서 시간에 따라 변화하게 함
    srand(time(0));
    int server_choice_num = rand() % 3;
    char server_choice[5];

    if (server_choice_num == 0)
        strcpy_s(server_choice, scissors); // 랜덤으로 서버가 가위를 냄
    else if (server_choice_num == 1)
        strcpy_s(server_choice, rock); // 랜덤으로 서버가 바위를 냄
    else if (server_choice_num == 2)
        strcpy_s(server_choice, paper); // 랜덤으로 서버가 보를 냄

    printf("server : %s\n", server_choice);

    if (strcmp(server_choice, client_RPS) == 0) { // 무승부 가위-가위, 보-보, 바위-바위
        printf("무승부입니다. 다시 도전!!");
        strcpy_s(result.RPS_result,"무승부입니다. 다시 도전!!");
        result.RPS_flag = 3;
    }

    else if (strcmp(server_choice, client_RPS) != 0) {
        if (strcmp(scissors, client_RPS) == 0) { // 클라가 가위를 냈다면
            if (strcmp(rock, server_choice) == 0) {
                printf("서버 : %s, 클라 : %s (바위,가위로 패!)\n", server_choice, client_RPS); // 서버 : 바위, 클라 : 가위 클라패
                strcpy_s(result.RPS_result, "서버가 바위를 내서 패배! <방어>합니다");
                result.RPS_flag = 1;
            }
            else if (strcmp(paper, server_choice) == 0) {
                printf("서버 :%s, 클라 : %s (보,가위로 승!)\n", server_choice, client_RPS); // 서버: 보, 클라 : 가위 클라승
                strcpy_s(result.RPS_result, "서버가 보를 내서 승리! <공격>합니다");
                result.RPS_flag = 2;
            }
        }

        if (strcmp(rock, client_RPS) == 0) { // 클라가 바위를 냈다면
            if (strcmp(scissors, server_choice) == 0) {
                printf("서버 : %s, 클라 : %s (가위,바위로 승!)\n", server_choice, client_RPS); // 서버 : 가위, 클라 : 바위 클라승
                strcpy_s(result.RPS_result, "서버가 가위를 내서 승리! <공격>합니다");
                result.RPS_flag = 2;
            }
            else if (strcmp(paper, server_choice) == 0) {
                printf("서버 :%s, 클라 : %s (보,바위로 패!)\n", server_choice, client_RPS); // 서버: 보, 클라 : 바위 클라패
                strcpy_s(result.RPS_result, "서버가 보를 내서 패배! <방어>합니다");
                result.RPS_flag = 1;
            }
        }

        if (strcmp(paper, client_RPS) == 0) { // 클라가 보를 냈다면
            if (strcmp(scissors, server_choice) == 0) {
                printf("서버 : %s, 클라 : %s (가위,보로 패!)\n", server_choice, client_RPS); // 서버 : 가위, 클라 : 보 클라패
                strcpy_s(result.RPS_result, "서버가 가위를 내서 패배! <방어>합니다");
                result.RPS_flag = 1;
            }
            else if (strcmp(rock, server_choice) == 0) {
                printf("서버 :%s, 클라 : %s (바위,보로 승!)\n", server_choice, client_RPS); // 서버: 바위, 클라 : 보 클라승
                strcpy_s(result.RPS_result, "서버가 바위를 내서 승리! <공격>합니다");
                result.RPS_flag = 2;
            }
        }
    }

    return result;
}

int main(int argc, char* argv[])
{
    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    // 데이터 통신에 사용할 변수
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1];
    char start_msg[72] = "***** 묵찌빠 게임을 시작하겠습니다.*****\n먼저 가위바위보를 진행합니다.\n"; // 시작할 때 보내는 Msg

    while (1) {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // 접속한 클라이언트 정보 출력
        printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // 시작 Msg 보내기!!!!
        send(client_sock, start_msg, strlen(start_msg), 0);

        while (1) {

            char win_rate[9] = "승률확인";
            char game_out[5] = "종료";

            if (strcmp(win_rate, buf) == 0) {
                printf("승률 전송\n");
                const char* RPS_result;
                RPS_result = Print_win_rate();
                retval = send(client_sock, RPS_result, BUFSIZE, 0);
                if (retval == SOCKET_ERROR) {
                    err_display("send()");
                    break;
                }
            }

            if (strcmp(game_out, buf) == 0) {
                printf("게임 종료\n");
                char game_out[] = "게임을 종료합니다.";
                retval = send(client_sock, game_out, sizeof(game_out), 0);
                if (retval == SOCKET_ERROR) {
                    err_display("send()");
                    break;
                }
                break;
            }
            while (1) {
                // 첫번째 가위바위보 데이터 받기
                retval = recv(client_sock, buf, BUFSIZE, 0);
                if (retval == SOCKET_ERROR) {
                    err_display("recv()");
                    break;
                }
                //else if (retval == 0)
                //    break;

                // 받은 데이터 출력
                buf[retval] = '\0';
                printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
                    ntohs(clientaddr.sin_port), buf);

                RPS_result rps_result;

                rps_result = RPS(buf); // 가위바위보 함수 호출
                printf("%d, %s\n", rps_result.RPS_flag, rps_result.RPS_result); // 서버에 플래그, 결과메시지 출력

                if (rps_result.RPS_flag == 1) {
                    printf("패배했음. 서버가 공격할게\n");
                    send(client_sock, rps_result.RPS_result, sizeof(rps_result.RPS_result), 0);
                    break;
                }

                else if (rps_result.RPS_flag == 2) {
                    printf("승리했음. 클라가 공격하셈\n");
                    send(client_sock, rps_result.RPS_result, sizeof(rps_result.RPS_result), 0);
                    break;
                }

                else if (rps_result.RPS_flag == 3) {
                    printf("무승부임\n");
                    send(client_sock, rps_result.RPS_result, sizeof(rps_result.RPS_result), 0);
                }
            }
            printf("가위바위보 끝 묵찌빠 시작");
            //char MJP_msg[] = "묵찌빠 게임을 시작합니다. ";
            //send(client_sock, MJP_msg, sizeof(MJP_msg), 0);

        }

        // closesocket()
        closesocket(client_sock);
        printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
    }
    // closesocket()
    closesocket(listen_sock);

    // 윈속 종료
    WSACleanup();
    return 0;
}
