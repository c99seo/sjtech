/*
 * Copyright (C) 2004 Nathan Lutchansky <lutchann@litech.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


/* Carl, 20090116, also defined in module/cap2enc/stream_kern.h. */
#define RTP_TRANS_NONE      0
#define RTP_TRANS_UDP       1
#define RTP_TRANS_INTER     2

#define MAX_INTERLEAVE_CHANNELS     8

struct rtp_endpoint;
struct conn;
struct session;

struct rtp_endpoint {
	struct session *session;
	int payload;
	int max_data_size;
	unsigned int ssrc;
	unsigned int last_timestamp;
	unsigned int timebase;
#if 0   // Carl
	unsigned int start_timestamp;
	unsigned int sync_timestamp;
	unsigned int pre_sync_timestamp;
#else
	// Carl
	unsigned int    nhz_last_timestamp;
	unsigned int    nhz;
	unsigned int    remainder;
	short           timed;
#endif
	short packetskip;
	unsigned int delay_time;
	short stopdetect;
	int seqnum;
	int packet_count;
	int octet_count;
	struct event *rtcp_send_event;
	int enable_send_rtcp; //kevin 2007.1.17
	int force_rtcp;
	int event_enabled;//kevin rtcp enabling flag
	struct timeval last_rtcp_recv;
	int retry;
	int trans_type;
	union {
		struct {
			char sdp_addr[48];
			int sdp_port;
			int rtp_fd;
			struct event *rtp_event;
			int rtcp_fd;
			struct event *rtcp_event;
			struct conn *conn;
			int rtp_chan;
		} udp;
		struct {
			struct conn *conn;
			int rtp_chan;
			int rtcp_chan;
		} inter;
	} trans;

	int enable_send_rtcp_app_motionNty;
	int timeSec_rtcp_app_motionNty;
	int flyKey;
	int keyframe_arrival;
	time_t  ep_create_time;
	struct timeval rtcp_tv_base;
	gmss_sr *sr;
};

typedef int (*get_sdp_func)(/*struct session *s*/gmss_sr *sr, char *dest, int *len/*, char *path*/);
typedef int (*setup_func)(struct session *s, int track, gmss_sr *sr);
typedef void (*play_func)(struct session *s, double *start);
typedef void (*pause_func)(struct session *s);
typedef void (*teardown_func)(struct session *s, struct rtp_endpoint *ep, int callback);     // Carl
typedef void (*session_close_func)(struct session *sess);

#define MAX_TRACKS  2

struct session {
	struct session *next;
	struct session *prev;
	get_sdp_func get_sdp;
	setup_func setup;
	play_func play;
	pause_func pause;
	teardown_func teardown;
	void *private;
	session_close_func control_close;
	void *control_private;
	struct timeval open_time;
	char addr[64];
	struct rtp_endpoint *ep[MAX_TRACKS];
};

struct loc_node {
	struct loc_node *next;
	struct loc_node *prev;
	char path[256];
};

// typedef struct session *(*open_func)( char *path, void *d );

struct req {
	struct conn *conn;
	struct pmsg *req;
	struct pmsg *resp;
};

#define CONN_PROTO_START    0
#define CONN_PROTO_RTSP     1
#define CONN_PROTO_HTTP     2
#define CONN_PROTO_SIP      3

struct conn {
	struct conn *next;
	struct conn *prev;
	int file;   // Carl
	int fd;
	int write_fd;  /* QuickTime uses the TCP connections of "GET" tunneling to send data to clients*/
	struct sockaddr_in client_addr;
	struct event *read_event;
	struct event *write_event;
	int proto;
	char http_tunnel_cookie[128]; /* used to identify QuickTime tunnels */
	int base64_count; /* QuickTime uses base64 when tunneling over HTTP */
	unsigned char req_buf[4096];
	int req_len;
	struct req *req_list;
#ifdef NO_MM
	unsigned char send_buf[16384];
//	unsigned char send_buf[PACKET_SIZE+500];
#else
	unsigned char send_buf[65536];
#endif
	int send_buf_r;
	int send_buf_w;
	int drop_after;
	void *proto_state;
	struct conn *stream_conn;  /* QuickTime uses two TCP connections for tunneling */
	int wait_teardown_reply;
};

/* There's probably a better place to put these... */
#define PUT_16(p,v) ((p)[0]=((v)>>8)&0xff,(p)[1]=(v)&0xff)
#define PUT_32(p,v) ((p)[0]=((v)>>24)&0xff,(p)[1]=((v)>>16)&0xff,(p)[2]=((v)>>8)&0xff,(p)[3]=(v)&0xff)
#if 1   /* Carl, 20090720. */ /* Fix VLC long-term disconnection issue. */
#define GET_16(p) ((((unsigned short)((p)[0])) << 8)|((p)[1]))
#define GET_32(p) ((((unsigned int)((p)[0])) << 24)| \
				   (((unsigned int)((p)[1])) << 16)| \
				   (((unsigned int)((p)[2])) << 8)|((p)[3]))
#else
#define GET_16(p) (((p)[0]<<8)|(p)[1])
#define GET_32(p) (((p)[0]<<24)|((p)[1]<<16)|((p)[2]<<8)|(p)[3])
#endif

void random_bytes(unsigned char *dest, int len);
void random_id(unsigned char *dest, int len);
struct rtp_endpoint *new_rtp_endpoint(int payload);
int connect_udp_endpoint(struct rtp_endpoint *ep,
						 struct in_addr dest_ip, int dest_port, int *our_port);
int avail_send_buf(struct conn *c);
int send_data(struct conn *c, unsigned char *d, int len);
int tcp_send_pmsg(struct conn *c, struct pmsg *msg, int len);
void connect_interleaved_endpoint(struct rtp_endpoint *ep,
								  struct conn *conn, int rtp_chan, int rtcp_chan);
void rtsp_conn_disconnect(struct conn *c, int callback);     // Carl
void http_conn_disconnect(struct conn *c);
void interleave_disconnect(struct conn *c, int chan);
int interleave_send(struct conn *conn, int chan, struct iovec *v, int count);
int interleave_recv(struct conn *c, int chan, unsigned char *d, int len);
void interleave_recv_rtcp(struct rtp_endpoint *ep, unsigned char *d, int len);
void del_rtp_endpoint(struct rtp_endpoint *ep);
// void update_rtp_timestamp( struct rtp_endpoint *ep, int time_increment );    // Carl


#ifdef WMP
int send_rtp_packet(struct rtp_endpoint *ep, struct iovec *v, int count,
					unsigned int timestamp, int marker, int keyframe, int pre_packet_size);

#else
int send_rtp_packet(struct rtp_endpoint *ep, struct iovec *v, int count,
					unsigned int timestamp, int marker);

#endif
void new_rtsp_location(char *path, char *realm, char *username, char *password,
					   open_func open, void *private);
struct session *new_session(void);
void del_session(struct session *sess);
int print_session_list(char *s, int maxlen);
int http_handle_msg(struct req *req);
int rtsp_handle_msg(struct req *req, struct conn *c);
