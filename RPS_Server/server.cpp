#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SERVERPORT 9000
#define BUFSIZE    512
#define GAWI 0
#define BAWEE 1
#define BO 2

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

// 서버와 클라이언트 간에 주고 받는 패킷 구조체 정의
#pragma pack(1)
typedef struct {
    int rps = 0; // 클라이언트가 선택한 가위바위보에 대한 플래그이다.        0 : 가위(찌), 1 : 바위(묵), 2 : 보(빠)
    int status = 0; // 처음 게임 시작 할 때 데이터를 받는 필드.              0 : 게임 시작 대기 상태, 1 : 게임 시작!
    int client_win_flag = 0; // 클라이언트가 승리할 때마다 1씩 증가.         클라이언트가 몇 판 이겼는지에 대한 Flag
    int setnum_flag = 0; // 묵찌빠 승패 판정 때마다 1씩 증가.                묵찌빠 게임을 몇 판 했는지에 대한 Flag
    int end_flag = 0; // 종료 플래그이다.                                    0 : default, 1 : 1이 들어오면 클라이언트 연결 바로 종료
    int win_flag = 0; // 가위바위보 결과 플래그.                             0 : 무승부, 1 : 클라이언트 패배(묵찌빠 방어), 2 : 클라이언트 승리(묵찌빠 공격)
    char data[64]; // 데이터 전달 필드
} RPS;
#pragma pack()

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

    RPS packet; // 서버와 클라이언트 간에 주고 받는 패킷 구조체 선언

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

        ZeroMemory(&packet, sizeof(packet));                                                // 서버에 연결 될 때마다 패킷 데이터 모두 0으로 초기화

        while (1) {
            if (packet.end_flag == 0) {
                // 연결 성공시 최초 시작 Msg 보내기!!!!
                strcpy_s(packet.data, "###############\t\t\t\tKJH 묵찌빠 게임\t\t\t\t###############");
                retval = send(client_sock, (char*)&packet, sizeof(packet), 0);
                if (retval == SOCKET_ERROR) {
                    err_display("send()");
                    packet.end_flag = 1;
                    break;
                }
                else if (retval == 0)
                    break;

                printf("요청 플래그 체크(전) : %d\n", packet.status);                       // 요청 플래그 제대로 전달 되었는지 확인하는 용도
                retval = recv(client_sock, (char*)&packet, sizeof(packet), 0);              // 게임 시작 요청 플래그 받음 (클라이언트에서 '시작' 입력) --> packet.status = 1
                if (retval == SOCKET_ERROR) {
                    err_display("recv()");
                    packet.end_flag = 1;
                    break;
                }
                else if (retval == 0)
                    break;
                printf("요청 플래그 체크(후) : %d\n", packet.status);                       // 요청 플래그 제대로 전달 되었는지 확인하는 용도
            }
            if (packet.end_flag == 1) {
                printf("게임 시작 요청 단계에서 종료를 입력\n");                            // 게임 시작 요청 단계에서 클라이언트가 '종료' 입력시 연결 종료 
                break;
            }

            while (packet.status == 1) {                                                    // 시작 요청 단계에서 올바르게 값을 전달 받아서 status가 1로 세트 되었다면 반복

                if (packet.end_flag == 0)                                                   // end_flag가 0이라면 서버에 게임 시작 출력 (이 if문과 아래 else if문은 서버 동작 확인 용도로, 없어도 무방함)
                    printf("게임 시작\n");

                else if (packet.end_flag == 1) {                                            // end_flag가 1이라면 종료
                    break;
                }

                // win_flag = 0이면 무승부라는 의미로, 가위바위보가 무승부라면 계속 반복
                while (packet.win_flag == 0) {
                    // 클라이언트의 가위바위보 데이터 받기
                    retval = recv(client_sock, (char*)&packet, sizeof(packet), 0);
                    if (retval == SOCKET_ERROR) {
                        err_display("recv()");
                        packet.end_flag = 1;
                        break;
                    }
                    else if (retval == 0)
                        break;

                    if (packet.end_flag == 1) {
                        printf("가위바위보 단계에서 종료를 입력\n");
                        break;
                    }

                    // 받은 데이터 출력
                    printf("[TCP/%s:%d] 클라가 보낸 것 : %d\n", inet_ntoa(clientaddr.sin_addr),
                        ntohs(clientaddr.sin_port), packet.rps);                                // 0 : 가위, 1 : 바위, 2 : 보

                    // 서버가 가위바위보를 랜덤으로 선택하게 하기 위한 부분
                    srand(time(0));
                    int server_rps = -1;                                                        // default:-1, 0:가위, 1:바위, 2:보
                    server_rps = rand() % 3;
                    printf("서버 선택 확인 : %d\n", server_rps);                                // 서버의 선택을 확인하기 위한 부분. 0 : 가위, 1 : 바위, 2 : 보

                    // 가위바위보가 무승부라면( 서버 가위바위보 선택 = 클라 가위바위보 입력 )
                    if (packet.rps == server_rps) {
                        printf("가위바위보 무승부에요\n");
                        packet.win_flag = 0;                                                    // win_flag --> 0 : 무승부, 1 : 클라 패배, 2 : 클라 승리
                    }
                    //가위바위보가 무승부가 아니라면
                    else {
                        if (packet.rps == GAWI) {                                               // 클라이언트가 가위를 입력했을 때
                            if (server_rps == BAWEE) {                                          // 서버가 바위라면
                                printf("클라 가위 서버 바위, 서버 승\n");
                                packet.win_flag = 1;                                            // win_flag --> 0 : 무승부, 1 : 클라 패배, 2 : 클라 승리
                            }
                            else if (server_rps == BO) {                                        // 서버가 보라면
                                printf("클라 가위 서버 보, 클라 승\n");
                                packet.win_flag = 2;                                            // win_flag --> 0 : 무승부, 1 : 클라 패배, 2 : 클라 승리
                            }
                        }
                        else if (packet.rps == BAWEE) {                                         // 클라이언트가 바위를 입력했을 때
                            if (server_rps == GAWI) {                                           // 서버가 가위라면
                                printf("클라 바위 서버 가위, 클라 승\n");
                                packet.win_flag = 2;                                            // win_flag --> 0 : 무승부, 1 : 클라 패배, 2 : 클라 승리
                            }
                            else if (server_rps == BO) {                                        // 서버가 보라면
                                printf("클라 바위 서버 보 , 서버 승\n");
                                packet.win_flag = 1;                                            // win_flag --> 0 : 무승부, 1 : 클라 패배, 2 : 클라 승리
                            }
                        }
                        else if (packet.rps = BO) {                                             // 클라이언트가 보를 입력했을 때
                            if (server_rps == GAWI) {                                           // 서버가 가위라면
                                printf("클라 보 서버 가위, 서버 승\n");
                                packet.win_flag = 1;                                            // win_flag --> 0 : 무승부, 1 : 클라 패배, 2 : 클라 승리
                            }
                            else if (server_rps == BAWEE) {                                     // 서버가 바위라면
                                printf("클라 보 서버 바위, 클라 승\n");
                                packet.win_flag = 2;                                            // win_flag --> 0 : 무승부, 1 : 클라 패배, 2 : 클라 승리
                            }
                        }
                    }

                    // 가위바위보 판정 결과(win_flag) 클라이언트에게 전송
                    retval = send(client_sock, (char*)&packet, sizeof(packet), 0);
                    if (retval == SOCKET_ERROR) {
                        err_display("send()");
                        packet.end_flag = 1;
                        break;
                    }
                    else if (retval == 0)
                        break;
                    printf("가위바위보하고 승리플래그 패킷 보냈음. %d바이트\n", retval);
                } // 가위바위보 끝나는 while 문

                // 종료 플래그가 1이 아니라면(종료를 입력하지 않았다면 묵찌빠 진행)
                if (packet.end_flag == 0) {
                    // 묵찌빠 게임 시작
                    // 클라이언트가 승리했거나 패배했다면. 즉, 가위바위보가 무승부가 아니라면
                    if (packet.win_flag == 1 || packet.win_flag == 2) {
                        strcpy_s(packet.data, "묵찌빠 게임을 진행합니다");
                        retval = send(client_sock, (char*)&packet, sizeof(packet), 0);          // 묵찌빠 게임 시작한다고 클라이언트에게 전송
                        if (retval == SOCKET_ERROR) {
                            err_display("send()");
                            packet.end_flag = 1;
                            break;
                        }
                        else if (retval == 0)
                            break;
                        printf("%d바이트. 묵찌빠시작한다고 보냈음\n", retval);

                        while (1) {
                            // 묵찌빠 데이터 받기
                            printf("묵찌빠 들어오길 기다리는 중...\n");
                            retval = recv(client_sock, (char*)&packet, sizeof(packet), 0);
                            if (retval == SOCKET_ERROR) {
                                err_display("recv()");
                                break;
                            }

                            // 묵찌빠 단계에서 '종료' 입력시 종료
                            if (packet.end_flag == 1) {
                                printf("묵찌빠 단계에서 종료를 입력\n");
                                break;
                            }

                            printf("[TCP/%s:%d] 클라가 보낸 것 : %d\n", inet_ntoa(clientaddr.sin_addr),
                                ntohs(clientaddr.sin_port), packet.rps);                        // 묵찌빠 데이터 확인.  0 : 찌, 1 : 묵, 2 : 빠

                            // 서버가 묵찌빠를 랜덤으로 선택하게 하기 위한 부분
                            srand(time(0));
                            int server_mjp = -1;                                                // default:-1, 0:가위, 1:바위, 2:보
                            server_mjp = rand() % 3;
                            printf("서버 선택 확인 : %d\n", server_mjp);

                            // 클라이언트가 '방어' 일 때
                            if (packet.win_flag == 1) {
                                // 클라이언트와 서버의 묵찌빠가 같다면 클라이언트 패배
                                if (packet.rps == server_mjp) {
                                    printf("서버가 이겼음!! 수고!!!\n");
                                    // 게임 종료와 함께 패배 메시지 전송. 패킷 내용 일부 초기화
                                    strcpy_s(packet.data, "게임 종료 : 패배하셨습니다!!ㅠㅠ\n");
                                    packet.status = 0;
                                    packet.win_flag = 0;
                                    packet.setnum_flag += 1;                                    // 서버가 이겼으니까 세트 수만 증가하고 승률 플래그 증가 안함.
                                    send(client_sock, (char*)&packet, sizeof(packet), 0);
                                    if (retval == SOCKET_ERROR) {
                                        err_display("send()");
                                        packet.end_flag = 1;
                                        break;
                                    }
                                    else if (retval == 0)
                                        break;
                                    break; // 게임 종료로 현재 게임 탈출 후 다음 게임 진행
                                }
                                else if (packet.rps == GAWI && server_mjp == BO) {              // 클라 방어상태에서 클라이언트가 가위바위보를 이긴다면 공수 교대
                                    printf("클라 : 찌, 서버 : 빠 || 공수교대\n");
                                    strcpy_s(packet.data, "클라 : 찌, 서버 : 빠 || 공수  교대(클라 공격)\n");
                                    packet.win_flag = 2;                                        // 공수 교대를 위해 승리 플래그 전환
                                }
                                else if (packet.rps == GAWI && server_mjp == BAWEE) {           // 클라 방어상태에서 클라이언트가 가위바위보를 진다면 공수그대로유지
                                    printf("클라 : 찌, 서버 : 묵 || 공수그대로\n");
                                    strcpy_s(packet.data, "클라 : 찌, 서버 : 묵 || 공수그대로(클라 방어)\n");
                                }
                                else if (packet.rps == BAWEE && server_mjp == BO) {
                                    printf("클라 : 묵, 서버 : 빠 || 공수그대로\n");
                                    strcpy_s(packet.data, "클라 : 묵, 서버 : 빠 || 공수그대로(클라 방어)\n");
                                }
                                else if (packet.rps == BAWEE && server_mjp == GAWI) {
                                    printf("클라 : 묵, 서버 : 찌 || 공수교대\n");
                                    strcpy_s(packet.data, "클라 : 묵, 서버 : 찌 || 공수  교대(클라 공격)\n");
                                    packet.win_flag = 2;
                                }
                                else if (packet.rps == BO && server_mjp == BAWEE) {
                                    printf("클라 : 빠, 서버 : 묵 || 공수교대\n");
                                    strcpy_s(packet.data, "클라 : 빠, 서버 : 묵 || 공수  교대(클라 공격)\n");
                                    packet.win_flag = 2;
                                }
                                else if (packet.rps == BO && server_mjp == GAWI) {
                                    printf("클라 : 빠, 서버 : 찌 || 공수그대로\n");
                                    strcpy_s(packet.data, "클라 : 빠, 서버 : 찌 || 공수그대로(클라 방어)\n");
                                }
                            }

                            // 클라이언트가 '공격' 일 때
                            else if (packet.win_flag == 2) {
                                // 클라이언트와 서버의 묵찌빠가 같다면 클라이언트 승리
                                if (packet.rps == server_mjp) {
                                    printf("클라가 이겼음!! ㅋㅋ");
                                    // 게임 종료와 함께 승리 메시지 전송. 패킷 내용 일부 초기화
                                    strcpy_s(packet.data, "게임 종료 : ★승리★하셨습니다!!!!!!!!\n");
                                    packet.status = 0;
                                    packet.win_flag = 0;
                                    packet.setnum_flag += 1;
                                    packet.client_win_flag += 1;                                // 클라가 이겼으므로 세트 수랑 승률 플래그 둘 다 증가
                                    send(client_sock, (char*)&packet, sizeof(packet), 0);
                                    if (retval == SOCKET_ERROR) {
                                        err_display("send()");
                                        packet.end_flag = 1;
                                        break;
                                    }
                                    else if (retval == 0)
                                        break;
                                    break; // 게임 종료로 현재 게임 탈출 후 다음 게임 진행
                                }
                                else if (packet.rps == GAWI && server_mjp == BO) {              // 클라 공격상태에서 클라이언트가 가위바위보를 이긴다면 공수그대로유지
                                    printf("클라 : 찌, 서버 : 빠 || 공수그대로");
                                    strcpy_s(packet.data, "클라 : 찌, 서버 : 빠 || 공수그대로(클라 공격)\n");
                                }
                                else if (packet.rps == GAWI && server_mjp == BAWEE) {           // 클라 공격상태에서 클라이언트가 가위바위보를 진다면 공수 교대
                                    printf("클라 : 찌, 서버 : 묵 || 공수교대");
                                    strcpy_s(packet.data, "클라 : 찌, 서버 : 묵 || 공수교대(클라 방어)\n");
                                    packet.win_flag = 1;                                        // 공수 교대를 위해 승리 플래그 전환
                                }
                                else if (packet.rps == BAWEE && server_mjp == BO) {
                                    printf("클라 : 묵, 서버 : 빠 || 공수교대");
                                    strcpy_s(packet.data, "클라 : 묵, 서버 : 빠 || 공수교대(클라 방어)\n");
                                    packet.win_flag = 1;
                                }
                                else if (packet.rps == BAWEE && server_mjp == GAWI) {
                                    printf("클라 : 묵, 서버 : 찌 || 공수그대로");
                                    strcpy_s(packet.data, "클라 : 묵, 서버 : 찌 || 공수그대로(클라 공격)\n");
                                }
                                else if (packet.rps == BO && server_mjp == BAWEE) {
                                    printf("클라 : 빠, 서버 : 묵 || 공수그대로");
                                    strcpy_s(packet.data, "클라 : 빠, 서버 : 묵 || 공수그대로(클라 공격)\n");
                                }
                                else if (packet.rps == BO && server_mjp == GAWI) {
                                    printf("클라 : 빠, 서버 : 찌 || 공수교대");
                                    strcpy_s(packet.data, "클라 : 빠, 서버 : 찌 || 공수교대(클라 방어)\n");
                                    packet.win_flag = 1;
                                }
                            }
                            send(client_sock, (char*)&packet, sizeof(packet), 0);
                            if (retval == SOCKET_ERROR) {
                                err_display("send()");
                                packet.end_flag = 1;
                                break;
                            }
                            else if (retval == 0)
                                break;
                        }

                    }// 묵찌빠 게임 조건문. win_flag가 1이나 2일 때.
                } // 종료 플래그가 0일 때 진행하는 조건문
            } // 게임 시작 요청 플래그가 1일 때 진행하는 반복문
        } // 클라이언트 accept하면 반복하는 무한루프

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
