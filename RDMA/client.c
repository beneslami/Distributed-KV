#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>

#include "debug.h"
#include "config.h"
#include "setup_ib.h"
#include "ib.h"
#include "client.h"

void*
client_thread_func (void *arg){
    int         ret		 = 0, n = 0, i = 0, j = 0;
    long	thread_id	 = (long) arg;
    int         msg_size	 = config_info.msg_size;
    int         num_concurr_msgs = config_info.num_concurr_msgs;
    int         num_peers        = ib_res.num_qps;

    pthread_t   self;
    cpu_set_t   cpuset;

    int                  num_wc		= 20;
    struct ibv_qp	**qp		= ib_res.qp;
    struct ibv_cq       *cq		= ib_res.cq;
    struct ibv_srq      *srq            = ib_res.srq;
    struct ibv_wc       *wc		= NULL;
    uint32_t             lkey           = ib_res.mr->lkey;

    char		*buf_ptr	= ib_res.ib_buf;
    char		*buf_base	= ib_res.ib_buf;
    int			 buf_offset	= 0;
    size_t               buf_size	= ib_res.ib_buf_size;

    uint32_t		imm_data	= 0;
    int			num_acked_peers = 0;
    bool		start_sending	= false;
    bool		stop		= false;
    struct timeval      start, end;
    long                ops_count	= 0;
    double              duration	= 0.0;
    double              throughput	= 0.0;

    /* set thread affinity */
    CPU_ZERO (&cpuset);
    CPU_SET  ((int)thread_id, &cpuset);
    self = pthread_self ();
    ret  = pthread_setaffinity_np (self, sizeof(cpu_set_t), &cpuset);
    check (ret == 0, "thread[%ld]: failed to set thread affinity", thread_id);

    /* pre-post recvs */
    wc = (struct ibv_wc *) calloc (num_wc, sizeof(struct ibv_wc));
    check (wc != NULL, "thread[%ld]: failed to allocate wc.", thread_id);

    for (i = 0; i < num_peers; i++) {
	for (j = 0; j < num_concurr_msgs; j++) {
	    ret = post_srq_recv (msg_size, lkey, (uint64_t)buf_ptr, srq, buf_ptr);
	    buf_offset = (buf_offset + msg_size) % buf_size;
	    buf_ptr = buf_base + buf_offset;
	}
    }

    /* wait for start signal */
    while (start_sending != true) {
        do {
            n = ibv_poll_cq (cq, num_wc, wc);
        } while (n < 1);
        check (n > 0, "thread[%ld]: failed to poll cq", thread_id);

        for (i = 0; i < n; i++) {
            if (wc[i].status != IBV_WC_SUCCESS) {
                check (0, "thread[%ld]: wc failed status: %s.",
                       thread_id, ibv_wc_status_str(wc[i].status));
            }
            if (wc[i].opcode == IBV_WC_RECV) {
                /* post a receive */
                post_srq_recv (msg_size, lkey, wc[i].wr_id, srq, (char *)wc[i].wr_id);

                if (ntohl(wc[i].imm_data) == MSG_CTL_START) {
		    num_acked_peers += 1;
		    if (num_acked_peers == num_peers) {
			start_sending = true;
			break;
		    }
                }
            }
        }
    }
    log ("thread[%ld]: ready to send", thread_id);

    /* pre-post sends */
    buf_offset = 0;
    debug ("buf_ptr = %"PRIx64"", (uint64_t)buf_ptr);
    for (i = 0; i < num_peers; i++) {
	for (j = 0; j < num_concurr_msgs; j++) {
	    ret = post_send (msg_size, lkey, (uint64_t)buf_ptr, (uint32_t)i, qp[i], buf_ptr);
	    check (ret == 0, "thread[%ld]: failed to post send", thread_id);
	    buf_offset = (buf_offset + msg_size) % buf_size;
	    buf_ptr = buf_base + buf_offset;
	}
    }

    num_acked_peers = 0;
    while (stop != true) {
        /* poll cq */
        n = ibv_poll_cq (cq, num_wc, wc);
        if (n < 0) {
            check (0, "thread[%ld]: Failed to poll cq", thread_id);
        }

        for (i = 0; i < n; i++) {
            if (wc[i].status != IBV_WC_SUCCESS) {
                if (wc[i].opcode == IBV_WC_SEND) {
                    check (0, "thread[%ld]: send failed status: %s; wr_id = %"PRIx64"",
                           thread_id, ibv_wc_status_str(wc[i].status), wc[i].wr_id);
                } else {
                    check (0, "thread[%ld]: recv failed status: %s; wr_id = %"PRIx64"",
                           thread_id, ibv_wc_status_str(wc[i].status), wc[i].wr_id);
                }
            }

	    if (wc[i].opcode == IBV_WC_RECV) {
                ops_count += 1;
                debug ("ops_count = %ld", ops_count);

                if (ops_count == NUM_WARMING_UP_OPS) {
                    gettimeofday (&start, NULL);
                }

		imm_data = ntohl(wc[i].imm_data);
		char *msg_ptr = (char *)wc[i].wr_id;

                if (imm_data == MSG_CTL_STOP) {
		    num_acked_peers += 1;
		    if (num_acked_peers == num_peers) {
			gettimeofday (&end, NULL);
			stop = true;
			break;
		    }
                } else {
		    /* echo the message back */
		    post_send (msg_size, lkey, 0, imm_data, qp[imm_data], msg_ptr);
		}

                /* post a new receive */
		ret = post_srq_recv (msg_size, lkey, wc[i].wr_id, srq, msg_ptr);
            }
        } /* loop through all wc */
    }

    /* dump statistics */
    duration   = (double)((end.tv_sec - start.tv_sec) * 1000000 +
			  (end.tv_usec - start.tv_usec));
    throughput = (double)(ops_count) / duration;
    log ("thread[%ld]: throughput = %f (Mops/s)",  thread_id, throughput);

    free (wc);
    pthread_exit ((void *)0);

 error:
    if (wc != NULL) {
    	free (wc);
    }
    pthread_exit ((void *)-1);
}

int
run_client(){
    int		ret	    = 0;
    long	num_threads = 1;
    long	i	    = 0;

    pthread_t	   *client_threads = NULL;
    pthread_attr_t  attr;
    void	   *status;

    log (LOG_SUB_HEADER, "Run Client");

    /* initialize threads */
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);

    client_threads = (pthread_t *) calloc (num_threads, sizeof(pthread_t));
    check (client_threads != NULL, "Failed to allocate client_threads.");

    for (i = 0; i < num_threads; i++) {
	ret = pthread_create (&client_threads[i], &attr,
			      client_thread_func, (void *)i);
	check (ret == 0, "Failed to create client_thread[%ld]", i);
    }

    bool thread_ret_normally = true;
    for (i = 0; i < num_threads; i++) {
	ret = pthread_join(client_threads[i], &status);
	check (ret == 0, "Failed to join client_thread[%ld].", i);
	if ((long)status != 0) {
            thread_ret_normally = false;
            log ("thread[%ld]: failed to execute", i);
        }
    }

    if (thread_ret_normally == false) {
        goto error;
    }

    pthread_attr_destroy(&attr);
    free (client_threads);
    return 0;

 error:
    if (client_threads != NULL) {
        free (client_threads);
    }

    pthread_attr_destroy(&attr);
    return -1;
}
