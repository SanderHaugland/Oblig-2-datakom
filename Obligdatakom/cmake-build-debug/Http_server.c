#include <stdio.h>
#include "Http_server.h"

#define BUF_SIZE 2000

#include<stdio.h>
#include<winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib,"ws2_32.lib")


void printIPaddr(struct sockaddr_in addr) {
    printf("%d.%d.%d.%d",
           addr.sin_addr.S_un.S_un_b.s_b1, addr.sin_addr.S_un.S_un_b.s_b2,
           addr.sin_addr.S_un.S_un_b.s_b3, addr.sin_addr.S_un.S_un_b.s_b4);
}

int readfile(char filename[], char sBuffer[], int nBufferSize) {
    int nFileSize = 0;
    FILE* fFile = fopen(filename, "r");

    if (fFile) {
        fseek(fFile, 0, SEEK_END);
        nFileSize = ftell(fFile);
        if(nFileSize > nBufferSize) {
            nFileSize = nBufferSize;
        }

        fseek(fFile, 0, SEEK_SET);
        if (sBuffer) {
            fread(sBuffer, 1, nFileSize, fFile);
        }
        fclose(fFile);
    }

    return nFileSize;
}

int httpServer() {

    WSADATA wsa;
    SOCKET socketListen=INVALID_SOCKET;
    SOCKET socketHTTPconnection=INVALID_SOCKET;
    struct sockaddr_in addrServer, sReceivedIPaddress;

    const short nPortNumber = 80;
    int nResult = 0;
    char sReceivedString[BUF_SIZE] = { 0 };
    char sHtmlFile[BUF_SIZE] = {0};

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("Winsock initialised.\n");

    socketListen = socket(AF_INET, SOCK_STREAM, 0);
    if (socketListen == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("TCP socket created.\n");

    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = INADDR_ANY;
    addrServer.sin_port = htons(nPortNumber);

    nResult = bind(socketListen, (struct sockaddr*)&addrServer, sizeof(addrServer));
    if (nResult == SOCKET_ERROR) {
        printf("Bind failed with error code : %d", WSAGetLastError());
        closesocket(socketListen);
        WSACleanup();
        return 1;
    }

    printf("Listen to incoming TCP-requests...\n");
    listen(socketListen, 5);

    int nClient = sizeof(struct sockaddr_in);
    socketHTTPconnection = accept(socketListen, (struct sockaddr*)&sReceivedIPaddress, &nClient);
    if (socketHTTPconnection == INVALID_SOCKET) {
        printf("accept failed with error code : %d", WSAGetLastError());
        closesocket(socketListen);
        closesocket(socketHTTPconnection);
        WSACleanup();
        return 1;
    }

    // oppgave 1. Motta HTTP-forespørsel fra klient
    nResult = recv(socketHTTPconnection, sReceivedString, BUF_SIZE - 1, 0);
    if (nResult == SOCKET_ERROR) {
        printf("recv failed with error code : %d", WSAGetLastError());
        closesocket(socketListen);
        closesocket(socketHTTPconnection);
        WSACleanup();
        return 1;
    }
    sReceivedString[nResult] = '\0';

    // oppgave 2. Skriv ut HTTP-forespørselen
    printf("\nHTTP Request\n%s\n", sReceivedString);

    // oppgave 3. Sjekk om forespørselen er en GET-forespørsel
    if (strncmp(sReceivedString, "GET", 3) == 0) {
        printf("GET request detected.\n");

        // Oppgave 4. Lese index filen
        int nFileSize = readfile("index.html", sHtmlFile, BUF_SIZE - 1);

        char sHttpHeader[512] = {0};
        char sHttpResponse[BUF_SIZE + 512] = {0};

        if (nFileSize > 0) {
            //sjekker om headeren hard enne http greien
            sHtmlFile[nFileSize] = '\0';
            sprintf(sHttpHeader,
                "HTTP/1.1 200 OK\r\n",
                nFileSize);
        }

        // Setter sammen headeren og alt annet
        int nHeaderLen = strlen(sHttpHeader);
        memcpy(sHttpResponse, sHttpHeader, nHeaderLen);
        memcpy(sHttpResponse + nHeaderLen, sHtmlFile, nFileSize);

        // 5. Send responsen tilbake til terminalen
        nResult = send(socketHTTPconnection, sHttpResponse, nHeaderLen + nFileSize, 0);
        if (nResult == SOCKET_ERROR) {
            printf("send failed with error code : %d", WSAGetLastError());
        } else {
            printf("HTTP response sent (%d bytes).\n", nResult);
        }

    }

    closesocket(socketListen);
    closesocket(socketHTTPconnection);
    WSACleanup();

    return 0;
}


int main() {
    httpServer();
    return 0;
}