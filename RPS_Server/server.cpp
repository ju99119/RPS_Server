#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SERVERPORT 9000
#define BUFSIZE    512

// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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
    // ���� �̰������ ���� ����
    // ���� �������� ���� ���� ����ؾ���
    char buffer[BUFSIZE];
    int match_num = 10;
    int win_num = 6;
    int lose_num = 4;
    float win_ratio = (float)win_num / lose_num;

    const char win_info1[] = "�� �÷��� / ";
    const char win_info2[] = "�� �¸�, ";
    const char win_info3[] = "�� �й� / ";
    const char win_info4[] = "���� �·� : ";
    sprintf_s(buffer, "%d%s%d%s%d%s%s%f%%", match_num, win_info1, win_num, win_info2, lose_num, win_info3, win_info4, win_ratio);

    return buffer;
}

typedef struct {
    int RPS_flag=0;
    char RPS_result[BUFSIZE];
} RPS_result;

RPS_result RPS(char client_RPS[]) {
    RPS_result result;
    char scissors[5] = "����";
    char rock[5] = "����";
    char paper[3] = "��";

    // ������ �������� ���� ���� ���� ���� �ϱ� ���ؼ� �ð��� ���� ��ȭ�ϰ� ��
    srand(time(0));
    int server_choice_num = rand() % 3;
    char server_choice[5];

    if (server_choice_num == 0)
        strcpy_s(server_choice, scissors); // �������� ������ ������ ��
    else if (server_choice_num == 1)
        strcpy_s(server_choice, rock); // �������� ������ ������ ��
    else if (server_choice_num == 2)
        strcpy_s(server_choice, paper); // �������� ������ ���� ��

    printf("server : %s\n", server_choice);

    if (strcmp(server_choice, client_RPS) == 0) { // ���º� ����-����, ��-��, ����-����
        printf("���º��Դϴ�. �ٽ� ����!!");
        strcpy_s(result.RPS_result,"���º��Դϴ�. �ٽ� ����!!");
        result.RPS_flag = 3;
    }

    else if (strcmp(server_choice, client_RPS) != 0) {
        if (strcmp(scissors, client_RPS) == 0) { // Ŭ�� ������ �´ٸ�
            if (strcmp(rock, server_choice) == 0) {
                printf("���� : %s, Ŭ�� : %s (����,������ ��!)\n", server_choice, client_RPS); // ���� : ����, Ŭ�� : ���� Ŭ����
                strcpy_s(result.RPS_result, "������ ������ ���� �й�! <���>�մϴ�");
                result.RPS_flag = 1;
            }
            else if (strcmp(paper, server_choice) == 0) {
                printf("���� :%s, Ŭ�� : %s (��,������ ��!)\n", server_choice, client_RPS); // ����: ��, Ŭ�� : ���� Ŭ���
                strcpy_s(result.RPS_result, "������ ���� ���� �¸�! <����>�մϴ�");
                result.RPS_flag = 2;
            }
        }

        if (strcmp(rock, client_RPS) == 0) { // Ŭ�� ������ �´ٸ�
            if (strcmp(scissors, server_choice) == 0) {
                printf("���� : %s, Ŭ�� : %s (����,������ ��!)\n", server_choice, client_RPS); // ���� : ����, Ŭ�� : ���� Ŭ���
                strcpy_s(result.RPS_result, "������ ������ ���� �¸�! <����>�մϴ�");
                result.RPS_flag = 2;
            }
            else if (strcmp(paper, server_choice) == 0) {
                printf("���� :%s, Ŭ�� : %s (��,������ ��!)\n", server_choice, client_RPS); // ����: ��, Ŭ�� : ���� Ŭ����
                strcpy_s(result.RPS_result, "������ ���� ���� �й�! <���>�մϴ�");
                result.RPS_flag = 1;
            }
        }

        if (strcmp(paper, client_RPS) == 0) { // Ŭ�� ���� �´ٸ�
            if (strcmp(scissors, server_choice) == 0) {
                printf("���� : %s, Ŭ�� : %s (����,���� ��!)\n", server_choice, client_RPS); // ���� : ����, Ŭ�� : �� Ŭ����
                strcpy_s(result.RPS_result, "������ ������ ���� �й�! <���>�մϴ�");
                result.RPS_flag = 1;
            }
            else if (strcmp(rock, server_choice) == 0) {
                printf("���� :%s, Ŭ�� : %s (����,���� ��!)\n", server_choice, client_RPS); // ����: ����, Ŭ�� : �� Ŭ���
                strcpy_s(result.RPS_result, "������ ������ ���� �¸�! <����>�մϴ�");
                result.RPS_flag = 2;
            }
        }
    }

    return result;
}

int main(int argc, char* argv[])
{
    int retval;

    // ���� �ʱ�ȭ
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

    // ������ ��ſ� ����� ����
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1];
    char start_msg[72] = "***** ����� ������ �����ϰڽ��ϴ�.*****\n���� ������������ �����մϴ�.\n"; // ������ �� ������ Msg

    while (1) {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // ������ Ŭ���̾�Ʈ ���� ���
        printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // ���� Msg ������!!!!
        send(client_sock, start_msg, strlen(start_msg), 0);

        while (1) {

            char win_rate[9] = "�·�Ȯ��";
            char game_out[5] = "����";

            if (strcmp(win_rate, buf) == 0) {
                printf("�·� ����\n");
                const char* RPS_result;
                RPS_result = Print_win_rate();
                retval = send(client_sock, RPS_result, BUFSIZE, 0);
                if (retval == SOCKET_ERROR) {
                    err_display("send()");
                    break;
                }
            }

            if (strcmp(game_out, buf) == 0) {
                printf("���� ����\n");
                char game_out[] = "������ �����մϴ�.";
                retval = send(client_sock, game_out, sizeof(game_out), 0);
                if (retval == SOCKET_ERROR) {
                    err_display("send()");
                    break;
                }
                break;
            }
            while (1) {
                // ù��° ���������� ������ �ޱ�
                retval = recv(client_sock, buf, BUFSIZE, 0);
                if (retval == SOCKET_ERROR) {
                    err_display("recv()");
                    break;
                }
                //else if (retval == 0)
                //    break;

                // ���� ������ ���
                buf[retval] = '\0';
                printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
                    ntohs(clientaddr.sin_port), buf);

                RPS_result rps_result;

                rps_result = RPS(buf); // ���������� �Լ� ȣ��
                printf("%d, %s\n", rps_result.RPS_flag, rps_result.RPS_result); // ������ �÷���, ����޽��� ���

                if (rps_result.RPS_flag == 1) {
                    printf("�й�����. ������ �����Ұ�\n");
                    send(client_sock, rps_result.RPS_result, sizeof(rps_result.RPS_result), 0);
                    break;
                }

                else if (rps_result.RPS_flag == 2) {
                    printf("�¸�����. Ŭ�� �����ϼ�\n");
                    send(client_sock, rps_result.RPS_result, sizeof(rps_result.RPS_result), 0);
                    break;
                }

                else if (rps_result.RPS_flag == 3) {
                    printf("���º���\n");
                    send(client_sock, rps_result.RPS_result, sizeof(rps_result.RPS_result), 0);
                }
            }
            printf("���������� �� ����� ����");
            //char MJP_msg[] = "����� ������ �����մϴ�. ";
            //send(client_sock, MJP_msg, sizeof(MJP_msg), 0);

        }

        // closesocket()
        closesocket(client_sock);
        printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
    }
    // closesocket()
    closesocket(listen_sock);

    // ���� ����
    WSACleanup();
    return 0;
}
