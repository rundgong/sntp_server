/*
    Copyright 2015 rundgong

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>


struct sntp_packet_t {
    uint8_t mode_vn_li;
    uint8_t stratum;
    uint8_t poll;
    int8_t precision;
    uint32_t root_delay;
    uint32_t root_dispersion;
    uint32_t reference_identifier;
    uint32_t reference_timestamp_secs;
    uint32_t reference_timestamp_fraq;
    uint32_t originate_timestamp_secs;
    uint32_t originate_timestamp_fraq;
    uint32_t receive_timestamp_seqs;
    uint32_t receive_timestamp_fraq;
    uint32_t transmit_timestamp_secs;
    uint32_t transmit_timestamp_fraq;
};

static const uint16_t NPT_PORT = 123;

void prepare_ntp_response(struct sntp_packet_t* request, struct sntp_packet_t* response)
{
    // Values taken from actual NTP server response @ 2015-09-14 08:52:54

    response->mode_vn_li = 0x1C;    // NTP version 3 + server mode
    response->stratum = 2;   // secondary reference
    response->poll = 3;
    response->precision = 0xE9;
    response->root_delay = htonl(0x0000002D);
    response->root_dispersion = htonl(0x00000BA2);
    response->reference_identifier = htonl(0xC0249016);    // 192.36.144.22
    response->reference_timestamp_secs = htonl(0xD9A101F9);
    response->reference_timestamp_fraq = htonl(0x071FEF81);
    response->originate_timestamp_secs = request->transmit_timestamp_secs;
    response->originate_timestamp_fraq = request->transmit_timestamp_fraq;
    response->receive_timestamp_seqs = htonl(0xD9A10966);
    response->receive_timestamp_fraq = htonl(0x0072A235);
    response->transmit_timestamp_secs = htonl(0xD9A10966);
    response->transmit_timestamp_fraq = htonl(0x0074799F);
}

int main(int argc, char**argv)
{
    int sockfd, num_received_bytes, ret;
    struct sockaddr_in server_address, client_address;
    socklen_t socket_length;
    char receive_buffer[1000];
    struct sntp_packet_t ntp_request, ntp_response;
    memset(&ntp_request, 0, sizeof(ntp_request));

    sockfd=socket(AF_INET,SOCK_DGRAM,0);

    memset( &server_address, 0, sizeof(server_address) );
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr=htonl(INADDR_ANY);
    server_address.sin_port=htons(NPT_PORT);
    ret = bind(sockfd,(struct sockaddr *)&server_address,sizeof(server_address));
    if(ret)
    {
        printf("*** Error bind failed ret=%d\n", ret);
        return ret;
    }

    printf("NTP server started...\n");
    for (;;)
    {
        socket_length = sizeof(client_address);
        num_received_bytes = recvfrom(sockfd, receive_buffer, 1000, 0,
              (struct sockaddr *)&client_address, &socket_length);

        if( num_received_bytes != sizeof(struct sntp_packet_t) )
        {
            printf("*** Error unexpected packet size=%d\n", num_received_bytes);
            continue;
        }

        memmove( &ntp_request, receive_buffer, num_received_bytes );

        prepare_ntp_response( &ntp_request, &ntp_response);

        printf("NTP Response sent\n");

        sendto(sockfd, &ntp_response, sizeof(struct sntp_packet_t), 0,
                (struct sockaddr *)&client_address, sizeof(client_address));

    }
}
