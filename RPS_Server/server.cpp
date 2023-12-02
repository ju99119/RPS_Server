#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
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

// ������ Ŭ���̾�Ʈ ���� �ְ� �޴� ��Ŷ ����ü ����
#pragma pack(1)
typedef struct {
    int rps = 0; // Ŭ���̾�Ʈ�� ������ ������������ ���� �÷����̴�.        0 : ����(��), 1 : ����(��), 2 : ��(��)
    int status = 0; // ó�� ���� ���� �� �� �����͸� �޴� �ʵ�.              0 : ���� ���� ��� ����, 1 : ���� ����!
    int client_win_flag = 0; // Ŭ���̾�Ʈ�� �¸��� ������ 1�� ����.         Ŭ���̾�Ʈ�� �� �� �̰������ ���� Flag
    int setnum_flag = 0; // ����� ���� ���� ������ 1�� ����.                ����� ������ �� �� �ߴ����� ���� Flag
    int end_flag = 0; // ���� �÷����̴�.                                    0 : default, 1 : 1�� ������ Ŭ���̾�Ʈ ���� �ٷ� ����
    int win_flag = 0; // ���������� ��� �÷���.                             0 : ���º�, 1 : Ŭ���̾�Ʈ �й�(����� ���), 2 : Ŭ���̾�Ʈ �¸�(����� ����)
    char data[64]; // ������ ���� �ʵ�
} RPS;
#pragma pack()

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

    RPS packet; // ������ Ŭ���̾�Ʈ ���� �ְ� �޴� ��Ŷ ����ü ����

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

        ZeroMemory(&packet, sizeof(packet));                                                // ������ ���� �� ������ ��Ŷ ������ ��� 0���� �ʱ�ȭ

        while (1) {
            if (packet.end_flag == 0) {
                // ���� ������ ���� ���� Msg ������!!!!
                strcpy_s(packet.data, "###############\t\t\t\tKJH ����� ����\t\t\t\t###############");
                retval = send(client_sock, (char*)&packet, sizeof(packet), 0);
                if (retval == SOCKET_ERROR) {
                    err_display("send()");
                    packet.end_flag = 1;
                    break;
                }
                else if (retval == 0)
                    break;

                printf("��û �÷��� üũ(��) : %d\n", packet.status);                       // ��û �÷��� ����� ���� �Ǿ����� Ȯ���ϴ� �뵵
                retval = recv(client_sock, (char*)&packet, sizeof(packet), 0);              // ���� ���� ��û �÷��� ���� (Ŭ���̾�Ʈ���� '����' �Է�) --> packet.status = 1
                if (retval == SOCKET_ERROR) {
                    err_display("recv()");
                    packet.end_flag = 1;
                    break;
                }
                else if (retval == 0)
                    break;
                printf("��û �÷��� üũ(��) : %d\n", packet.status);                       // ��û �÷��� ����� ���� �Ǿ����� Ȯ���ϴ� �뵵
            }
            if (packet.end_flag == 1) {
                printf("���� ���� ��û �ܰ迡�� ���Ḧ �Է�\n");                            // ���� ���� ��û �ܰ迡�� Ŭ���̾�Ʈ�� '����' �Է½� ���� ���� 
                break;
            }

            while (packet.status == 1) {                                                    // ���� ��û �ܰ迡�� �ùٸ��� ���� ���� �޾Ƽ� status�� 1�� ��Ʈ �Ǿ��ٸ� �ݺ�

                if (packet.end_flag == 0)                                                   // end_flag�� 0�̶�� ������ ���� ���� ��� (�� if���� �Ʒ� else if���� ���� ���� Ȯ�� �뵵��, ��� ������)
                    printf("���� ����\n");

                else if (packet.end_flag == 1) {                                            // end_flag�� 1�̶�� ����
                    break;
                }

                // win_flag = 0�̸� ���ºζ�� �ǹ̷�, ������������ ���ºζ�� ��� �ݺ�
                while (packet.win_flag == 0) {
                    // Ŭ���̾�Ʈ�� ���������� ������ �ޱ�
                    retval = recv(client_sock, (char*)&packet, sizeof(packet), 0);
                    if (retval == SOCKET_ERROR) {
                        err_display("recv()");
                        packet.end_flag = 1;
                        break;
                    }
                    else if (retval == 0)
                        break;

                    if (packet.end_flag == 1) {
                        printf("���������� �ܰ迡�� ���Ḧ �Է�\n");
                        break;
                    }

                    // ���� ������ ���
                    printf("[TCP/%s:%d] Ŭ�� ���� �� : %d\n", inet_ntoa(clientaddr.sin_addr),
                        ntohs(clientaddr.sin_port), packet.rps);                                // 0 : ����, 1 : ����, 2 : ��

                    // ������ ������������ �������� �����ϰ� �ϱ� ���� �κ�
                    srand(time(0));
                    int server_rps = -1;                                                        // default:-1, 0:����, 1:����, 2:��
                    server_rps = rand() % 3;
                    printf("���� ���� Ȯ�� : %d\n", server_rps);                                // ������ ������ Ȯ���ϱ� ���� �κ�. 0 : ����, 1 : ����, 2 : ��

                    // ������������ ���ºζ��( ���� ���������� ���� = Ŭ�� ���������� �Է� )
                    if (packet.rps == server_rps) {
                        printf("���������� ���ºο���\n");
                        packet.win_flag = 0;                                                    // win_flag --> 0 : ���º�, 1 : Ŭ�� �й�, 2 : Ŭ�� �¸�
                    }
                    //������������ ���ºΰ� �ƴ϶��
                    else {
                        if (packet.rps == GAWI) {                                               // Ŭ���̾�Ʈ�� ������ �Է����� ��
                            if (server_rps == BAWEE) {                                          // ������ �������
                                printf("Ŭ�� ���� ���� ����, ���� ��\n");
                                packet.win_flag = 1;                                            // win_flag --> 0 : ���º�, 1 : Ŭ�� �й�, 2 : Ŭ�� �¸�
                            }
                            else if (server_rps == BO) {                                        // ������ �����
                                printf("Ŭ�� ���� ���� ��, Ŭ�� ��\n");
                                packet.win_flag = 2;                                            // win_flag --> 0 : ���º�, 1 : Ŭ�� �й�, 2 : Ŭ�� �¸�
                            }
                        }
                        else if (packet.rps == BAWEE) {                                         // Ŭ���̾�Ʈ�� ������ �Է����� ��
                            if (server_rps == GAWI) {                                           // ������ �������
                                printf("Ŭ�� ���� ���� ����, Ŭ�� ��\n");
                                packet.win_flag = 2;                                            // win_flag --> 0 : ���º�, 1 : Ŭ�� �й�, 2 : Ŭ�� �¸�
                            }
                            else if (server_rps == BO) {                                        // ������ �����
                                printf("Ŭ�� ���� ���� �� , ���� ��\n");
                                packet.win_flag = 1;                                            // win_flag --> 0 : ���º�, 1 : Ŭ�� �й�, 2 : Ŭ�� �¸�
                            }
                        }
                        else if (packet.rps = BO) {                                             // Ŭ���̾�Ʈ�� ���� �Է����� ��
                            if (server_rps == GAWI) {                                           // ������ �������
                                printf("Ŭ�� �� ���� ����, ���� ��\n");
                                packet.win_flag = 1;                                            // win_flag --> 0 : ���º�, 1 : Ŭ�� �й�, 2 : Ŭ�� �¸�
                            }
                            else if (server_rps == BAWEE) {                                     // ������ �������
                                printf("Ŭ�� �� ���� ����, Ŭ�� ��\n");
                                packet.win_flag = 2;                                            // win_flag --> 0 : ���º�, 1 : Ŭ�� �й�, 2 : Ŭ�� �¸�
                            }
                        }
                    }

                    // ���������� ���� ���(win_flag) Ŭ���̾�Ʈ���� ����
                    retval = send(client_sock, (char*)&packet, sizeof(packet), 0);
                    if (retval == SOCKET_ERROR) {
                        err_display("send()");
                        packet.end_flag = 1;
                        break;
                    }
                    else if (retval == 0)
                        break;
                    printf("�����������ϰ� �¸��÷��� ��Ŷ ������. %d����Ʈ\n", retval);
                } // ���������� ������ while ��

                // ���� �÷��װ� 1�� �ƴ϶��(���Ḧ �Է����� �ʾҴٸ� ����� ����)
                if (packet.end_flag == 0) {
                    // ����� ���� ����
                    // Ŭ���̾�Ʈ�� �¸��߰ų� �й��ߴٸ�. ��, ������������ ���ºΰ� �ƴ϶��
                    if (packet.win_flag == 1 || packet.win_flag == 2) {
                        strcpy_s(packet.data, "����� ������ �����մϴ�");
                        retval = send(client_sock, (char*)&packet, sizeof(packet), 0);          // ����� ���� �����Ѵٰ� Ŭ���̾�Ʈ���� ����
                        if (retval == SOCKET_ERROR) {
                            err_display("send()");
                            packet.end_flag = 1;
                            break;
                        }
                        else if (retval == 0)
                            break;
                        printf("%d����Ʈ. ����������Ѵٰ� ������\n", retval);

                        while (1) {
                            // ����� ������ �ޱ�
                            printf("����� ������ ��ٸ��� ��...\n");
                            retval = recv(client_sock, (char*)&packet, sizeof(packet), 0);
                            if (retval == SOCKET_ERROR) {
                                err_display("recv()");
                                break;
                            }

                            // ����� �ܰ迡�� '����' �Է½� ����
                            if (packet.end_flag == 1) {
                                printf("����� �ܰ迡�� ���Ḧ �Է�\n");
                                break;
                            }

                            printf("[TCP/%s:%d] Ŭ�� ���� �� : %d\n", inet_ntoa(clientaddr.sin_addr),
                                ntohs(clientaddr.sin_port), packet.rps);                        // ����� ������ Ȯ��.  0 : ��, 1 : ��, 2 : ��

                            // ������ ������� �������� �����ϰ� �ϱ� ���� �κ�
                            srand(time(0));
                            int server_mjp = -1;                                                // default:-1, 0:����, 1:����, 2:��
                            server_mjp = rand() % 3;
                            printf("���� ���� Ȯ�� : %d\n", server_mjp);

                            // Ŭ���̾�Ʈ�� '���' �� ��
                            if (packet.win_flag == 1) {
                                // Ŭ���̾�Ʈ�� ������ ������� ���ٸ� Ŭ���̾�Ʈ �й�
                                if (packet.rps == server_mjp) {
                                    printf("������ �̰���!! ����!!!\n");
                                    // ���� ����� �Բ� �й� �޽��� ����. ��Ŷ ���� �Ϻ� �ʱ�ȭ
                                    strcpy_s(packet.data, "���� ���� : �й��ϼ̽��ϴ�!!�Ф�\n");
                                    packet.status = 0;
                                    packet.win_flag = 0;
                                    packet.setnum_flag += 1;                                    // ������ �̰����ϱ� ��Ʈ ���� �����ϰ� �·� �÷��� ���� ����.
                                    send(client_sock, (char*)&packet, sizeof(packet), 0);
                                    if (retval == SOCKET_ERROR) {
                                        err_display("send()");
                                        packet.end_flag = 1;
                                        break;
                                    }
                                    else if (retval == 0)
                                        break;
                                    break; // ���� ����� ���� ���� Ż�� �� ���� ���� ����
                                }
                                else if (packet.rps == GAWI && server_mjp == BO) {              // Ŭ�� �����¿��� Ŭ���̾�Ʈ�� ������������ �̱�ٸ� ���� ����
                                    printf("Ŭ�� : ��, ���� : �� || ��������\n");
                                    strcpy_s(packet.data, "Ŭ�� : ��, ���� : �� || ����  ����(Ŭ�� ����)\n");
                                    packet.win_flag = 2;                                        // ���� ���븦 ���� �¸� �÷��� ��ȯ
                                }
                                else if (packet.rps == GAWI && server_mjp == BAWEE) {           // Ŭ�� �����¿��� Ŭ���̾�Ʈ�� ������������ ���ٸ� �����״������
                                    printf("Ŭ�� : ��, ���� : �� || �����״��\n");
                                    strcpy_s(packet.data, "Ŭ�� : ��, ���� : �� || �����״��(Ŭ�� ���)\n");
                                }
                                else if (packet.rps == BAWEE && server_mjp == BO) {
                                    printf("Ŭ�� : ��, ���� : �� || �����״��\n");
                                    strcpy_s(packet.data, "Ŭ�� : ��, ���� : �� || �����״��(Ŭ�� ���)\n");
                                }
                                else if (packet.rps == BAWEE && server_mjp == GAWI) {
                                    printf("Ŭ�� : ��, ���� : �� || ��������\n");
                                    strcpy_s(packet.data, "Ŭ�� : ��, ���� : �� || ����  ����(Ŭ�� ����)\n");
                                    packet.win_flag = 2;
                                }
                                else if (packet.rps == BO && server_mjp == BAWEE) {
                                    printf("Ŭ�� : ��, ���� : �� || ��������\n");
                                    strcpy_s(packet.data, "Ŭ�� : ��, ���� : �� || ����  ����(Ŭ�� ����)\n");
                                    packet.win_flag = 2;
                                }
                                else if (packet.rps == BO && server_mjp == GAWI) {
                                    printf("Ŭ�� : ��, ���� : �� || �����״��\n");
                                    strcpy_s(packet.data, "Ŭ�� : ��, ���� : �� || �����״��(Ŭ�� ���)\n");
                                }
                            }

                            // Ŭ���̾�Ʈ�� '����' �� ��
                            else if (packet.win_flag == 2) {
                                // Ŭ���̾�Ʈ�� ������ ������� ���ٸ� Ŭ���̾�Ʈ �¸�
                                if (packet.rps == server_mjp) {
                                    printf("Ŭ�� �̰���!! ����");
                                    // ���� ����� �Բ� �¸� �޽��� ����. ��Ŷ ���� �Ϻ� �ʱ�ȭ
                                    strcpy_s(packet.data, "���� ���� : �ڽ¸����ϼ̽��ϴ�!!!!!!!!\n");
                                    packet.status = 0;
                                    packet.win_flag = 0;
                                    packet.setnum_flag += 1;
                                    packet.client_win_flag += 1;                                // Ŭ�� �̰����Ƿ� ��Ʈ ���� �·� �÷��� �� �� ����
                                    send(client_sock, (char*)&packet, sizeof(packet), 0);
                                    if (retval == SOCKET_ERROR) {
                                        err_display("send()");
                                        packet.end_flag = 1;
                                        break;
                                    }
                                    else if (retval == 0)
                                        break;
                                    break; // ���� ����� ���� ���� Ż�� �� ���� ���� ����
                                }
                                else if (packet.rps == GAWI && server_mjp == BO) {              // Ŭ�� ���ݻ��¿��� Ŭ���̾�Ʈ�� ������������ �̱�ٸ� �����״������
                                    printf("Ŭ�� : ��, ���� : �� || �����״��");
                                    strcpy_s(packet.data, "Ŭ�� : ��, ���� : �� || �����״��(Ŭ�� ����)\n");
                                }
                                else if (packet.rps == GAWI && server_mjp == BAWEE) {           // Ŭ�� ���ݻ��¿��� Ŭ���̾�Ʈ�� ������������ ���ٸ� ���� ����
                                    printf("Ŭ�� : ��, ���� : �� || ��������");
                                    strcpy_s(packet.data, "Ŭ�� : ��, ���� : �� || ��������(Ŭ�� ���)\n");
                                    packet.win_flag = 1;                                        // ���� ���븦 ���� �¸� �÷��� ��ȯ
                                }
                                else if (packet.rps == BAWEE && server_mjp == BO) {
                                    printf("Ŭ�� : ��, ���� : �� || ��������");
                                    strcpy_s(packet.data, "Ŭ�� : ��, ���� : �� || ��������(Ŭ�� ���)\n");
                                    packet.win_flag = 1;
                                }
                                else if (packet.rps == BAWEE && server_mjp == GAWI) {
                                    printf("Ŭ�� : ��, ���� : �� || �����״��");
                                    strcpy_s(packet.data, "Ŭ�� : ��, ���� : �� || �����״��(Ŭ�� ����)\n");
                                }
                                else if (packet.rps == BO && server_mjp == BAWEE) {
                                    printf("Ŭ�� : ��, ���� : �� || �����״��");
                                    strcpy_s(packet.data, "Ŭ�� : ��, ���� : �� || �����״��(Ŭ�� ����)\n");
                                }
                                else if (packet.rps == BO && server_mjp == GAWI) {
                                    printf("Ŭ�� : ��, ���� : �� || ��������");
                                    strcpy_s(packet.data, "Ŭ�� : ��, ���� : �� || ��������(Ŭ�� ���)\n");
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

                    }// ����� ���� ���ǹ�. win_flag�� 1�̳� 2�� ��.
                } // ���� �÷��װ� 0�� �� �����ϴ� ���ǹ�
            } // ���� ���� ��û �÷��װ� 1�� �� �����ϴ� �ݺ���
        } // Ŭ���̾�Ʈ accept�ϸ� �ݺ��ϴ� ���ѷ���

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
